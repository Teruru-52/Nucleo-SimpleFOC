#include "main_exec.h"
#include "SimpleFOC.h"

// magnetic sensor instance - SPI
MagneticSensorSPI sensor = MagneticSensorSPI(AS5048_SPI, SPI3_CS);

// BLDC motor & driver instance
BLDCMotor motor = BLDCMotor(14);
BLDCDriver3PWM driver = BLDCDriver3PWM(DRV_en1, DRV_en2, DRV_en3);

// current sensor
InlineCurrentSense current_sense = InlineCurrentSense(0.01f, 50.0f);

// voltage or torque set point variable
float target = 2;

void setup()
{
    // initialise magnetic sensor hardware
    sensor.init(&hspi3);
    // link the motor to the sensor
    motor.linkSensor(&sensor);

    // power supply voltage
    driver.voltage_power_supply = 12;
    driver.init();
    motor.linkDriver(&driver);

    // voltage control (default)
    // aligning voltage
    motor.voltage_sensor_align = 5;

    // foc_current control
    // current sense init hardware
    current_sense.init(&hadc1);
    // link the current sense to the motor
    // motor.linkCurrentSense(&current_sense);
    // set torque mode: voltage, dc_current, foc_current
    // motor.torque_controller = TorqueControlType::foc_current;

    // choose FOC modulation (optional)
    motor.foc_modulation = FOCModulationType::SpaceVectorPWM;
    // set motion control loop to be used
    motor.controller = MotionControlType::torque;

    // initialize motor
    motor.init();
    // align sensor and start FOC
    motor.initFOC();
    _delay(1000);
}

void timerCallback()
{
    // main FOC algorithm function
    // the faster you run this function the better
    // Arduino UNO loop  ~1kHz
    // Bluepill loop ~10kHz
    motor.loopFOC();

    // Motion control function
    // velocity, position or voltage (defined in motor.controller)
    // this function can be run at much lower frequency than loopFOC() function
    // You can also use motor.move() and set the motor.target in the code
    motor.move(target);
}