#include <Arduino.h>            // Include the Arduino core library
#include "EPD.h"                // Include the e-paper display library
#include "EPD_GUI.h"            // Include the e-paper display GUI library
#include "Ap_29demo.h"          // Custom library file
#include "FS.h"                 // File system library
#include "SPIFFS.h"             // SPIFFS file system library, for file read/write

#include <WiFi.h>               // Include the WiFi library
#include <WebServer.h>          // Include the Web Server library
#include <Adafruit_GFX.h>       // Include the Adafruit graphics library
#include <Fonts/FreeMonoBold9pt7b.h> // Include the FreeMonoBold font

#define txt_size 3536          // Define the size of the txt file
#define pre_size 2208          // Define the size of the pre file

// Define the size of the e-paper image buffer
uint8_t ImageBW[12480]; // E-paper image buffer

// Clear the canvas and reinitialize the e-paper display
void clear_all() {
  Paint_NewImage(ImageBW, EPD_W, EPD_H, Rotation, WHITE); // Create a new image buffer
  Paint_Clear(WHITE); // Clear the canvas
  EPD_FastInit(); // Fast initialize the e-paper display
  EPD_Display_Clear(); // Clear the display content
  EPD_Update(); // Update the display
}

// Create a WebServer instance, listening on port 80
WebServer server(80);
const char* AP_SSID = "ESP32_Config"; // WiFi hotspot name

// HTML form for uploading files
String HTML_UPLOAD = "<form method=\"post\" action=\"ok\" enctype=\"multipart/form-data\">"
                     "<input type=\"file\" name=\"msg\">"
                     "<input class=\"btn\" type=\"submit\" name=\"submit\" value=\"Submit\">"
                     "</form>";

// Handling the root path request
void handle_root() {
  server.send(200, "text/html", HTML_UPLOAD); // Send the HTML content of the upload form
}

// HTML page after successful upload
String HTML_OK = "<!DOCTYPE html>"
                 "<html>"
                 "<body>"
                 "<h1>OK</h1>"
                 "</body>"
                 "</html>";

// Object for storing uploaded files
File fsUploadFile;                  // File object for saving the uploaded file
unsigned char price_formerly[pre_size]; // Buffer for storing the uploaded image data (pre_size size)
unsigned char txt_formerly[txt_size]; // Buffer for storing the uploaded image data (txt_size size)
String filename;                   // Variable for storing the filename

// Handle file upload and display on the e-paper
void okPage() {
  server.send(200, "text/html", HTML_OK); // Send the success page

  HTTPUpload& upload = server.upload(); // Get the uploaded file data

  // Note: The initial size of upload.buf is only 1436 bytes, adjust as needed
  // Click ctrl + left mouse button on upload.buf to find HTTP_UPLOAD_BUFLEN in WebServer.h
  // Adjust it to an appropriate size (e.g., 14360 bytes)
  if (upload.status == UPLOAD_FILE_END) { // Upload completed
    Serial.println("Drawing file");
    Serial.println(upload.filename); // Print the filename
    Serial.println(upload.totalSize); // Print the file size
    
    // Determine the file type based on the file size
    if (upload.totalSize == txt_size) // If the file size matches txt_size
      filename = "txt.bin";          // Set the filename to txt.bin
    else
      filename = "pre.bin";          // Otherwise, set the filename to pre.bin

    // Save the received file
    if (!filename.startsWith("/")) filename = "/" + filename; // Ensure the filename starts with '/'
    fsUploadFile = SPIFFS.open(filename, FILE_WRITE); // Open the file for writing
    fsUploadFile.write(upload.buf, upload.totalSize); // Write the file content
    fsUploadFile.close(); // Close the file
    Serial.println("Save successful");
    Serial.printf("Saved: ");
    Serial.println(filename);

    // Store the uploaded file data in the corresponding array
    if (upload.totalSize == txt_size) {
      for (int i = 0; i < txt_size; i++) {
        txt_formerly[i] = upload.buf[i]; // Store the uploaded data in the txt_formerly array
      }
      Serial.println("txt_formerly OK");
    } else {
      for (int i = 0; i < pre_size; i++) {
        price_formerly[i] = upload.buf[i]; // Store the uploaded data in the price_formerly array
      }
      Serial.println("price_formerly OK");
    }

    // Initialize the e-paper display and show the image
    EPD_FastInit(); // Initialize the partial display mode of the E-Paper screen
    EPD_ShowPicture(0, 0, EPD_H, 40, background_top, WHITE); // Show the background image

    // Display the appropriate content based on the uploaded file type
    if (upload.totalSize != txt_size) {
      EPD_ShowPicture(30, 180, 368, 48, price_formerly, WHITE); // Show the content of the pre file
    } else {
      EPD_ShowPicture(30, 60, 272, 104, txt_formerly, WHITE); // Show the content of the txt file
    }

    EPD_Display(ImageBW); // Update the screen content
    EPD_Update(); // Refresh the screen content
    EPD_DeepSleep(); // Enter deep sleep mode to save power
  }
}

void setup() {
  Serial.begin(115200);  // Start serial communication, set baud rate to 115200
  if (SPIFFS.begin()) {  // Try to start the SPIFFS file system
    Serial.println("SPIFFS Started.");  // If SPIFFS starts successfully, print the success message
  } else {
    // If SPIFFS fails to start, try to format the SPIFFS partition
    if (SPIFFS.format()) {
      // If formatting is successful, print a message and restart the device
      Serial.println("SPIFFS partition formatted successfully");
      ESP.restart();  // Restart the ESP device
    } else {
      // If formatting fails, print an error message
      Serial.println("SPIFFS partition format failed");
    }
    return;  // End the setup() function
  }

  Serial.println("Trying to connect to ");

  // Configure WiFi in AP mode and start the WiFi hotspot
  WiFi.mode(WIFI_AP);  // Set WiFi to access point mode
  boolean result = WiFi.softAP(AP_SSID, "");  // Start the WiFi hotspot, SSID is AP_SSID, password is empty
  if (result) {
    IPAddress myIP = WiFi.softAPIP();  // Get the IP address of the hotspot
    // Print hotspot related information
    Serial.println("");
    Serial.print("Soft-AP IP address = ");
    Serial.println(myIP);  // Print the IP address of the hotspot
    Serial.println(String("MAC address = ") + WiFi.softAPmacAddress().c_str());  // Print the MAC address
    Serial.println("waiting ...");
  } else {
    // If starting the hotspot fails
    Serial.println("WiFiAP Failed");
    delay(3000);  // Wait for 3 seconds
  }

  // Set HTTP server routes
  server.on("/", handle_root);  // Handler for the root path
  server.on("/ok", okPage);  // Handler for the /ok path
  server.begin();  // Start the HTTP server
  Serial.println("HTTP server started");
  delay(100);  // Delay for 100 milliseconds

  // Set the screen power pin and start the screen
  pinMode(7, OUTPUT);  // Set pin 7 to output mode
  digitalWrite(7, HIGH);  // Power the screen

  EPD_GPIOInit();  // Initialize the screen GPIO
  Paint_NewImage(ImageBW, EPD_W, EPD_H, 180, WHITE);  // Create a new image canvas
  Paint_Clear(WHITE);  // Clear the canvas

  EPD_FastInit();  // Fast initialize the screen
  EPD_Display_Clear();  // Clear the screen display
  EPD_Update();  // Update the screen display
  UI_price();  // Update the price interface
}

void loop() {
  server.handleClient();  // Handle HTTP client requests
}

// Function to update the price interface
void UI_price() {

  // txt:3536 pre:2208
  EPD_FastInit();  // Fast initialize the screen
  EPD_ShowPicture(0, 0, EPD_H, 40, background_top, WHITE);  // Show the background image
  EPD_Display(ImageBW);  // Update the screen display
  EPD_Update();  // Refresh the screen
  EPD_DeepSleep();  // Enter deep sleep mode

  // Check if the file exists and read its content
  if (SPIFFS.exists("/txt.bin")) {  // Check if /txt.bin file exists
    File file = SPIFFS.open("/txt.bin", FILE_READ);  // Open the file for reading
    if (!file) {
      Serial.println("Failed to open file for reading");  // If the file cannot be opened, print an error message
      return;
    }
    // Read data from the file into the array
    size_t bytesRead = file.read(txt_formerly, txt_size);

    Serial.println("File content:");  // Print the file content
    while (file.available()) {
      Serial.write(file.read());  // Read the file content byte by byte
    }
    file.close();  // Close the file

    EPD_FastInit();  // Fast initialize the screen
    EPD_ShowPicture(30, 60, 272, 104, txt_formerly, WHITE);  // Show the file content image
    EPD_Display(ImageBW);  // Update the screen display
    EPD_Update();  // Refresh the screen
    EPD_DeepSleep();  // Enter deep sleep mode
  }

  if (SPIFFS.exists("/pre.bin")) {  // Check if /pre.bin file exists
    File file = SPIFFS.open("/pre.bin", FILE_READ);  // Open the file for reading
    if (!file) {
      Serial.println("Failed to open file for reading");  // If the file cannot be opened, print an error message
      return;
    }
    // Read data from the file into the array
    size_t bytesRead = file.read(price_formerly, pre_size);

    Serial.println("File content:");  // Print the file content
    while (file.available()) {
      Serial.write(file.read());  // Read the file content byte by byte
    }
    file.close();  // Close the file

    EPD_FastInit();  // Fast initialize the screen
    EPD_ShowPicture(30, 180, 368, 48, price_formerly, WHITE);  // Show the price image
    EPD_Display(ImageBW);  // Update the screen display
    EPD_Update();  // Refresh the screen
    EPD_DeepSleep();  // Enter deep sleep mode
  }
}