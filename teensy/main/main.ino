#include <Wire.h>
#include <SPI.h>

//SD Card Library
#include <SD.h>

//#include "altimeter.h"
//#include "thermometer.h"
#include "gps.h"
#include "gyroscope.h"
#include "accelerometer.h"
//#include "thermometer.h"

File myFile;
const int chipSelect = BUILTIN_SDCARD;

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

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }

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
  unsigned int failSafeTime = 500;
  for (int i = 0; i < num_sensors; i++) {
    elapsedMillis waiting;
    // Time out failsafe
    while (waiting < failSafeTime) {
      mySensors[i]->read_sensor_value();
//    Serial1.println(millis()-time);
//    time = millis();
    }
    waiting = 0;
  }
}

int count = 0;
void print_all_sensors() {
  //prints a single string containing all sensor values
  //to be decoded by python script
  String output = "";
  //expect precision issues to # of digits
  //python script currently expects 2 digits per datum

  //GPS
  Serial1.print("^latitude:");
  Serial1.println(myGps->get_latitude());
  output.concat("^latitude:");
  output.concat(myGps->get_latitude());
//  Serial1.print(";");

  Serial1.print("^longitude:");
  Serial1.println(myGps->get_longitude());
  output.concat("^longitude:");
  output.concat(myGps->get_longitude());
//  Serial1.print(";");

  //Gyroscope
  Cartesian_Coordinates GyCoordinates = myGyroscope->get_sensor_coordinates();
  Serial1.print("^GyX:");
  Serial1.println(GyCoordinates.x);
  output.concat("^GyX:");
  output.concat(GyCoordinates.x);
  
//  Serial1.print(";");
  Serial1.print("^GyY:");
  Serial1.println(GyCoordinates.y);
  output.concat("^GyY:");
  output.concat(GyCoordinates.y);
  
//  Serial1.print(";");
  Serial1.print("^GyZ:");
  Serial1.println(GyCoordinates.z);
  output.concat("^GyZ:");
  output.concat(GyCoordinates.z);
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
  output.concat("^speed:");
  output.concat(myGps->get_speed());
//  Serial1.print(";");

  Cartesian_Coordinates AcCoordinates = myAccelerometer->get_sensor_coordinates();
  
  Serial1.print("^acceleration x:");
  Serial1.println(AcCoordinates.x);
  output.concat("acceleration x:");
  output.concat(AcCoordinates.x);
//  Serial1.print(";");

  Serial1.print("^acceleration y:");
  Serial1.println(AcCoordinates.y);
  output.concat("acceleration y:");
  output.concat(AcCoordinates.y);
//  Serial1.print(";");

  Serial1.print("^acceleration z:");
  Serial1.println(AcCoordinates.z);
  output.concat("acceleration z:");
  output.concat(AcCoordinates.z);
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
  
  output.concat("^Count: ");
  output.concat(count);

  myFile = SD.open("SMV.txt", FILE_WRITE);

  if (myFile) {
      Serial.print(output);
      myFile.println(output);
    // close the file:
      myFile.close();
      //Serial.println("done.");
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }
  
//  count++;
}
