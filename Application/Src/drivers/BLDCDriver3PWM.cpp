#include "BLDCDriver3PWM.h"

BLDCDriver3PWM::BLDCDriver3PWM(GPIO_Value en1, GPIO_Value en2, GPIO_Value en3)
{
  // enable_pin pin
  enableA_pin = en1;
  enableB_pin = en2;
  enableC_pin = en3;

  // default power-supply value
  voltage_power_supply = DEF_POWER_SUPPLY;
  voltage_limit = NOT_SET;
}

// enable motor driver
void BLDCDriver3PWM::enable()
{
  // enable_pin the driver
  Write_GPIO(enableA_pin, GPIO_PIN_SET);
  Write_GPIO(enableB_pin, GPIO_PIN_SET);
  Write_GPIO(enableC_pin, GPIO_PIN_SET);
  // set zero to PWM
  setPwm(0, 0, 0);
}

// disable motor driver
void BLDCDriver3PWM::disable()
{
  // set zero to PWM
  setPwm(0, 0, 0);
  // disable the driver
  Write_GPIO(enableA_pin, GPIO_PIN_RESET);
  Write_GPIO(enableB_pin, GPIO_PIN_RESET);
  Write_GPIO(enableC_pin, GPIO_PIN_RESET);
}

// init hardware pins
int BLDCDriver3PWM::init()
{
  // sanity check for the voltage limit configuration
  if (!_isset(voltage_limit) || voltage_limit > voltage_power_supply)
    voltage_limit = voltage_power_supply;

  // Set the pwm frequency to the pins
  // hardware specific function - depending on driver and mcu
  _configure3PWM();
  printf("MOT: Initialize timer\n");
  initialized = true;
  return 1;
}

// Set voltage to the pwm pin
void BLDCDriver3PWM::setPhaseState(PhaseState sa, PhaseState sb, PhaseState sc)
{
  // disable if needed
  Write_GPIO(enableA_pin, sa == PhaseState::PHASE_ON ? GPIO_PIN_SET : GPIO_PIN_RESET);
  Write_GPIO(enableB_pin, sb == PhaseState::PHASE_ON ? GPIO_PIN_SET : GPIO_PIN_RESET);
  Write_GPIO(enableC_pin, sc == PhaseState::PHASE_ON ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// Set voltage to the pwm pin
void BLDCDriver3PWM::setPwm(float Ua, float Ub, float Uc)
{

  // limit the voltage in driver
  Ua = _constrain(Ua, 0.0f, voltage_limit);
  Ub = _constrain(Ub, 0.0f, voltage_limit);
  Uc = _constrain(Uc, 0.0f, voltage_limit);
  // calculate duty cycle
  // limited in [0,1]
  dc_a = _constrain(Ua / voltage_power_supply, 0.0f, 1.0f);
  dc_b = _constrain(Ub / voltage_power_supply, 0.0f, 1.0f);
  dc_c = _constrain(Uc / voltage_power_supply, 0.0f, 1.0f);

  // hardware specific writing
  // hardware specific function - depending on driver and mcu
  _writeDutyCycle3PWM(dc_a, dc_b, dc_c);
  // printf("dc_a: %.3f, dc_b: %.3f, dc_c: %.3f\n", dc_a, dc_b, dc_c);
}
