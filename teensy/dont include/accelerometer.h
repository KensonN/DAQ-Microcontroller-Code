#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H
#include "cartesian_sensor.h"

class Accelerometer: public Cartesian_Sensor
{
public:
    Accelerometer(const int &pin);

    void read_sensor_value();
};

#endif
