#include <Arduino.h>
#include "EPD.h"
#include "EPD_GUI.h"

uint8_t ImageBW[12480]; // Store image data for EPD display

// Function: Determine the state of each GPIO pin and display it on the EPD
void judgement_function(int* pin)
{
  char buffer[30]; // Buffer to store formatted strings
  EPD_GPIOInit(); // Initialize GPIO pins for EPD
  Paint_NewImage(ImageBW, EPD_W, EPD_H, 180, WHITE); // Create a new EPD image, width EPD_W, height EPD_H, background color white
  Paint_Clear(WHITE); // Clear the canvas, fill with white background

  EPD_FastInit(); // Fast initialize EPD
  EPD_Display_Clear(); // Clear the EPD display
  EPD_Update(); // Update the EPD display
  EPD_PartInit(); // Initialize partial display functionality of EPD

  // Iterate through each GPIO pin
  for (int i = 0; i < 12; i++)
  {
    int state = digitalRead(pin[i]); // Read the state of the GPIO pin

    // Format the string to display based on the GPIO pin state
    if (state == HIGH) {
      int length = sprintf(buffer, "GPIO%d : on", pin[i]); // Format string indicating the GPIO pin is on
      buffer[length] = '\0'; // Add null terminator to the string
      EPD_ShowString(0, 0 + i * 20, buffer, 16, BLACK); // Display the formatted string on the EPD
    } else {
      int length = sprintf(buffer, "GPIO%d : off", pin[i]); // Format string indicating the GPIO pin is off
      buffer[length] = '\0'; // Add null terminator to the string
      EPD_ShowString(0, 0 + i * 20, buffer, 16, BLACK); // Display the formatted string on the EPD
    }
  }

  EPD_Display(ImageBW); // Display the EPD image
  EPD_Update(); // Update the EPD display
  EPD_DeepSleep(); // Enter deep sleep mode for EPD to save power
}

// Array of GPIO pins to monitor their states
int pin_Num[12] = {8, 3, 14, 9, 16, 15, 18, 17, 20, 19, 38, 21};

void setup() {
  Serial.begin(115200); // Initialize serial communication at 115200 baud rate

  // Set the screen power pin as output and set its level to high
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);

  // Set each GPIO pin as output and set their levels to high
  for (int i = 0; i < 12; i++) {
    pinMode(pin_Num[i], OUTPUT);
    digitalWrite(pin_Num[i], HIGH);
  }

  judgement_function(pin_Num); // Call the function to start monitoring and displaying GPIO pin states
}

void loop() {
  // No other actions in the main loop, just delay for 1 second
  delay(1000);
}