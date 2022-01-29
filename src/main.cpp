#include <Arduino.h>
#include <Wifi.h>
#include <Firebase_ESP_Client.h>
#include <Time.h>
#include <sensitve_data.h> // stores Firebase data, passwords, etc


// Serial output pins
#define RXD 26
#define TXD 27

// Firebase objects
FirebaseData data;
FirebaseAuth auth;
FirebaseConfig config;

// Firebase config
String temperaturePath = "/temperature";
String timePath = "/time";
String mainPath = "/Data";

String parentPath;
FirebaseJson json;


// Time stuff
const char* ntp = "pool.ntp.org";
const long pst_offset_sec = -28000; // Vancouver is in UTC-8 (-8 * 60 * 60)
const long dayLightSavings_offset = 3600; // really?
const char * time_now;



// Code based on https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino/#3
void setupWiFi() {
  // set the wifi to station mode
  WiFi.mode(WIFI_STA);
  // begin wifi connection and log in
  WiFi.begin(ssid, password);
  Serial.printf("Connecting to %s", ssid);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  // print out the local ip after connecting
  Serial.print(WiFi.localIP());
  Serial.println("end of setupwifi");
}


void setupFirebase() {
  // deal with auth
  config.api_key = API_KEY;
  auth.user.email = EMAIL;
  auth.user.password = PASS;

  // set the url
  config.database_url = URL;
  Firebase.reconnectWiFi(true);
  data.setResponseSize(4096);
  
  // begin the database connection
  Firebase.begin(&config, &auth);
  Serial.println("end of setupFirebase");
}

// 
void setupTime() {
  configTime(pst_offset_sec, dayLightSavings_offset, ntp);
  Serial.println("end of setupTime");
}


// returns the current time from ntp server
char * currentTime() {
  time_t now = time(0);
  struct tm * time_info;
  time( &now );
  time_info = localtime(&now);

  char * time_curr = asctime(time_info);

  return time_curr;
}

void setup() {
  // ----- Setup Serial output -----
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD, TXD);
  // ----- Other setup functions
  setupWiFi();
  setupFirebase();
  setupTime();
}

void loop() {
  while (Serial2.available()) {

    if (Firebase.ready()) {
      Serial.println("fb ready");

      time_now = currentTime();
      Serial.println("time ready");
  
      // for debugging:
      Serial.println("time :");
      Serial.printf("%s", time_now);
  
      //boolean inputReady = true;
      // get temp from serial
      //if (inputReady) {
      String temp_reading =  Serial2.readStringUntil(13);
      Serial.println("temp read");
      Serial.println("temp :");
      Serial.println(temp_reading);
      json.set(temperaturePath.c_str(), temp_reading);
      //inputReady = false;
      //}
      json.set(timePath.c_str(), String(time_now));
      Serial.printf("Setting database ... %s\n", Firebase.RTDB.setJSON(&data, mainPath.c_str(), &json) ? "ok" : data.errorReason().c_str());
    }
  }
}


// API WEB KEY: AIzaSyCoOd_Wb7hUUj0KYTHCmsgpfc-q8vCl0SY
// url: https://sensorlogging-7a5ce-default-rtdb.firebaseio.com/
// UID: QbVeBlExUyRzncC7cLpRn7vKBNh1