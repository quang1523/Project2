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
