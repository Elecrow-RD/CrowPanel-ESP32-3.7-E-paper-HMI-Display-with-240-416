#include <Arduino.h>
#include "EPD.h"
#include "EPD_GUI.h"

uint8_t ImageBW[12480];  // Buffer for storing the display image.
#define HOME_KEY 2        // Definition of the home key pin.
int HOME_NUM = 0;         // Variable for the home key press state, initially not pressed.

void setup() {
  Serial.begin(115200);   // Initialize serial communication with a baud rate of 115200.

  pinMode(7, OUTPUT);     // Set the screen power pin as output mode.
  digitalWrite(7, HIGH);  // Initialize the screen power to high to supply power.

  pinMode(41, OUTPUT);    // Set the POWER light pin as output mode.

  pinMode(HOME_KEY, INPUT);  // Set the home key pin as input mode.
}

void loop() {
  int flag = 0;  // Flag indicating whether the screen display needs to be updated.

  // Detect if the home key is pressed.
  if (digitalRead(HOME_KEY) == 0)
  {
    delay(100);  // Delay for debouncing.
    if (digitalRead(HOME_KEY) == 1)
    {
      Serial.println("HOME_KEY");  // Print the home key press information to the serial port.
      HOME_NUM =!HOME_NUM;         // Toggle the home key press state.
      
      flag = 1;  // Set the flag to 1 indicating that the screen display needs to be updated.
    }
  }

  // If the screen display needs to be updated.
  if (flag == 1)
  {
    char buffer[30];  // Buffer for storing strings.

    EPD_GPIOInit();   // Initialize the screen GPIO.
    Paint_NewImage(ImageBW, EPD_W, EPD_H, 180, WHITE);  // Create a new white background image.
    Paint_Clear(WHITE); // Clear the canvas.

    EPD_FastInit();   // Fast initialization of the screen.
    EPD_Display_Clear();  // Clear the screen display content.
    EPD_Update();     // Update the display.
    EPD_PartInit();   // Partial refresh initialization.

    // Set the POWER light state and the displayed string according to the home key state.
    if (HOME_NUM == 1)
    {
      digitalWrite(41, HIGH);  // When the home key is pressed, set the POWER light high.
      strcpy(buffer, "PWR:on");  // Set the displayed string to "PWR:on".
    }
    else
    {
      digitalWrite(41, LOW);   // When the home key is not pressed, set the POWER light low.
      strcpy(buffer, "PWR:off");  // Set the displayed string to "PWR:off".
    }

    EPD_ShowString(0, 0 + 0 * 20, buffer, 16, BLACK);  // Display the string at the specified position on the screen.
    EPD_Display(ImageBW);   // Display the updated image on the screen.
    EPD_Update();   // Update the display.
    EPD_DeepSleep();  // Put the screen to sleep to save energy.
  }
}