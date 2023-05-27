#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "credentials.h" //wifi credentials not included on github
#include <NTRIPClient.h>
#include "ids.h"

#define DATABASE_URL "smv-daq.firebaseio.com"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;

// NTP Client
const long utcOffsetInSeconds = 0;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds); //offset set later on in code

long startTime = 0; //intiialize start time variable 
String latestTrial = "";

// RTK Correction
NTRIPClient ntrip_c;

void readSerial() {
  // Serial.println("---");
  // Serial.print("Serial Begin");
  // Serial.println(millis());
  bool strEnd = false;
  while (Serial.available() > 0) { //read in serial
    String serialString = "";
    char head = Serial.read();
    if (head == '^') { //if reading in beginning of data line
      while (Serial.available() > 0) { //read until newline character
        char incomingByte = Serial.read();
        serialString.concat(incomingByte);
        if (incomingByte == '\n')
          break;
      }
    }
    String prefix = serialString.substring(0, serialString.lastIndexOf(':'));
    double data = serialString.substring(serialString.lastIndexOf(':') + 1).toDouble();
    if (serialString.length() > 0) {
      bool dataGood = false;
      if (json.isMember(prefix)) {
        json.set(prefix, data);
        // Serial.println(prefix);
      }
      else {
        Serial.print("not: ");
        Serial.println(prefix);
      }
    }
  }
  // Serial.print("Serial End");
  // Serial.println(millis());
}

void NTRIPsetup() {
  Serial.println("Requesting SourceTable.");
  if(ntrip_c.reqSrcTbl(host,httpPort)){
    char buffer[512];
    delay(5);
    while(ntrip_c.available()){
      ntrip_c.readLine(buffer,sizeof(buffer));
      Serial.print(buffer); 
    }
  }
  else{
    Serial.println("SourceTable request error");
  }
  Serial.print("Requesting SourceTable is OK\n");
  ntrip_c.stop(); //Need to call "stop" function for next request.
  
  Serial.println("Requesting MountPoint's Raw data");
  if(!ntrip_c.reqRaw(host,httpPort,mntpnt,user,passwd)){
    delay(15000);
    ESP.restart();
  }
  Serial.println("Requesting MountPoint is OK");
}

void setup() {
  // put your setup code here, to run once:
  Serial.setRxBufferSize(1024);
  Serial.begin(115200);
  Serial1.begin(9600);
  WiFi.begin(SSID, PW); // connect to wifi
  Serial.println("Connecting...");
  int LED1 = 16;
  pinMode(LED1, OUTPUT);
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    digitalWrite(LED1, HIGH);
    delay(50);
    digitalWrite(LED1, LOW);
    delay(50);
    // Serial.println("Connecting...");
  }
  // digitalWrite(LED1, HIGH);
  Serial.println("Connected!");
  Serial.println(WiFi.localIP());

  NTRIPsetup();

  timeClient.begin();
  timeClient.setTimeOffset(-7*60*60); //set timezone; currently using PST
  timeClient.update();

  time_t epochTime = timeClient.getEpochTime(); // date format junk
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  int currentYear = ptm->tm_year+1900;
  String currentTime = timeClient.getFormattedTime();
  latestTrial = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay) +  " " + currentTime;
  startTime = millis();
  
  config.database_url = DATABASE_URL; // using firebase "test mode"; temporary and should add auth 
  config.signer.test_mode = true;

  fbdo.setBSSLBufferSize(16384, 16384);

  Firebase.reconnectWiFi(true);
  Firebase.begin(&config, &auth);
  
  Firebase.setString(fbdo, "Latest Trial", latestTrial); // trial is named based off of time and date now
  Serial.println("New Latest Trial: " + latestTrial);
  if (Firebase.getString(fbdo, "Latest Trial")) {
    Serial.println(fbdo.to<String>());
  }
  Serial.println("Setup done");
  
  json.set("GPS latitude",0);
  json.set("GPS longitude",0);
  json.set("GPS speed",0);

  for (int i = 0; i < 7; i++) {
    json.set("Bear 1 " + motorMessage[i],0);
    json.set("Bear 2 " + motorMessage[i],0);
  }
  for (int i = 0; i < 8; i++) {
    json.set("Power Control " + powerMessage[i],0);
  }
  for (int i = 0; i < 2; i++) {
    json.set("Steering " + steeringMessage[i],0);
  }
  String jsonString;
  json.toString(jsonString);
  Serial.println(jsonString);
}

unsigned long lastTime = 0;

void loop() {
  readSerial();
  // json.toString(Serial, true); // print to serial, can remove later on for performance. think data should be bottleneck
  // Serial.println("---");
  // Serial.print("Before: ");
  // Serial.println(millis());
  String currentTime = String(millis()-startTime); //time since start
  String jsonString;
  json.toString(jsonString);
  // Serial.println(jsonString);
  Firebase.setStringAsync(fbdo, "Latest Time", currentTime);
  // Serial.print("Middle: ");
  // Serial.println(millis());
  yield();
  if (!Firebase.setJSONAsync(fbdo, latestTrial + "/" + currentTime, json)) { // send to firebase, if fail print reason
    Serial.print("Error: ");
    Serial.print(fbdo.errorReason());
    Serial.println(currentTime);
  }
  // Serial.print("After: ");
  // Serial.println(millis());
  lastTime = millis(); 

  // NTRIP correction data
  while(ntrip_c.available()) {
      char ch = ntrip_c.read();        
      Serial1.print(ch);
      // Serial.print(ch);
  }
}
