#include "time_utils.h"

// function buffering delay()
// arduino uno function doesn't work well with interrupts
void _delay(unsigned long ms)
{
  // unsigned long t = _micros() + ms * 1000;
  // while (_micros() < t)
  // {
  // };
  // regular micros
  HAL_Delay(ms);
}

// function buffering _micros()
// arduino function doesn't work well with interrupts
unsigned long _micros()
{
  // regular micros
  return micros();
}
