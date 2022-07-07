#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "wifiCredentials.h" //wifi credentials not included on github

#define DATABASE_URL "smv-daq.firebaseio.com"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;

const long utcOffsetInSeconds = 0;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds); //offset set later on in code

long startTime = 0; //intiialize start time variable 
String latestTrial = "";

void readSerial() {
  bool strEnd = false;
  String serialString;
  while (Serial.available() > 0) { //read in serial
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
    String allowed[] = {"latitude", "longitude", "GyX", "GyY", "GyZ", "speed", "acceleration x", "acceleration y", "acceleration z", "Count"}; 
    bool good = false;
    for (int i = 0; i < 10; i++) { //double check that string is good and isn't scrambled
      if (prefix == allowed[i]) {
        good = true;
        break;
      }
    }
    if (good) { //save into json object to be pushed
      json.set(serialString.substring(0, serialString.lastIndexOf(':')), serialString.substring(serialString.lastIndexOf(':') + 1).toFloat());
    }
    serialString = "";
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.begin(SSID, PW); // connect to wifi
  Serial.println("Connecting...");
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(10);
  }
  Serial.println("Connected!");
  Serial.println(WiFi.localIP());

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

  Firebase.reconnectWiFi(true);
  Firebase.begin(&config, &auth);
  
  Firebase.setString(fbdo, "Latest Trial", latestTrial); // trial is named based off of time and date now
  Serial.println("New Latest Trial: " + latestTrial);
  if (Firebase.getString(fbdo, "Latest Trial"))
    Serial.println(fbdo.to<String>());
}

void loop() {
  readSerial();
  json.toString(Serial, true); // print to serial, can remove later on for performance. think data should be bottleneck
  Serial.println();
  String currentTime = String(millis()-startTime); //time since start
  Firebase.setString(fbdo, "Latest Time", currentTime);
  if (!Firebase.setJSON(fbdo, latestTrial+ "/" + currentTime, json)) // send to firebase, if fail print reason
    Serial.println(fbdo.errorReason());
}
