
#pragma once
#include <chrono>

bool InitFixedTimer();
void QuitFixedTimer();
void WaitForNextFixedTick();
std::chrono::microseconds ElapsedFixedTime();
