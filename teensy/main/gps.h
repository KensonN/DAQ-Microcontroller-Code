#ifndef GPS_H
#define GPS_H
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>

class GPS {
  public:
    GPS(int device);
    void readValues();
    double get_latitude();
    double get_longitude();
    double get_speed();
  private:
    SFE_UBLOX_GNSS gps;
    double m_latitude;
    double m_longitude;
    double m_speed;
};

#endif
