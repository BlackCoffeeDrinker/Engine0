
#include "timer.hpp"

#include "interrupt.hpp"
#include "memory.hpp"

#include <chrono>

// The exact base frequency of the 8253/8254 PIT chip
constexpr unsigned long PIT_BASE_FREQUENCY = 1193182;
constexpr unsigned int MULTIPLIER = 4;
constexpr int TIMER_INT = 0;

constexpr unsigned short hz_to_pit_divisor(double hz) {
  // Prevent divide-by-zero or negative frequencies
  if (hz <= 0.0) return 0;

  // Calculate divisor with standard rounding (+0.5)
  auto divisor = static_cast<unsigned long>((PIT_BASE_FREQUENCY / hz) + 0.5);

  // Windows 95 DOS Box Safety Cap: Never exceed ~1,000 Hz (divisor 1193)
  if (divisor < 1193) {
    return 1193;
  }

  // PIT counter registers are 16-bit maximum (65535)
  if (divisor > 65535) return 0;// 0 wraps around to 65536 natively

  return static_cast<unsigned short>(divisor);
}

constexpr unsigned short multiplier_to_pit_divisor(unsigned int multiplier) {
  if (multiplier == 0) return 0;

  // The default BIOS divisor is 65536 (represented as 0 in hardware)
  unsigned long divisor = 65536 / multiplier;

  // Windows 95 DOS Box Safety Cap (45x multiplier max, approx 820 Hz)
  if (divisor < 1193) {
    return 1193;
  }

  return static_cast<unsigned short>(divisor);
}

constexpr unsigned short PIT_DIVISOR = multiplier_to_pit_divisor(MULTIPLIER);

static volatile int tick_count = 0;
static DOS_InterruptHook handle_timer_tick_hook;

/* set_timer_rate:
 *  Sets the delay time for PIT channel 0 in cycle mode.
 */
static void set_timer_rate(long time) {
  outportb(0x43, 0x34);
  outportb(0x40, time & 0xff);
  outportb(0x40, time >> 8);
}

/* fixed_timer_handler:
 *  Interrupt handler for the fixed-rate timer driver.
 *  will be called automatically every 18.2 ms
 */
static void fixed_timer_handler() {
  tick_count = tick_count + 1;
  if ((tick_count % MULTIPLIER) == 0) {
    DOS_CallOriginalInterrupt(&handle_timer_tick_hook);
  }
  DOS_EndOfInterrupt(TIMER_INT);
}

static void fixed_timer_handler_end() {}// end-of-ISR label for memory locking

/* fixed_timer_init:
 *  Installs the fixed-rate timer driver.
 */
bool InitFixedTimer() {
  DOS_LockVariable(tick_count);
  DOS_LockCode(fixed_timer_handler, fixed_timer_handler_end);

  tick_count = 0;

  DOS_DisableInterrupts();
  const bool success = DOS_HookInterrupt(TIMER_INT, fixed_timer_handler, &handle_timer_tick_hook);
  for (int i = 0; i < 4; i++) {
    set_timer_rate(PIT_DIVISOR);
  }
  DOS_EnableInterrupts();

  return success;
}

/* fixed_timer_exit:
 *  Shuts down the fixed-rate timer driver.
 */
void QuitFixedTimer() {
  DOS_DisableInterrupts();

  for (int i = 0; i < 4; i++) {
    set_timer_rate(0);
  }

  tick_count = 0;

  DOS_UnhookInterrupt(&handle_timer_tick_hook, false);
  DOS_EnableInterrupts();
}

void WaitForNextFixedTick() {
  const auto current_tick = tick_count;
  while (current_tick == tick_count) {
    asm volatile("" ::: "memory");// Tells GCC to reload update_required from RAM every iteration
  }
}

std::chrono::microseconds ElapsedFixedTime() {
  // 18200 us is the base tick, divided by 4 (MULTIPLIER) = 4550 us per tick
  constexpr unsigned long US_PER_TICK = 18200 / MULTIPLIER;

  // Atomic read on 32-bit x86, perfectly safe without masking interrupts
  return std::chrono::microseconds(tick_count * US_PER_TICK);
}
