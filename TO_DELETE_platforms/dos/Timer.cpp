#include "Timer.hpp"

#include <dpmi.h>
#include <cstdint>
#include <go32.h>
#include <dos.h>

constexpr auto TIMER_ISR = 8;

namespace {
_go32_dpmi_seginfo timerOldISR, timerNewISR;

volatile uint32_t ticks;

/***
 * fixed_timer_handler:
 *  Interrupt handler for the fixed-rate timer driver.
 */
int fixed_timer_handler() {
  ++ticks;
  return 0;
}
}// namespace

void timer_init() {
  ticks = 0;

  _go32_dpmi_lock_data((void *)&ticks, sizeof(ticks));
  _go32_dpmi_lock_code((void *)&fixed_timer_handler, 4096);
  _go32_dpmi_get_protected_mode_interrupt_vector(TIMER_ISR, &timerOldISR);
  timerNewISR.pm_offset = (unsigned long)&fixed_timer_handler;
  timerNewISR.pm_selector = (unsigned short)_go32_my_cs();
  _go32_dpmi_chain_protected_mode_interrupt_vector(TIMER_ISR, &timerNewISR);
}

uint32_t timer_ticks() {
  return ticks;
}

void timer_shutdown() {
  _go32_dpmi_set_protected_mode_interrupt_vector(TIMER_ISR, &timerOldISR);
}
std::chrono::milliseconds timer_since_start() {
  disable();
  const auto t = ticks;
  enable();

  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::microseconds(54925 * t));
}
