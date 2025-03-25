#include <BLEDevice.h>          // Include the BLE device library
#include <BLEServer.h>          // Include the BLE server library
#include <BLEUtils.h>           // Include the BLE utility library
#include <BLE2902.h>            // Include the BLE descriptor library
#include <FS.h>                 // Include the file system library
#include <SPIFFS.h>             // Include the SPIFFS file system library
#include <Arduino.h>            // Include the Arduino library
#include "EPD.h"                // Include the E-Paper driver library
#include "EPD_GUI.h"            // Include the E-Paper GUI library
#include "Ap_29demo.h"          // Include the custom application demo library

uint8_t ImageBW[12480];         // Define an array to store image data
#define txt_size 3536           // Define the size of text data
#define pre_size 2208           // Define the size of preset data

#define SERVICE_UUID "fb1e4001-54ae-4a28-9f74-dfccb248601d" // BLE service UUID
#define CHARACTERISTIC_UUID "fb1e4002-54ae-4a28-9f74-dfccb248601d" // BLE characteristic UUID

BLECharacteristic *pCharacteristicRX;  // BLE characteristic object
std::vector<uint8_t> dataBuffer;        // Data buffer to accumulate received data
size_t totalReceivedBytes = 0;          // Total number of bytes received
bool dataReceived = false;              // Data reception complete flag
// Object for storing uploaded files
File fsUploadFile;
unsigned char price_formerly[pre_size]; // Buffer for storing the uploaded image data
unsigned char txt_formerly[txt_size]; // Buffer for storing the uploaded image data
String filename; // Array to store the filename

// BLE characteristic callback class
class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue(); // Get the written data

      if (value.length() > 0) { // If the length of the received data is greater than 0
        Serial.printf("."); // Print a dot in the serial monitor to indicate data reception
        if (value == "OK") { // If the received data is "OK"
          dataReceived = true; // Set the data received flag to true
          return; // Exit the function
        }
        size_t len = value.length(); // Get the data length
        if (len > 0) { // If the data length is greater than 0
          // Append the received data to the buffer
          dataBuffer.insert(dataBuffer.end(), value.begin(), value.end());
          totalReceivedBytes += len; // Update the total number of bytes received
        }
      }
    }
};

void setup() {
  Serial.begin(115200); // Initialize serial communication, set baud rate to 115200

  // Start the SPIFFS file system
  if (SPIFFS.begin()) {
    Serial.println("SPIFFS Started."); // If SPIFFS starts successfully, print a message
  } else {
    // Format the SPIFFS partition
    if (SPIFFS.format()) {
      Serial.println("SPIFFS partition formatted successfully"); // If formatting is successful, print a message
      ESP.restart(); // Restart the device
    } else {
      Serial.println("SPIFFS partition format failed"); // If formatting fails, print an error message
    }
    return; // Exit the function
  }

  // Initialize the BLE device
  BLEDevice::init("ESP32_S3_BLE"); // Initialize the BLE device, set the device name to "ESP32_S3_BLE"
  BLEServer *pServer = BLEDevice::createServer(); // Create a BLE server
  BLEService *pService = pServer->createService(SERVICE_UUID); // Create a BLE service

  pCharacteristicRX = pService->createCharacteristic(
                        CHARACTERISTIC_UUID,
                        BLECharacteristic::PROPERTY_WRITE // Create a writable characteristic
                      );

  pCharacteristicRX->setCallbacks(new MyCallbacks()); // Set the characteristic's callback function
  pCharacteristicRX->addDescriptor(new BLE2902()); // Add a BLE2902 descriptor

  pService->start(); // Start the service
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising(); // Get the BLE advertising object
  pAdvertising->addServiceUUID(SERVICE_UUID); // Add the service UUID to the advertising
  pAdvertising->start(); // Start the advertising

  // Initialize the E-Paper display
  pinMode(7, OUTPUT); // Set pin 7 to output mode
  digitalWrite(7, HIGH); // Set pin 7 to high level, power supply
  EPD_GPIOInit(); // Initialize the E-Paper GPIO pins
  Paint_NewImage(ImageBW, EPD_W, EPD_H, 180, WHITE); // Create a new image canvas
  Paint_Clear(WHITE); // Clear the canvas, fill with white

  EPD_FastInit(); // Fast initialize the E-Paper display
  EPD_Display_Clear(); // Clear the E-Paper display
  EPD_Update(); // Update the E-Paper display
  UI_price(); // Update the price interface
}

void loop() {
  // If the dataReceived flag is true, process the data
  main_ui(); // Call the main UI function
}

// Process BLE received data
void ble_pic()
{
  // Check if data has been received
  if (dataReceived) {
    // If the data buffer is not empty
    if (!dataBuffer.empty()) {
      size_t bufferSize = dataBuffer.size(); // Get the size of the data buffer
      Serial.println(bufferSize); // Print the size of the data buffer to the serial monitor

      // Determine the filename based on the size of the data buffer
      if (dataBuffer.size() == txt_size) // If the data size equals txt_size
        filename = "txt.bin"; // Set the filename to txt.bin
      else
        filename = "pre.bin"; // Otherwise, set the filename to pre.bin

      // Ensure the filename starts with a slash
      if (!filename.startsWith("/")) filename = "/" + filename;

      // Open the file for writing
      fsUploadFile = SPIFFS.open(filename, FILE_WRITE);
      fsUploadFile.write(dataBuffer.data(), dataBuffer.size()); // Write the data to the file
      fsUploadFile.close(); // Close the file
      Serial.println("Save successful"); // Print a success message
      Serial.printf("Saved: "); 
      Serial.println(filename); // Print the saved filename

      // Copy the data to different arrays based on the data size
      if (bufferSize == txt_size)
      {
        for (int i = 0; i < txt_size; i++) {
          txt_formerly[i] = dataBuffer[i]; // Copy the data to the txt_formerly array
        }
        Serial.println("txt_formerly OK"); // Print a success message for processing the txt_formerly array
      } else
      {
        for (int i = 0; i < pre_size; i++) {
          price_formerly[i] = dataBuffer[i]; // Copy the data to the price_formerly array
        }
        Serial.println("price_formerly OK"); // Print a success message for processing the price_formerly array
      }

      EPD_FastInit(); // Initialize the partial display mode of the E-Paper screen
      EPD_ShowPicture(0, 0, EPD_H, 40, background_top, WHITE); // Display the background image

      // Display different images on the screen based on the data size
      if (bufferSize != txt_size)
      {
        EPD_ShowPicture(30, 180, 368, 48, price_formerly, WHITE); // Display the price image
      } else
      {
        EPD_ShowPicture(30, 60, 272, 104, txt_formerly, WHITE); // Display the text image
      }

      EPD_Display(ImageBW); // Update the screen content
      EPD_Update(); // Refresh the screen content
      EPD_DeepSleep(); // Enter deep sleep mode to save power

      // Clear the buffer after writing the data
      dataBuffer.clear();
      totalReceivedBytes = 0; // Reset the total number of bytes received
    }

    // Reset the flag after processing the data
    dataReceived = false;
  }
}

// Clear the display canvas and refresh the screen
void clear_all()
{
  Paint_NewImage(ImageBW, EPD_W, EPD_H, Rotation, WHITE); // Create a new canvas, color is white
  Paint_Clear(WHITE); // Clear the canvas
  EPD_FastInit(); // Initialize the fast mode of the E-Paper screen
  EPD_Display_Clear(); // Clear the screen content
  EPD_Update(); // Update the screen content
}

// Main UI functionality handling
void main_ui()
{
  // Process BLE received data
  ble_pic();
}

// Display the price interface
void UI_price()
{
  // Predefined: txt size is 3536, pre size is 2208
  EPD_FastInit(); // Initialize the fast mode of the E-Paper screen
  EPD_ShowPicture(0, 0, EPD_H, 40, background_top, WHITE); // Display the background image
  EPD_Display(ImageBW); // Update the screen content
  EPD_Update(); // Refresh the screen content
  EPD_DeepSleep(); // Enter deep sleep mode to save power

  // If the txt.bin file exists, read and display it
  if (SPIFFS.exists("/txt.bin")) {
    File file = SPIFFS.open("/txt.bin", FILE_READ); // Open the txt.bin file for reading
    if (!file) {
      Serial.println("Failed to open file for reading"); // If the file cannot be opened, print an error message
      return;
    }
    // Read data from the file into the array
    size_t bytesRead = file.read(txt_formerly, txt_size);
    Serial.println("File content:");
    while (file.available()) {
      Serial.write(file.read()); // Print the file content to the serial monitor
    }
    file.close(); // Close the file

    EPD_FastInit(); // Initialize the fast mode of the E-Paper screen
    EPD_ShowPicture(30, 60, 272, 104, txt_formerly, WHITE); // Display the text image on the screen
    EPD_Display(ImageBW); // Update the screen content
    EPD_Update(); // Refresh the screen content
    EPD_DeepSleep(); // Enter deep sleep mode to save power
  }

  // If the pre.bin file exists, read and display it
  if (SPIFFS.exists("/pre.bin")) {
    File file = SPIFFS.open("/pre.bin", FILE_READ); // Open the pre.bin file for reading
    if (!file) {
      Serial.println("Failed to open file for reading"); // If the file cannot be opened, print an error message
      return;
    }
    // Read data from the file into the array
    size_t bytesRead = file.read(price_formerly, pre_size);
    Serial.println("File content:");
    while (file.available()) {
      Serial.write(file.read()); // Print the file content to the serial monitor
    }
    file.close(); // Close the file

    EPD_FastInit(); // Initialize the fast mode of the E-Paper screen
    EPD_ShowPicture(30, 180, 368, 48, price_formerly, WHITE); // Display the price image on the screen
    EPD_Display(ImageBW); // Update the screen content
    EPD_Update(); // Refresh the screen content
    EPD_DeepSleep(); // Enter deep sleep mode to save power
  }
}