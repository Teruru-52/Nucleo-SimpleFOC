#include "HallSensor.h"

/*
  HallSensor(int hallA, int hallB , int cpr, int index)
  - hallA, hallB, hallC    - HallSensor A, B and C pins
  - pp           - pole pairs
*/

HallSensor::HallSensor(GPIO_Value _hallA, GPIO_Value _hallB, GPIO_Value _hallC, int _pp)
{

  // hardware pins
  pinA = _hallA;
  pinB = _hallB;
  pinC = _hallC;

  // hall has 6 segments per electrical revolution
  cpr = _pp * 6;

  // extern pullup as default
  // pullup = Pullup::USE_EXTERN;
}

//  HallSensor interrupt callback functions
void HallSensor::handleCallback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == pinA.GPIO_PIN_x)
    A_active = Read_GPIO(pinA);
  if (GPIO_Pin == pinB.GPIO_PIN_x)
    B_active = Read_GPIO(pinB);
  if (GPIO_Pin == pinC.GPIO_PIN_x)
    C_active = Read_GPIO(pinC);
  updateState();
}

/**
 * Updates the state and sector following an interrupt
 */
void HallSensor::updateState()
{
  int8_t new_hall_state = C_active + (B_active << 1) + (A_active << 2);

  // glitch avoidance #1 - sometimes we get an interrupt but pins haven't changed
  if (new_hall_state == hall_state)
    return;

  long new_pulse_timestamp = _micros();
  hall_state = new_hall_state;

  int8_t new_electric_sector = ELECTRIC_SECTORS[hall_state];
  int8_t electric_sector_dif = new_electric_sector - electric_sector;
  if (electric_sector_dif > 3)
  {
    // underflow
    direction = Direction::CCW;
    electric_rotations += direction;
  }
  else if (electric_sector_dif < (-3))
  {
    // overflow
    direction = Direction::CW;
    electric_rotations += direction;
  }
  else
  {
    direction = (new_electric_sector > electric_sector) ? Direction::CW : Direction::CCW;
  }
  electric_sector = new_electric_sector;

  // glitch avoidance #2 changes in direction can cause velocity spikes.  Possible improvements needed in this area
  if (direction == old_direction)
  {
    // not oscilating or just changed direction
    pulse_diff = new_pulse_timestamp - pulse_timestamp;
  }
  else
  {
    pulse_diff = 0;
  }

  pulse_timestamp = new_pulse_timestamp;
  total_interrupts++;
  old_direction = direction;
  if (onSectorChange != nullptr)
    onSectorChange(electric_sector);
}

/**
 * Optionally set a function callback to be fired when sector changes
 * void onSectorChange(int sector) {
 *  ... // for debug or call driver directly?
 * }
 * sensor.attachSectorCallback(onSectorChange);
 */
void HallSensor::attachSectorCallback(void (*_onSectorChange)(int sector))
{
  onSectorChange = _onSectorChange;
}

// Sensor update function. Safely copy volatile interrupt variables into Sensor base class state variables.
void HallSensor::update()
{
  angle_prev_ts = pulse_timestamp;
  long last_electric_rotations = electric_rotations;
  int8_t last_electric_sector = electric_sector;
  angle_prev = ((float)((last_electric_rotations * 6 + last_electric_sector) % cpr) / (float)cpr) * _2PI;
  full_rotations = (int32_t)((last_electric_rotations * 6 + last_electric_sector) / cpr);
}

/*
  Shaft angle calculation
  TODO: numerical precision issue here if the electrical rotation overflows the angle will be lost
*/
float HallSensor::getSensorAngle()
{
  return ((float)(electric_rotations * 6 + electric_sector) / (float)cpr) * _2PI;
}

/*
  Shaft velocity calculation
  function using mixed time and frequency measurement technique
*/
float HallSensor::getVelocity()
{
  long last_pulse_timestamp = pulse_timestamp;
  long last_pulse_diff = pulse_diff;
  if (last_pulse_diff == 0 || ((long)(_micros() - last_pulse_timestamp) > last_pulse_diff * 2))
  { // last velocity isn't accurate if too old
    return 0;
  }
  else
  {
    return direction * (_2PI / (float)cpr) / (last_pulse_diff / 1000000.0f);
  }
}

// HallSensor initialisation of the hardware pins
// and calculation variables
void HallSensor::init()
{
  // initialise the electrical rotations to 0
  electric_rotations = 0;

  // init hall_state
  A_active = Read_GPIO(pinA);
  B_active = Read_GPIO(pinB);
  C_active = Read_GPIO(pinC);
  updateState();

  pulse_timestamp = _micros();

  // we don't call Sensor::init() here because init is handled in HallSensor class.
}
