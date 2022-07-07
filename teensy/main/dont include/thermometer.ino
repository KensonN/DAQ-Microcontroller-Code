#include "thermometer.h"
#include <SPI.h>
// https://github.com/sparkfun/MPL3115A2_Breakout/tree/V_H1.1_L1.2.0/Libraries/Arduino
#include <SparkFunLSM9DS1.h>

LSM9DS1 thermometer;

Thermometer::Thermometer(const int &pin): Sensor(pin) {
//  thermometer.begin();
//  thermometer.setModeAltimeter();
//  // recommended setup from sample code
//  thermometer.setOversampleRate(128); // Set Oversample to the recommended 128
//  thermometer.enableEventFlags(); // Enable all three pressure and temp even  
  thermometer.settings.temp.enabled = true;
}

void Thermometer::read_sensor_value() {
  // readTemp() - ºC
  // readTempF() - ºF
  if (thermometer.tempAvailable())
  {
    thermometer.readTemp();
  }
  m_raw_value = thermometer.temperature;
  m_sensor_value = thermometer.temperature;
}
