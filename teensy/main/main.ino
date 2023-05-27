
#include <Wire.h> //I2C library
#include <SD.h> //SD card library
#include "SMVcanbus.h" //CAN bus class
#include "gps.h" //GPS class and functions
#include "ids.h"
#include <TimeLib.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
#include <WString.h>
#include <MAX6956.h>

File myFile;
const int chipSelect = BUILTIN_SDCARD;

GPS gps(1);
CANBUS can(DAQ);

//Bear 1
double bear1_vals[] = {0, 0, 0, 0, 0, 0, 0};
int bear1_count = 7;

//Bear 2
double bear2_vals[] = {0, 0, 0, 0, 0, 0, 0};
int bear2_count = 7;

//Power Control
double powerControl_vals[] = {0, 0, 0, 0, 0, 0, 0, 0};
int powerControl_count = 8;

//Steering
double steering_vals[] = {0, 0};
int steering_count = 2;

bool SDgood = true;
char* fileName = "2023-04-02-T-18-22-30.txt";


#define MAX6956_1 0x40 //AD0 GND, AD1 GND
#define MAX6956_2 0x44 //AD0 GND, AD1 VCC
#define BRIGHTNESS 0xFF // Max brightness
MAX6956 max6956(MAX6956_1, MAX6956_2, BRIGHTNESS);

void display() {
  // Serial.print("RPM: ");
  // Serial.println(bear1_vals[RPM]);
  max6956.outputDigit(gps.get_speed(), 1);
  // delay(50);
}

void setup() {
  Serial.begin(115200);  // Start serial for output
  Serial2.begin(115200);  // Start serial for TX to ESP8266
  Wire.begin();
  // Wire.setClock(100000);
  // while (!Serial) ;
  if (!SD.begin(chipSelect)) {
    Serial.println("SD Card Initialization Failed!");
    SDgood = false;
    // return;
  }
  getFileName();
  Serial.println("setup done");
}

long currentTime = 0;

void loop() {  
  listenToCAN();
  gps.readValues();
  display();
  if (millis() - currentTime > 100) {
    printValues();
    currentTime = millis();
  }
}

void getFileName() {
  setSyncProvider(getTeensy3Time);   // the function to get the time from the RTC
  if(timeStatus()!= timeSet) {
    Serial.println("Unable to sync with the RTC");
  }
  else {
    Serial.println("RTC has set the system time");
  }
  String fileNameStr = "";
  fileNameStr.concat(year());
  fileNameStr.concat("-");
  if (month() < 10) {
    fileNameStr.concat("0");
  }
  fileNameStr.concat(month());
  fileNameStr.concat("-");
  if (day() < 10) {
    fileNameStr.concat("0");
  }
  fileNameStr.concat(day());
  fileNameStr.concat("-");
  fileNameStr.concat("T");
  fileNameStr.concat("-");
  if (hour() < 10) {
    fileNameStr.concat("0");
  }
  fileNameStr.concat(hour());
  fileNameStr.concat("-");
  if (minute() < 10) {
    fileNameStr.concat("0");
  }
  fileNameStr.concat(minute());
  fileNameStr.concat("-");
  if (second() < 10) {
    fileNameStr.concat("0");
  }
  fileNameStr.concat(second());
  fileNameStr.concat(".txt");
  fileNameStr.toCharArray(fileName, 26);
  Serial.println(fileName);
}

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

void listenToCAN() {
  can.looper();
  if (can.isThere()) {
    String canDevice = can.getHardware();
    String canDataType = can.getDataType();
    double canData = can.getData();
    // Serial.print(canDevice);
    // Serial.print(" ");
    // Serial.print(canDataType);
    // Serial.print(" ");
    // Serial.print(canData);
    // Serial.println(";");
    if (canDevice == "Bear 1") {
      for (int i = 0; i < bear1_count; i++) {
        if (canDataType == motorMessage[i]) {
          bear1_vals[i] = canData;
        } 
      }
    }
    else if (canDevice == "Bear 2") {
      for (int i = 0; i < bear2_count; i++) {
        if (canDataType == motorMessage[i]) {
          bear2_vals[i] = canData;
        } 
      }
    } 
    else if (canDevice == "Power Control") {
      for (int i = 0; i < powerControl_count; i++) {
        if (canDataType == powerMessage[i]) {
          powerControl_vals[i] = canData;
        } 
      }
    }
    else if (canDevice == "Steering Wheel") {
      for (int i = 0; i < steering_count; i++) {
        if (canDataType == steeringMessage[i]) {
          steering_vals[i] = canData;
        }
      }
    }
    else {
      Serial.print("Unknown device...");
    }
  }
}

void printValues() {
  String toSD;
  toSD.concat(millis());
  toSD.concat(": ");
  // GPS
  double gps_latitude = gps.get_latitude();
  Serial2.print("^GPS latitude:");
  Serial2.println(gps_latitude, 7);
  toSD += String(gps_latitude,7);
  toSD.concat(";");

  double gps_longitude = gps.get_longitude();
  Serial2.print("^GPS longitude:");
  Serial2.println(gps_longitude, 7);
  toSD += String(gps_longitude, 7);
  toSD.concat(";");

  double gps_speed = gps.get_speed();
  Serial2.print("^GPS speed:");
  Serial2.println(gps_speed);
  toSD.concat(gps_speed);
  toSD.concat(";");

  for (int i = 0; i < bear1_count; i++) {
    Serial2.print("^Bear 1 ");
    Serial2.print(motorMessage[i]);
    Serial2.print(":");
    Serial2.println(bear1_vals[i]);

    toSD.concat(bear1_vals[i]);
    toSD.concat(";");
  }

  for (int i = 0; i < bear2_count; i++) {
    Serial2.print("^Bear 2 ");
    Serial2.print(motorMessage[i]);
    Serial2.print(":");
    Serial2.println(bear2_vals[i]);

    toSD.concat(bear2_vals[i]);
    toSD.concat(";");
  }

  for (int i = 0; i < powerControl_count; i++) {
    Serial2.print("^Power Control ");
    Serial2.print(powerMessage[i]);
    Serial2.print(":");
    Serial2.println(powerControl_vals[i]);

    toSD.concat(powerControl_vals[i]);
    toSD.concat(";");
  }

  for (int i = 0; i < steering_count; i++) {
    Serial2.print("^Steering ");
    Serial2.print(steeringMessage[i]);
    Serial2.print(":");
    Serial2.println(steering_vals[i]);

    toSD.concat(steering_vals[i]);
    toSD.concat(";");
  }
  // Serial.print("SD: ");
  // Serial.println(toSD);
  //SD CARD
  if (SDgood) {
    myFile = SD.open(fileName, FILE_WRITE);
    // Serial.print(myFile);
    // Serial.println(myFile);
    if (myFile) {
      Serial.print("SD: ");
      Serial.println(toSD);
      myFile.println(toSD);
    // close the file:
      myFile.close();
    } 
    else {
        // if the file didn't open, print an error:
        Serial.println("error opening file");
    }
  }
}
