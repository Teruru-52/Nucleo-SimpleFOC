#include "main_exec.h"
#include "SimpleFOC.h"

// hall sensor instance
HallSensor sensor = HallSensor(HALL_A, HALL_B, HALL_C, pp);

// BLDC motor & driver instance
BLDCMotor motor = BLDCMotor(pp);
BLDCDriver3PWM driver = BLDCDriver3PWM(DRV_EN1, DRV_EN2, DRV_EN3);

// angle set point variable
float target_angle = 0;

// gpio interrupt callback for hall sensors
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    sensor.handleCallback(GPIO_Pin);
}

void setup()
{
    Write_GPIO(LED_LD2, GPIO_PIN_SET);
    resetDriver();

    // initialise hall sensor hardware
    sensor.init();
    // link the motor to the sensor
    motor.linkSensor(&sensor);

    // power supply voltage
    driver.voltage_power_supply = 12;
    driver.init();
    motor.linkDriver(&driver);

    // aligning voltage [V]
    motor.voltage_sensor_align = 3;
    // index search velocity [rad/s]
    motor.velocity_index_search = 3;

    // set motion control loop to be used
    motor.controller = MotionControlType::angle;
    motor.torque_controller = TorqueControlType::voltage;

    // contoller configuration
    // default parameters in defaults.h

    // velocity PI controller parameters
    motor.PID_velocity.P = 0.2f;
    motor.PID_velocity.I = 0.5f;
    motor.PID_velocity.D = 0;
    // default voltage_power_supply
    motor.voltage_limit = 12;
    // jerk control using voltage voltage ramp
    // default value is 300 volts per sec  ~ 0.3V per millisecond
    motor.PID_velocity.output_ramp = 1000;

    // velocity low pass filtering time constant
    motor.LPF_velocity.Tf = 0.01f;

    // angle P controller
    motor.P_angle.P = 5.0f;
    //  maximal velocity of the position control
    motor.velocity_limit = 4;

    // initialize motor
    motor.init();
    // align sensor and start FOC
    motor.initFOC();
    _delay(1000);
    // motor.foc_modulation = FOCModulationType::Trapezoid_150;
    Write_GPIO(LED_LD2, GPIO_PIN_RESET);
}

void resetDriver()
{
    // reset DRV8313 driver
    Write_GPIO(DRV_nRESET, GPIO_PIN_RESET);
    _delay(100);
    Write_GPIO(DRV_nRESET, GPIO_PIN_SET);
    Write_GPIO(DRV_nSLEEP, GPIO_PIN_SET);
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
    motor.move(target_angle);
    static int cnt = 0;
    cnt = (cnt + 1) % 3000;
    if (cnt == 0)
        target_angle = 0;
    else if (cnt == 1000)
        target_angle = M_PI / 2.0f;
    else if (cnt == 2000)
        target_angle = M_PI;
    // target_angle += 0.001;
}