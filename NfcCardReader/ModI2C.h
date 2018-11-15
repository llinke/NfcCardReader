#ifndef __MOD_I2C_H__
#define __MOD_I2C_H__

// **************************************************
// *** Includes
// **************************************************
#pragma region Includes
#include <Arduino.h>
#include "SerialDebug.h"
#include <vector>
#include <map>
#include <string.h>

#include <Wire.h>
#pragma endregion
// **************************************************

// **************************************************
// *** Variable and Constants Declarations
// **************************************************
#pragma region Variables and Constants
// [I2C]
const int I2C_SCL_PIN = D1;
const int I2C_SCA_PIN = D2;
const int I2C_BUS_SPEED = 100000; // 100kHz for PCF8574
const int I2C_CLK_STRETCH_LIMIT = 1600;
#pragma endregion
// **************************************************

// **************************************************
// *** I2C Helper Methods
// **************************************************
#pragma region I2C Helper Methods
void ScanI2C()
{
    byte error, address;
    int nDevices;

    DEBUG_PRINTLN("I2C: scanning I2C Bus...");
    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
        // The i2c scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            DEBUG_PRINT("I2C: device found at address 0x");
            if (address < 16)
            {
                DEBUG_PRINT("0");
            }
            DEBUG_PRINT(address, HEX);
            DEBUG_PRINTLN(" !");

            nDevices++;
        }
        else if (error == 4)
        {
            DEBUG_PRINT("I2C: Unknown error at address 0x");
            if (address < 16)
            {
                DEBUG_PRINT("0");
            }
            DEBUG_PRINTLN(address, HEX);
        }
    }
    if (nDevices == 0)
    {
        DEBUG_PRINTLN("I2C: No I2C devices found\n");
    }
    else
    {
        DEBUG_PRINTLN("I2C: Done.\n");
    }

    delay(2000);
}

void InitI2C()
{
    DEBUG_PRINTLN("I2C: Initializing Wire");
    Wire.begin(I2C_SCA_PIN, I2C_SCL_PIN);

    DEBUG_PRINTLN("I2C: setting bus speed to " + String(I2C_BUS_SPEED) + ".");
    Wire.setClock(I2C_BUS_SPEED);

    DEBUG_PRINTLN("I2C: setting clock stretch limit to " + String(I2C_CLK_STRETCH_LIMIT) + ".");
    Wire.setClockStretchLimit(I2C_CLK_STRETCH_LIMIT);

    ScanI2C();
}
#pragma endregion
// **************************************************

#endif