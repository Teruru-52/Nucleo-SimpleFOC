#include "LowsideCurrentSense.h"
// #include "communication/SimpleFOCDebug.h"

// LowsideCurrentSensor constructor
//  - shunt_resistor  - shunt resistor value
//  - gain  - current-sense op-amp gain
//  - phA   - A phase adc pin
//  - phB   - B phase adc pin
//  - phC   - C phase adc pin (optional)
LowsideCurrentSense::LowsideCurrentSense(float _shunt_resistor, float _gain, int _pinC)
{
    pinC = _pinC;

    shunt_resistor = _shunt_resistor;
    amp_gain = _gain;
    volts_to_amps_ratio = 1.0f / _shunt_resistor / _gain; // volts to amps
    // gains for each phase
    gain_a = volts_to_amps_ratio;
    gain_b = volts_to_amps_ratio;
    gain_c = volts_to_amps_ratio;
}

LowsideCurrentSense::LowsideCurrentSense(float _mVpA, int _pinC)
{
    pinC = _pinC;

    volts_to_amps_ratio = 1000.0f / _mVpA; // mV to amps
    // gains for each phase
    gain_a = volts_to_amps_ratio;
    gain_b = volts_to_amps_ratio;
    gain_c = volts_to_amps_ratio;
}

// Lowside sensor init function
int LowsideCurrentSense::init(ADC_HandleTypeDef *_adc)
{
    adc = _adc;
    HAL_ADC_Start_DMA(adc, (uint32_t *)adc_value, 3);
    if (driver == nullptr)
    {
        // SIMPLEFOC_DEBUG("CUR: Driver not linked!");
        return 0;
    }

    if (driver_type == DriverType::BLDC)
        static_cast<BLDCDriver *>(driver)->setPwm(driver->voltage_limit / 2, driver->voltage_limit / 2, driver->voltage_limit / 2);
    // calibrate zero offsets
    calibrateOffsets();
    // set zero voltage to all phases
    if (driver_type == DriverType::BLDC)
        static_cast<BLDCDriver *>(driver)->setPwm(0, 0, 0);
    // set the initialized flag
    initialized = true;
    // return success
    return 1;
}
// Function finding zero offsets of the ADC
void LowsideCurrentSense::calibrateOffsets()
{
    const int calibration_rounds = 2000;

    // find adc offset = zero current voltage
    offset_ia = 0;
    offset_ib = 0;
    offset_ic = 0;
    // read the adc voltage 1000 times ( arbitrary number )
    for (int i = 0; i < calibration_rounds; i++)
    {
        // _startADC3PinConversionLowSide();
        offset_ia += (adc_value[0] * ADC_REF_VOLTAGE / ADC_RESOLUTION);
        offset_ib += (adc_value[1] * ADC_REF_VOLTAGE / ADC_RESOLUTION);
        if (_isset(pinC))
            offset_ic += (adc_value[2] * ADC_REF_VOLTAGE / ADC_RESOLUTION);
        _delay(1);
    }
    // calculate the mean offsets
    offset_ia = offset_ia / calibration_rounds;
    offset_ib = offset_ib / calibration_rounds;
    if (_isset(pinC))
        offset_ic = offset_ic / calibration_rounds;
}

// read all three phase currents (if possible 2 or 3)
PhaseCurrent_s LowsideCurrentSense::getPhaseCurrents()
{
    PhaseCurrent_s current;
    // _startADC3PinConversionLowSide();
    current.a = (adc_value[0] * ADC_REF_VOLTAGE / ADC_RESOLUTION - offset_ia) * gain_a;                       // amps
    current.b = (adc_value[1] * ADC_REF_VOLTAGE / ADC_RESOLUTION - offset_ib) * gain_b;                       // amps
    current.c = (!_isset(pinC)) ? 0 : (adc_value[2] * ADC_REF_VOLTAGE / ADC_RESOLUTION - offset_ic) * gain_c; // amps
    return current;
}
