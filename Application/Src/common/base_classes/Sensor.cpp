#include "Sensor.h"
#include "../foc_utils.h"
#include "../time_utils.h"

void Sensor::update()
{
    float val = getSensorAngle();
    if (val < 0) // sensor angles are strictly non-negative. Negative values are used to signal errors.
        return;  // TODO signal error, e.g. via a flag and counter
    angle_prev_ts = _micros();
    float d_angle = val - angle_prev;
    // if overflow happened track it as full rotation
    if (abs(d_angle) > (0.8f * _2PI))
        full_rotations += (d_angle > 0) ? -1 : 1;
    angle_prev = val;
    // printf("angle_val: %.3f\n", val);
}

/** get current angular velocity (rad/s) */
float Sensor::getVelocity()
{
    // calculate sample time
    // float Ts = (angle_prev_ts - vel_angle_prev_ts) * 1e-6f;
    float Ts = 0.001;
    if (Ts < 0.0f)
    { // handle micros() overflow - we need to reset vel_angle_prev_ts
        vel_angle_prev = angle_prev;
        vel_full_rotations = full_rotations;
        vel_angle_prev_ts = angle_prev_ts;
        return velocity;
    }
    if (Ts < min_elapsed_time)
        return velocity; // don't update velocity if deltaT is too small

    velocity = ((float)(full_rotations - vel_full_rotations) * _2PI + (angle_prev - vel_angle_prev)) / Ts;
    vel_angle_prev = angle_prev;
    vel_full_rotations = full_rotations;
    vel_angle_prev_ts = angle_prev_ts;
    return velocity;
}

void Sensor::init()
{
    // initialize all the internal variables of Sensor to ensure a "smooth" startup (without a 'jump' from zero)
    getSensorAngle(); // call once
    _delay(1);
    vel_angle_prev = getSensorAngle(); // call again
    vel_angle_prev_ts = _micros();
    _delay(1);
    getSensorAngle(); // call once
    _delay(1);
    angle_prev = getSensorAngle(); // call again
    angle_prev_ts = _micros();
    printf("vel_angle_prev: %.3f\n", vel_angle_prev);
    printf("vel_angle_prev_ts: %ld\n", vel_angle_prev_ts);
    printf("angle_prev: %.3f\n", angle_prev);
    printf("angle_prev_ts: %ld\n", angle_prev_ts);
}

float Sensor::getMechanicalAngle()
{
    return angle_prev;
}

float Sensor::getAngle()
{
    return (float)full_rotations * _2PI + angle_prev;
}

double Sensor::getPreciseAngle()
{
    return (double)full_rotations * (double)_2PI + (double)angle_prev;
}

int32_t Sensor::getFullRotations()
{
    return full_rotations;
}

int Sensor::needsSearch()
{
    return 0; // default false
}
