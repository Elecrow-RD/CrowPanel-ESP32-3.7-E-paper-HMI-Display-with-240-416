#include <Arduino.h>
#include "EPD.h"
#include "EPD_GUI.h"
#include "BLEDevice.h"    // BLE driver library
#include "BLEServer.h"    // BLE Bluetooth server library
#include "BLEUtils.h"     // BLE utility library
#include "BLE2902.h"      // Characteristic descriptor library

BLECharacteristic *pCharacteristic;
BLEServer *pServer;
BLEService *pService;
bool deviceConnected = false;
char BLEbuf[32] = {0};

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// Callback class: Handle BLE server connection events
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("------> BLE connected.");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("------> BLE disconnected.");
      pServer->startAdvertising(); // Restart advertising
      Serial.println("Start advertising");
    }
};

// Callback class: Handle BLE characteristic write events
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.print("------> Received Value: ");

        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        Serial.println();

        // Process the received value
        if (rxValue.find("A") != -1) {
          Serial.println("Rx A!");
        }
        else if (rxValue.find("B") != -1) {
          Serial.println("Rx B!");
        }
        Serial.println();
      }
    }
};

// EPD initialization parameters
uint8_t ImageBW[12480]; // Image data for EPD display

void setup() {
  pinMode(7, OUTPUT);    // Configure the screen power pin as output
  digitalWrite(7, HIGH); // Initialize the screen power to high level (off state)

  // Initialize the BLE device
  BLEDevice::init("CrowPanel3-7");

  // Create the BLE server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE service
  pService = pServer->createService(SERVICE_UUID);

  // Create the BLE characteristic: TX (for notifications)
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());

  // Create the BLE characteristic: RX (for writes)
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the BLE service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
}

int flag = 0;

void loop() {
  // Main loop
  if (deviceConnected) {
    // If the device is connected, send TX value every second
    memset(BLEbuf, 0, 32);
    memcpy(BLEbuf, (char*)"Hello BLE APP!", 32);
    pCharacteristic->setValue(BLEbuf);
    pCharacteristic->notify(); // Send notification to the application
    Serial.print("*** Sent Value: ");
    Serial.print(BLEbuf);
    Serial.println(" ***");
    
    // Switch to displaying BLE connection status
    if (flag != 2)
      flag = 1;
  }
  else {
    // If the device is not connected
    if (flag != 4)
      flag = 3;
  }

  if (flag == 1) {
    // Handling when BLE is connected
    char buffer[30];
    EPD_GPIOInit();              // Initialize EPD-related GPIO
    Paint_NewImage(ImageBW, EPD_W, EPD_H, 180, WHITE); // Create a new EPD image
    Paint_Clear(WHITE);          // Clear the canvas

    EPD_FastInit();              // Fast initialize EPD
    EPD_Display_Clear();         // Clear the EPD display
    EPD_Update();                // Update the EPD display
    EPD_PartInit();              // Initialize partial display functionality

    strcpy(buffer, "Bluetooth connected");  // Copy the connection status message
    strcpy(BLEbuf, "Sent Value:");
    strcat(BLEbuf, "Hello BLE APP!");
    
    // Display connection status information and the sent BLE value on the EPD
    EPD_ShowString(0, 0 + 0 * 20, buffer, 16, BLACK);
    EPD_ShowString(0, 0 + 1 * 20, BLEbuf, 16, BLACK);
    EPD_Display(ImageBW);        // Display the EPD image
    EPD_Update();                // Update the EPD display
    EPD_DeepSleep();             // Enter EPD deep sleep mode

    flag = 2;                    // Switch the flag to the display completed state
  }
  else if (flag == 3) {
    // Handling when BLE is not connected
    char buffer[30];
    EPD_GPIOInit();              // Initialize EPD-related GPIO
    Paint_NewImage(ImageBW, EPD_W, EPD_H, 180, WHITE); // Create a new EPD image
    Paint_Clear(WHITE);          // Clear the canvas

    EPD_FastInit();              // Fast initialize EPD
    EPD_Display_Clear();         // Clear the EPD display
    EPD_Update();                // Update the EPD display

    // Display the not connected status information on the EPD (optional part is commented out)
    // strcpy(buffer, "Bluetooth not connected!");
    // EPD_ShowString(0, 0 + 0 * 20, buffer, 16, BLACK);
    EPD_Display(ImageBW);        // Display the EPD image
    EPD_Update();                // Update the EPD display
    // EPD_DeepSleep();          // Optionally enter EPD deep sleep mode

    flag = 4;                    // Switch the flag to the not connected state display completed
  }

  delay(1000); // Delay for 1 second
}