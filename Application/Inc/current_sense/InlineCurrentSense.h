#ifndef INLINE_CS_LIB_H
#define INLINE_CS_LIB_H

#include "../common/foc_utils.h"
#include "../common/time_utils.h"
#include "../common/defaults.h"
#include "../common/base_classes/CurrentSense.h"
#include "../common/base_classes/BLDCDriver.h"
#include "../common/lowpass_filter.h"
#include "hardware_api.h"

class InlineCurrentSense : public CurrentSense
{
public:
  /**
    InlineCurrentSense class constructor
    @param shunt_resistor shunt resistor value
    @param gain current-sense op-amp gain
    @param phA A phase adc pin
    @param phB B phase adc pin
    @param phC C phase adc pin (optional)
  */
  InlineCurrentSense(float shunt_resistor, float gain, int pinC = NOT_SET);
  /**
    InlineCurrentSense class constructor
    @param mVpA mV per Amp ratio
    @param phA A phase adc pin
    @param phB B phase adc pin
    @param phC C phase adc pin (optional)
  */
  InlineCurrentSense(float mVpA, int pinC = NOT_SET);

  // CurrentSense interface implementing functions
  int init(ADC_HandleTypeDef *_adc) override;
  PhaseCurrent_s getPhaseCurrents() override;

private:
  // gain variables
  float shunt_resistor;      //!< Shunt resistor value
  float amp_gain;            //!< amp gain value
  float volts_to_amps_ratio; //!< Volts to amps ratio
  uint16_t adc_value[3];

  ADC_HandleTypeDef *adc;

  /**
   *  Function finding zero offsets of the ADC
   */
  void calibrateOffsets();
};

#endif