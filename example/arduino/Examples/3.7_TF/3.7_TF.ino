#include <Arduino.h>
#include "EPD.h"
#include "EPD_GUI.h"

#include "SD.h"

#define SD_MOSI 40   // SD card MOSI pin
#define SD_MISO 13   // SD card MISO pin
#define SD_SCK 39    // SD card SCK pin
#define SD_CS 10     // SD card CS pin
SPIClass SD_SPI = SPIClass(HSPI);  // Use HSPI as the SD card SPI interface
uint8_t ImageBW[12480];  // Buffer for displaying images.

void setup() {
  Serial.begin(115200);  // Initialize serial communication with a baud rate of 115200.

  // Initialize the screen power pin.
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);  // Set the screen power pin to high to supply power.

  // Initialize the screen backlight pin.
  pinMode(42, OUTPUT);
  digitalWrite(42, HIGH);  // Set the screen backlight pin to high to turn on the backlight.
  delay(10);  // Wait for circuit stabilization.

  // Initialize the SD card.
  SD_SPI.begin(SD_SCK, SD_MISO, SD_MOSI);  // Start the SD card's SPI interface.
  if (!SD.begin(SD_CS, SD_SPI, 80000000))  // Try to mount the file system and set the SPI clock rate to 80MHz.
  {
    Serial.println(F("ERROR: File system mount failed!"));  // Prompt for mount failure.
  }
  else
  {
    Serial.printf("SD Size: %lluMB \n", SD.cardSize() / (1024 * 1024));  // Print SD card capacity information.
    char buffer[30];  // String buffer, assuming no more than 30 characters.
    int length = sprintf(buffer, "SD Size:%lluMB", SD.cardSize() / (1024 * 1024));  // Format SD card capacity information.
    buffer[length] = '\0';  // Manually add the end-of-string symbol.

    // Initialize display settings.
    EPD_GPIOInit();  // Initialize the screen GPIO.
    Paint_NewImage(ImageBW, EPD_W, EPD_H, 180, WHITE);  // Create a new white background image.
    Paint_Clear(WHITE);  // Clear the canvas.

    EPD_FastInit();  // Fast initialization of the screen.
    EPD_Display_Clear();  // Clear the screen display content.
    EPD_Update();  // Update the display.
    Serial.println(buffer);  // Print SD card capacity information to the serial port.

    EPD_PartInit();  // Partial refresh initialization.
    EPD_ShowString(0, 0, buffer, 16, BLACK);  // Display SD card capacity information on the screen.

    EPD_Display(ImageBW);  // Update the display.
    EPD_Update();  // Update the display.
    EPD_DeepSleep();  // Put the screen to sleep to save energy.
  }
}

void loop() {
  delay(10);  // Delay in the main loop.
}