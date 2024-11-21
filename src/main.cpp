#include <Arduino.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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
int read_data = 0;

// Thông số oled 0.96 inch
#define SCREEN_WIDTH 128 // Chiều rộng màn hình OLED theo pixel
#define SCREEN_HEIGHT 64 // Chiều cao màn hình OLED theo pixel

// Khai báo màn hình SSD1306 kết nối với I2C (chân SDA, SCL)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//Khai báo I2c
#define SDA_PIN 8  // Chân SDA
#define SCL_PIN 9  // Chân SCL
void setup() {
  Serial.begin(115200);
  Wire.setPins(SDA_PIN, SCL_PIN);
  // Khởi tạo Oled
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
      // Clear màn hình và hiển thị thông điệp
  delay(1000);
  display.clearDisplay();
  display.setTextSize(1);      // Kích thước chữ
  display.setTextColor(SSD1306_WHITE);  // Màu chữ
  display.setCursor(0, 0);    // Vị trí bắt đầu
  display.println(F("Hello, World!"));
  display.display();          // Cập nhật hiển thị

// Khởi tạo Wifi và Firebase
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

              // Hiển thị giá trị nhận được lên màn hình OLED
        display.clearDisplay();              // Xóa nội dung màn hình
        display.setTextSize(2);              // Đặt kích thước chữ lớn hơn
        display.setTextColor(SSD1306_WHITE); // Màu chữ trắng
        display.setCursor(0, 20);            // Đặt vị trí hiển thị
        display.print("Value: ");
        display.println(read_data);          // Hiển thị giá trị
        display.display();    
    }

  } else {

    Serial.println(fbdo.errorReason()); //print he error (if any)

    }

  }
}