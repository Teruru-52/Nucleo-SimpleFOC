#include "MagneticSensorSPI.h"

/** Typical configuration for the 14bit AMS AS5147 magnetic sensor over SPI interface */
MagneticSensorSPIConfig_s AS5147_SPI = {
    .bit_resolution = 14,
    .angle_register = 0x3FFF,
    .data_start_bit = 13,
    .command_rw_bit = 14,
    .command_parity_bit = 15};
// AS5048 and AS5047 are the same as AS5147
MagneticSensorSPIConfig_s AS5048_SPI = AS5147_SPI;
MagneticSensorSPIConfig_s AS5047_SPI = AS5147_SPI;

/** Typical configuration for the 14bit MonolithicPower MA730 magnetic sensor over SPI interface */
MagneticSensorSPIConfig_s MA730_SPI = {
    .bit_resolution = 14,
    .angle_register = 0x0000,
    .data_start_bit = 15,
    .command_rw_bit = 0,    // not required
    .command_parity_bit = 0 // parity not implemented
};

// MagneticSensorSPI(GPIO_Value cs, float _bit_resolution, int _angle_register)
//  cs              - SPI chip select pin
//  _bit_resolution   sensor resolution bit number
// _angle_register  - (optional) angle read register - default 0x3FFF
MagneticSensorSPI::MagneticSensorSPI(GPIO_Value cs, int _bit_resolution, int _angle_register)
{

    chip_select_pin = cs;
    // angle read register of the magnetic sensor
    angle_register = _angle_register ? _angle_register : DEF_ANGLE_REGISTER;
    // register maximum value (counts per revolution)
    cpr = _powtwo(_bit_resolution);
    bit_resolution = _bit_resolution;

    command_parity_bit = 15; // for backwards compatibilty
    command_rw_bit = 14;     // for backwards compatibilty
    data_start_bit = 13;     // for backwards compatibilty
}

MagneticSensorSPI::MagneticSensorSPI(MagneticSensorSPIConfig_s config, GPIO_Value cs)
{
    chip_select_pin = cs;
    // angle read register of the magnetic sensor
    angle_register = config.angle_register ? config.angle_register : DEF_ANGLE_REGISTER;
    // register maximum value (counts per revolution)
    cpr = _powtwo(config.bit_resolution);
    bit_resolution = config.bit_resolution;

    command_parity_bit = config.command_parity_bit; // for backwards compatibilty
    command_rw_bit = config.command_rw_bit;         // for backwards compatibilty
    data_start_bit = config.data_start_bit;         // for backwards compatibilty
}

void MagneticSensorSPI::init(SPI_HandleTypeDef *_spi)
{
    spi = _spi;
    Write_GPIO(chip_select_pin, GPIO_PIN_SET);
    this->Sensor::init(); // call base class init
}

//  Shaft angle calculation
//  angle is in radians [rad]
float MagneticSensorSPI::getSensorAngle()
{
    return (getRawCount() / (float)cpr) * _2PI;
}

// function reading the raw counter of the magnetic sensor
int MagneticSensorSPI::getRawCount()
{
    return (int)MagneticSensorSPI::read(angle_register);
}

// SPI functions
/**
 * Utility function used to calculate even parity of uint16_t
 */
uint8_t MagneticSensorSPI::spiCalcEvenParity(uint16_t value)
{
    uint8_t cnt = 0;
    uint8_t i;

    for (i = 0; i < 16; i++)
    {
        if (value & 0x1)
            cnt++;
        value >>= 1;
    }
    return cnt & 0x1;
}

/*
 * Read a register from the sensor
 * Takes the address of the register as a 16 bit uint16_t
 * Returns the value of the register
 */
uint16_t MagneticSensorSPI::read(uint16_t angle_register)
{

    uint16_t command = angle_register;
    uint8_t command_8bit[2];
    uint16_t register_value;
    uint8_t register_value_8bit[2];

    if (command_rw_bit > 0)
    {
        command = angle_register | (1 << command_rw_bit);
    }
    if (command_parity_bit > 0)
    {
        // Add a parity bit on the the MSB
        command |= ((uint16_t)spiCalcEvenParity(command) << command_parity_bit);
    }

    // Send the command
    command_8bit[0] = (command >> 8) & 0xFF;
    command_8bit[1] = command & 0xFF;
    Write_GPIO(chip_select_pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(spi, command_8bit, register_value_8bit, 2, 1);
    Write_GPIO(chip_select_pin, GPIO_PIN_SET);

    register_value = (static_cast<uint16_t>(register_value_8bit[1]) << 8) | static_cast<uint16_t>(register_value_8bit[0]);
    register_value = register_value >> (1 + data_start_bit - bit_resolution); // this should shift data to the rightmost bits of the uint16_t

    const static uint16_t data_mask = 0xFFFF >> (16 - bit_resolution);

    return register_value & data_mask; // Return the data, stripping the non data (e.g parity) bits
}

/**
 * Closes the SPI connection
 * SPI has an internal SPI-device counter, for each init()-call the close() function must be called exactly 1 time
 */
void MagneticSensorSPI::close()
{
    HAL_SPI_MspDeInit(spi);
}