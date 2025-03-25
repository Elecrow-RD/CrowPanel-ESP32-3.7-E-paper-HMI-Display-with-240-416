#include <Arduino.h>
#include "EPD.h"
#include "EPD_GUI.h"
#include <WiFi.h>

// Define the SSID and password required for WiFi connection.
String ssid = "yanfa_software";
String password = "yanfa-123456";

// Define a buffer for displaying images.
uint8_t ImageBW[12480];

void setup() {
  Serial.begin(115200);  // Initialize serial communication with a baud rate of 115200.

  pinMode(7, OUTPUT);  // Initialize the screen power pin.
  digitalWrite(7, HIGH);  // Set the screen power pin to high to supply power.

  // Connect to WiFi.
  WiFi.begin(ssid, password);  // Start connecting to the WiFi network with the specified SSID.

  // Wait until the WiFi connection is successful.
  while (WiFi.status()!= WL_CONNECTED) {
    delay(500);
    Serial.print(".");  // Print dots for connection status.
  }

  Serial.println("");  // Print an empty line.
  Serial.println("WiFi connected");  // Print WiFi connection success message.
  Serial.println("IP address: ");  // Print IP address information.
  Serial.println(WiFi.localIP());  // Print the locally assigned IP address.

  char buffer[40];  // Define a character array as a buffer for displaying information.

  // Initialize the screen GPIO.
  EPD_GPIOInit();

  // Create a new image with a white background.
  Paint_NewImage(ImageBW, EPD_W, EPD_H, 180, WHITE);

  // Clear the canvas by filling it with white.
  Paint_Clear(WHITE);

  // Fast initialization of the screen.
  EPD_FastInit();

  // Clear the display content on the screen.
  EPD_Display_Clear();

  // Update the display.
  EPD_Update();

  // Partial refresh initialization.
  EPD_PartInit();

  // Copy the string "WiFi connected" to the buffer.
  strcpy(buffer, "WiFi connected");

  // Display the "WiFi connected" string on the screen.
  EPD_ShowString(0, 0 + 0 * 20, buffer, 16, BLACK);

  // Copy the string "IP address: " to the buffer.
  strcpy(buffer, "IP address: ");

  // Convert the WiFi IP address to a string and append it to the buffer.
  strcat(buffer, WiFi.localIP().toString().c_str());

  // Display the IP address information on the screen.
  EPD_ShowString(0, 0 + 1 * 20, buffer, 16, BLACK);

  // Update the display.
  EPD_Display(ImageBW);

  // Update the display.
  EPD_Update();

  // Put the screen into deep sleep mode to save energy.
  EPD_DeepSleep();
}

void loop() {
  // The main loop is empty, and the program loops infinitely here.
}