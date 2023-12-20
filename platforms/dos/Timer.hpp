#pragma once

#include <cstdint>
#include <chrono>

void timer_init();
std::chrono::milliseconds timer_since_start();
void timer_shutdown();
