#include "gps.h"
#include <Wire.h>

SFE_UBLOX_GNSS gpsObject;

GPS::GPS(int device){
  Wire.begin();
  gps = gpsObject;
  if (gps.begin() == false) //Connect to the Ublox module using Wire port
  {
    Serial.println(F("Ublox GPS not detected at default I2C address. Please check wiring. Freezing."));
    return;
  }
  Serial.println("GPS initialized");
  gps.setI2COutput(COM_TYPE_UBX);
  gps.setNavigationFrequency(0.25);
  gps.setAutoPVT(true);
  m_latitude = 0;
  m_longitude = 0;
  m_speed = 0;
}

void GPS::readValues() {
  if (gps.getPVT() == true && (gps.getInvalidLlh() == false)) {
    m_latitude = gps.getLatitude() / 1E7;
    m_longitude = gps.getLongitude() / 1E7;
    m_speed = gps.getGroundSpeed() / 447.040496;
  }
}

double GPS::get_latitude() {
  return m_latitude;
}

double GPS::get_longitude() {
  return m_longitude;
}

double GPS::get_speed() {
  return m_speed;
}
