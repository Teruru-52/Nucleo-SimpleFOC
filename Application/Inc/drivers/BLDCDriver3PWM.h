#ifndef BLDCDriver3PWM_h
#define BLDCDriver3PWM_h

#include "main.h"
#include "../common/base_classes/BLDCDriver.h"
#include "../common/foc_utils.h"
#include "../common/time_utils.h"
#include "../common/defaults.h"
#include "hardware_api.h"

/**
 3 pwm bldc driver class
*/
class BLDCDriver3PWM : public BLDCDriver
{
public:
  /**
    BLDCDriver class constructor
    @param en1 enable pin (optional input)
    @param en2 enable pin (optional input)
    @param en3 enable pin (optional input)
  */
  BLDCDriver3PWM(GPIO_Value en1, GPIO_Value en2, GPIO_Value en3);

  /**  Motor hardware init function */
  int init() override;
  /** Motor disable function */
  void disable() override;
  /** Motor enable function */
  void enable() override;

  // hardware variables
  int pwmA;               //!< phase A pwm pin number
  int pwmB;               //!< phase B pwm pin number
  int pwmC;               //!< phase C pwm pin number
  GPIO_Value enableA_pin; //!< enable pin number
  GPIO_Value enableB_pin; //!< enable pin number
  GPIO_Value enableC_pin; //!< enable pin number

  /**
   * Set phase voltages to the hardware
   *
   * @param Ua - phase A voltage
   * @param Ub - phase B voltage
   * @param Uc - phase C voltage
   */
  void setPwm(float Ua, float Ub, float Uc) override;

  /**
   * Set phase voltages to the hardware
   * > Only possible is the driver has separate enable pins for all phases!
   *
   * @param sc - phase A state : active / disabled ( high impedance )
   * @param sb - phase B state : active / disabled ( high impedance )
   * @param sa - phase C state : active / disabled ( high impedance )
   */
  virtual void setPhaseState(PhaseState sa, PhaseState sb, PhaseState sc) override;

private:
};

#endif
