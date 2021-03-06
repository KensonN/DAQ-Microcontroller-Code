#include <Wire.h>
#include <SPI.h>

//#include "altimeter.h"
//#include "thermometer.h"
#include "gps.h"
#include "gyroscope.h"
#include "accelerometer.h"
//#include "thermometer.h"

//Function definitions
void read_all_sensors();
void print_all_sensors();

const int num_sensors = 3;
Sensor* mySensors[num_sensors];
//Altimeter* myAltimeter;
GPS* myGps;
Gyroscope* myGyroscope;
Accelerometer* myAccelerometer;
//Thermometer* myThermometer;

int hallEffectSensorPin = 0; // Hall sensor at pin 0
unsigned int countsHES;
unsigned int rpm; //unsigned gives only positive values
unsigned long previoustimeHES;

void count_function()
{ /*The ISR function
Called on Interrupt
Update counts*/
countsHES++;
}

void setup() {
  Serial.begin(115200);  // Start serial for output
  Serial1.begin(115200);  // Start serial for output
  Wire.begin();
//  myAltimeter = new Altimeter(0x6A);
//  mySensors[0] = myAltimeter;
  myGps = new GPS(0x6B);
  mySensors[0] = myGps;
  myGyroscope = new Gyroscope(0x60);
  mySensors[1] = myGyroscope;
  myAccelerometer = new Accelerometer(0x60);
  mySensors[2] = myAccelerometer;
//  myThermometer = new Thermometer(0x60);
//  mySensors[4] = myThermometer;

  //Intiates Serial communications
  attachInterrupt(digitalPinToInterrupt(hallEffectSensorPin), count_function, RISING); //Interrupts are called on Rise of Input
  pinMode(hallEffectSensorPin, INPUT); //Sets sensor as input
  countsHES = 0;
  rpm = 0;
  previoustimeHES = 0; //Initialise the values
  
  delay(1);
}

void loop() {
  read_all_sensors();
  detachInterrupt(0); //Interrupts are disabled
  rpm = 60*1000/(millis() - previoustimeHES)*countsHES;
  previoustimeHES = millis(); //Resets the clock
  countsHES = 0; //Resets the counter
  
  print_all_sensors();
  
  attachInterrupt(digitalPinToInterrupt(hallEffectSensorPin), count_function, RISING); //Counter restarted
}

void read_all_sensors() {
//  int time = millis();
  for (int i = 0; i < num_sensors; i++) {
    mySensors[i]->read_sensor_value();
//    Serial1.println(millis()-time);
//    time = millis();
  }
}
int count = 0;
void print_all_sensors() {
  //prints a single string containing all sensor values
  //to be decoded by python script

  //expect precision issues to # of digits
  //python script currently expects 2 digits per datum

  //GPS
  Serial1.print("^latitude:");
  Serial1.println(myGps->get_latitude());
//  Serial1.print(";");
  Serial1.print("^longitude:");
  Serial1.println(myGps->get_longitude());
//  Serial1.print(";");

  //Gyroscope
  Cartesian_Coordinates GyCoordinates = myGyroscope->get_sensor_coordinates();
  Serial1.print("^GyX:");
  Serial1.println(GyCoordinates.x);
//  Serial1.print(";");
  Serial1.print("^GyY:");
  Serial1.println(GyCoordinates.y);
//  Serial1.print(";");
  Serial1.print("^GyZ:");
  Serial1.println(GyCoordinates.z);
//  Serial1.print(";");

//  Serial1.print("Alt:");
//  Serial1.print(myAltimeter->get_raw_value()+27);
//  Serial1.print(";");
//  Serial1.print("Tem:");
//  Serial1.print(myAltimeter->get_sensor_value()-11);
//  Serial1.print(";");

  //hall effect
//  Serial1.print("Rpm:");
//  Serial1.print(rpm); //Calculated values are displayed
//  Serial1.print(";");
  Serial1.print("^speed:");
  Serial1.println(myGps->get_speed());
//  Serial1.print(";");
//
  Cartesian_Coordinates AcCoordinates = myAccelerometer->get_sensor_coordinates();
  Serial1.print("^acceleration x:");
  Serial1.println(AcCoordinates.x);
//  Serial1.print(";");
  Serial1.print("^acceleration y:");
  Serial1.println(AcCoordinates.y);
//  Serial1.print(";");
  Serial1.print("^acceleration z:");
  Serial1.println(AcCoordinates.z);
//  Serial1.print(";");
//  Serial.print("Count:");
//  Serial.print(count);
//  Serial.print(";");
//  Serial.print("Time:");
//  Serial.print(millis());
  Serial1.print("^Count: ");
  Serial1.print(count);
  Serial.println(count);
  count++;
  Serial1.println();
//  count++;
}
