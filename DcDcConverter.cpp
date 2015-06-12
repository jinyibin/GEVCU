/*
 * DcDcConverter.cpp
 *
 * Parent class for all DC-DC Converters
 *
 Copyright (c) 2013 Collin Kidder, Michael Neuweiler, Charles Galpin

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 */

#include "DcDcConverter.h"

DcDcConverter::DcDcConverter() : Device() {
    hvVoltage = 0;
    lvVoltage = 0;
    hvCurrent = 0;
    lvCurrent = 0;
    temperature = 0;
}

DcDcConverter::~DcDcConverter() {
}

/*
 * Return the device type
 */
DeviceType DcDcConverter::getType() {
    return (DEVICE_DCDC);
}

/**
 * act on messages the super-class does not react upon, like state change
 * to ready or running which should enable/disable the power-stage of the converter
 */
void DcDcConverter::handleStateChange(Status::SystemState oldState, Status::SystemState newState) {
    Device::handleStateChange(oldState, newState);
    if (newState == Status::ready || newState == Status::charging || newState == Status::charged
            || newState == Status::running || newState == Status::batteryHeating) {
        powerOn = true;
    } else {
        powerOn = false;
    }
    systemIO->setEnableDcDc(powerOn);
}

/*
 * Load configuration data from EEPROM.
 *
 * If not available or the checksum is invalid, default values are chosen.
 */
void DcDcConverter::loadConfiguration()
{
    DcDcConverterConfiguration *config = (DcDcConverterConfiguration *) getConfiguration();

    if (!config) { // as lowest sub-class make sure we have a config object
        config = new DcDcConverterConfiguration();
        setConfiguration(config);
    }

    Device::loadConfiguration(); // call parent
    Logger::info(getId(), "DC-DC converter configuration:");

#ifdef USE_HARD_CODED

    if (false) {
#else
    if (prefsHandler->checksumValid()) { //checksum is good, read in the values stored in EEPROM
#endif
        uint8_t temp;
        prefsHandler->read(DCDC_BOOST_MODE, &config->mode);
        prefsHandler->read(DCDC_LOW_VOLTAGE, &config->lowVoltageCommand);
        prefsHandler->read(DCDC_HV_UNDERVOLTAGE_LIMIT, &config->hvUndervoltageLimit);
        prefsHandler->read(DCDC_LV_BUCK_CURRENT_LIMIT, &config->lvBuckModeCurrentLimit);
        prefsHandler->read(DCDC_HV_BUCK_CURRENT_LIMIT, &config->hvBuckModeCurrentLimit);
        prefsHandler->read(DCDC_HIGH_VOLTAGE, &config->highVoltageCommand);
        prefsHandler->read(DCDC_LV_UNDERVOLTAGE_LIMIT, &config->lvUndervoltageLimit);
        prefsHandler->read(DCDC_LV_BOOST_CURRENT_LIMIT, &config->lvBoostModeCurrentLinit);
        prefsHandler->read(DCDC_HV_BOOST_CURRENT_LIMIT, &config->hvBoostModeCurrentLimit);
    } else { //checksum invalid. Reinitialize values and store to EEPROM
        config->mode = false; // default mode: buck, meaning: reduce from HV to LV
        config->lowVoltageCommand = 135; // 13.5 V
        config->hvUndervoltageLimit = 330; // 330 V
        config->lvBuckModeCurrentLimit = 250; // 250 A
        config->hvBuckModeCurrentLimit = 200; // 20.0 A
        config->highVoltageCommand = 0;
        config->lvUndervoltageLimit = 0;
        config->lvBoostModeCurrentLinit = 0;
        config->hvBoostModeCurrentLimit = 0;
        saveConfiguration();
    }

    Logger::info(getId(), "operation mode: %s", (config->mode ? "boost" : "buck"));
    Logger::info(getId(), "buck : LV command %fV (max %dA), HV min %dV (max %fA)", (float) config->lowVoltageCommand / 10.0F, config->lvBuckModeCurrentLimit, config->hvUndervoltageLimit, (float) config->hvBuckModeCurrentLimit / 10.0F);
    Logger::info(getId(), "boost: HV command %dV (max %fA), LV min %fV (max %dA)", config->highVoltageCommand, (float) config->hvBoostModeCurrentLimit / 10.0, (float) config->lvUndervoltageLimit / 10.0F, config->lvBoostModeCurrentLinit);
}

/*
 * Store the current configuration parameters to EEPROM.
 */
void DcDcConverter::saveConfiguration()
{
    DcDcConverterConfiguration *config = (DcDcConverterConfiguration *) getConfiguration();

    Device::saveConfiguration(); // call parent

    prefsHandler->write(DCDC_BOOST_MODE, config->mode);
    prefsHandler->write(DCDC_LOW_VOLTAGE, config->lowVoltageCommand);
    prefsHandler->write(DCDC_HV_UNDERVOLTAGE_LIMIT, config->hvUndervoltageLimit);
    prefsHandler->write(DCDC_LV_BUCK_CURRENT_LIMIT, config->lvBuckModeCurrentLimit);
    prefsHandler->write(DCDC_HV_BUCK_CURRENT_LIMIT, config->hvBuckModeCurrentLimit);
    prefsHandler->write(DCDC_HIGH_VOLTAGE, config->highVoltageCommand);
    prefsHandler->write(DCDC_LV_UNDERVOLTAGE_LIMIT, config->lvUndervoltageLimit);
    prefsHandler->write(DCDC_LV_BOOST_CURRENT_LIMIT, config->lvBoostModeCurrentLinit);
    prefsHandler->write(DCDC_HV_BOOST_CURRENT_LIMIT, config->hvBoostModeCurrentLimit);

    prefsHandler->saveChecksum();
}
