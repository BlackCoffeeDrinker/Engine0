
#pragma once
#include <cstdint>
#include <dpmi.h>
#include <pc.h>

// 8259 PIC (Programmable Interrupt Controller) ports and commands
constexpr uint16_t PIC1_COMMAND = 0x20;  // master PIC command port
constexpr std::uint16_t PIC1_DATA = 0x21;// master PIC data (mask) port
constexpr uint16_t PIC2_COMMAND = 0xA0;  // slave PIC command port
constexpr uint16_t PIC2_DATA = 0xA1;     // slave PIC data (mask) port
constexpr uint8_t PIC_EOI = 0x20;        // end-of-interrupt command

extern "C" {
inline int DOS_IRQToVector(int irq) { return irq + ((irq > 7) ? 104 : 8); }
inline void DOS_DisableInterrupts() { __asm__ __volatile__("cli\n"); }
inline void DOS_EnableInterrupts() { __asm__ __volatile__("sti\n"); }

inline void DOS_EndOfInterrupt(int irq) {
  if (irq > 7) {
    outportb(PIC2_COMMAND, PIC_EOI);
  }
  outportb(PIC1_COMMAND, PIC_EOI);
}

typedef void (*DOS_InterruptHookFn)();
typedef struct DOS_InterruptHook {
  DOS_InterruptHookFn fn;
  int irq;
  int interrupt_vector;// this is the _vector_ number, not the IRQ number!
  _go32_dpmi_seginfo irq_handler_seginfo;
  _go32_dpmi_seginfo original_irq_handler_seginfo;
} DOS_InterruptHook;

bool DOS_HookInterrupt(int irq, DOS_InterruptHookFn fn, DOS_InterruptHook *hook);// `irq` is the IRQ number, not the interrupt vector number!
void DOS_CallOriginalInterrupt(DOS_InterruptHook *hook);
void DOS_UnhookInterrupt(DOS_InterruptHook *hook, bool disable_interrupt);
}
