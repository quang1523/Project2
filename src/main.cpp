#include <Arduino.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Thông tin mạng WiFi
#define WIFI_SSID "MANG DAY KTX 912"
#define WIFI_PASSWORD "83585852"

// Thông tin Firebase
#define API_KEY "AIzaSyAzMtFitvCSEdSldpqG4-mT-57JzcYT6GU"
#define DATABASE_URL "https://project2-dc08e-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Địa chỉ MAC dự kiến
String macAddressFromFirebase = "";

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseData fbdo_s1;

bool signUpOK = false;
unsigned long sendDataPrevMillis = 0;
bool ledStatus = false;
int read_data = 0;
#define LED_PIN 22 // for led pin

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wifi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected With Ip : ");
  Serial.println(WiFi.localIP());
  Serial.println();
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("SignUp Ok");
    signUpOK = true;
  }

  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (!Firebase.RTDB.beginStream(&fbdo_s1, "/LED/digital")) {
    Serial.println("FAILED: " + String(fbdo_s1.errorReason().c_str()));
  }
}

void loop() {


if (Firebase.ready() && signUpOK && (millis() -

sendDataPrevMillis > 8000 || sendDataPrevMillis == 0)) {

  sendDataPrevMillis = millis();

  if (Firebase.RTDB.getInt(&fbdo, "/test/int")) {

    if (fbdo.dataType() == "int") {

      read_data = fbdo.intData();

      Serial.print("Data received: ");

      Serial.println(read_data); //print the data received from the Firebase  database
    }

  } else {

    Serial.println(fbdo.errorReason()); //print he error (if any)

    }

  }
}