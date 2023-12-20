#pragma once

#include <stdint.h>
#include <dos.h>

class InterruptGuard {
  int e;

public:
  InterruptGuard() {
    e = disable();
  }

  ~InterruptGuard() {
    if (e)
      enable();
  }
};

/* set_timer:
 *  Sets the delay time for PIT channel 1 in one-shot mode.
 */
void set_timer(uint16_t time) {
  outportb(0x43, 0x30);
  outportb(0x40, time & 0xFFu);
  outportb(0x40, static_cast<uint8_t>(time >> 8u));
}

/* set_timer_rate:
 *  Sets the delay time for PIT channel 1 in cycle mode.
 */
void set_timer_rate(uint16_t time) {
  outportb(0x43, 0x34);
  outportb(0x40, time & 0xFFu);
  outportb(0x40, static_cast<uint8_t>(time >> 8u));
}

/* read_timer:
 *  Reads the elapsed time from PIT channel 1.
 */
uint16_t read_timer() {
  outportb(0x43, 0x00);
  const uint8_t a = inportb(0x40);
  const uint8_t b = inportb(0x40);

  const auto combined = static_cast<uint16_t>((b << 8) | a);

  return (0xFFFFu - combined + 1u) & 0xFFFFu;
}

void timer_disable() {
  outportb(0x21, inportb(0x21) | 1u);
}

void timer_enable() {
  outportb(0x21, inportb(0x21) & static_cast<uint8_t>(~1u));
}
