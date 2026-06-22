
#include "interrupt.hpp"

#include <cstdint>
#include <go32.h>
#include <pc.h>

bool DOS_HookInterrupt(int irq, DOS_InterruptHookFn fn, DOS_InterruptHook *hook) {
  hook->fn = fn;
  hook->irq = irq;
  hook->interrupt_vector = DOS_IRQToVector(irq);

  // Prepare seginfo for the wrapper. _go32_dpmi_allocate_iret_wrapper
  // expects pm_offset to be the address of the C function to call.
  hook->irq_handler_seginfo.pm_offset = reinterpret_cast<uint32_t>(fn);
  hook->irq_handler_seginfo.pm_selector = _go32_my_cs();

  if (_go32_dpmi_allocate_iret_wrapper(&hook->irq_handler_seginfo) != 0) {
    return false;
  }

  _go32_dpmi_get_protected_mode_interrupt_vector(hook->interrupt_vector, &hook->original_irq_handler_seginfo);
  _go32_dpmi_set_protected_mode_interrupt_vector(
      hook->interrupt_vector,
      &hook->irq_handler_seginfo);

  // enable interrupt on the correct PIC
  if (irq > 7) {
    outportb(PIC2_DATA, inportb(PIC2_DATA) & ~(1 << (irq - 8)));// unmask on slave PIC
    outportb(PIC1_DATA, inportb(PIC1_DATA) & ~(1 << 2));        // ensure cascade (IRQ2) is unmasked
  } else {
    outportb(PIC1_DATA, inportb(PIC1_DATA) & ~(1 << irq));// unmask on master PIC
  }

  return true;
}

void DOS_CallOriginalInterrupt(DOS_InterruptHook *hook) {
  if (!hook || hook->original_irq_handler_seginfo.pm_selector == 0) return;

  struct {
    uint32_t offset;
    uint16_t selector;
  } __attribute__((packed)) far_ptr;

  far_ptr.offset   = hook->original_irq_handler_seginfo.pm_offset;
  far_ptr.selector = hook->original_irq_handler_seginfo.pm_selector;

  
  // We need to simulate an interrupt call to the original handler.
  // Interrupt handlers expect flags to be on the stack, and they end with IRET.
  // A far call (lcall) followed by IRET in the handler will correctly return here
  // because IRET will pop EIP, CS, and FLAGS.
  __asm__ __volatile__(
      "pushfl\n\t"       // Satisfies the IRETD of the original handler
      "lcall *%0"        // Execute a completely safe, alignment-verified 32-bit far call
      :
      : "m" (far_ptr)
      : "eax", "ecx", "edx", "memory", "cc"
  );
}

void DOS_UnhookInterrupt(DOS_InterruptHook *hook, bool disable_interrupt) {
  if (!hook || !hook->fn) {
    return;
  }

  if (disable_interrupt) {
    if (hook->irq > 7) {
      outportb(PIC2_DATA, inportb(PIC2_DATA) | (1 << (hook->irq - 8)));// mask on slave PIC
    } else {
      outportb(PIC1_DATA, inportb(PIC1_DATA) | (1 << hook->irq));// mask on master PIC
    }
  }

  _go32_dpmi_set_protected_mode_interrupt_vector(hook->interrupt_vector, &hook->original_irq_handler_seginfo);
  _go32_dpmi_free_iret_wrapper(&hook->irq_handler_seginfo);

  hook->fn = nullptr;
}
