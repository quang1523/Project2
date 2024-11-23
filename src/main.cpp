<<<<<<< HEAD
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
float temp = 0;
float humidity = 0;
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
  if (Firebase.ready() && signUpOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    float temp = 15;
    float humidity = 20;

    if (Firebase.RTDB.setFloat(&fbdo, "Sensor/temp_data", temp)) {
      Serial.println();
      Serial.print(temp);
      Serial.println("-successfully saved to " + String(fbdo.dataPath()));
      Serial.println("(" + String(fbdo.dataType()) + ")");
    } else {
      Serial.println("FAILED : " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "Sensor/humidity_data", humidity)) {
      Serial.println();
      Serial.print(humidity);
      Serial.println("-successfully saved to " + fbdo.dataPath());
      Serial.println("(" + fbdo.dataType() + ")");
    } else {
      Serial.println("FAILED : " + fbdo.errorReason());
    }
  }

  // Stream builder for LED
  if (Firebase.ready() && signUpOK) {
    if (!Firebase.RTDB.readStream(&fbdo_s1)) {
      Serial.println("FAILED: " + fbdo_s1.errorReason());
    }

    if (fbdo_s1.streamAvailable()) {
      if (fbdo_s1.dataType() == "boolean") {
        ledStatus = fbdo_s1.boolData();
        Serial.println("Successfully read from " + fbdo_s1.dataPath() + ": (" + fbdo_s1.dataType() + ")");
        if (ledStatus == true) {
          digitalWrite(LED_PIN, HIGH);
        } else {
          digitalWrite(LED_PIN, LOW);
        }
      } else {
        Serial.println("FAILED: " + fbdo_s1.errorReason());
      }
    }
  }
}
=======
#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xf0, 0x9e, 0x9e, 0x21, 0x8c, 0x90};

// Define the pins used by the SSD1306 display (SPI)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_MOSI  23 // Data (DIN)
#define OLED_CLK   18 // Clock (SCK)
#define OLED_DC    16 // Data/Command
#define OLED_CS    5  // Chip Select
#define OLED_RESET 17 // Reset

// Initialize the display object for SPI
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_DC, OLED_RESET, OLED_CS);

typedef struct struct_message {
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;

// Create a struct_message called myData
struct_message myData;
esp_now_peer_info_t peerInfo;

volatile bool sendStatus = false;
volatile bool lastSendSuccess = false;

// Function to update display
void updateDisplay(const char* message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(message);
  display.display();
}

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  lastSendSuccess = (status == ESP_NOW_SEND_SUCCESS);
  sendStatus = true; // Trigger display update in loop
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println("OLED initialization failed");
    while (true); // Stop execution if OLED fails to initialize
  }
  updateDisplay("Initializing...");

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    updateDisplay("ESP-NOW Init Failed");
    return;
  }

  // Register Send Callback
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    updateDisplay("Add Peer Failed");
    return;
  }

  updateDisplay("Setup Complete");
}

void loop() {
  // Set values to send
  strcpy(myData.a, "THIS IS A CHAR");
  myData.b = random(1, 20);
  myData.c = 1.2;
  myData.d = false;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending the data");
    updateDisplay("Send Error");
  }

  // Update display based on send status
  if (sendStatus) {
    sendStatus = false;
    if (lastSendSuccess) {
      updateDisplay("Send Success");
    } else {
      updateDisplay("Send Fail");
    }
  }

  delay(2000);
}
>>>>>>> 0fa84ee (no complain)
