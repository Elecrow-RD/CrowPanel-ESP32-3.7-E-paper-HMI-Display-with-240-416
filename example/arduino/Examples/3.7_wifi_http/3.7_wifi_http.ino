#include <WiFi.h>
#include <Arduino.h>  // Include Arduino library to provide basic functions.
#include "EPD.h"               // Include EPD library for controlling electronic paper displays.
#include "EPD_GUI.h"           // Include EPD_GUI library for graphical user interface operations.

#include <ArduinoJson.h>       // Include ArduinoJson library for parsing JSON data.
#include <HTTPClient.h>        // Include HTTPClient library for sending HTTP requests.
#include "pic.h"                // Include custom image file.

const char * ID = "yanfa_software";     // ID of the WiFi network.
const char * PASSWORD = "yanfa-123456"; // Password of the WiFi network.

// Weather related parameters.
String API = "SBy5V5m7FkAn1e_vi";  // Key for the weather API.
String WeatherURL = "";             // Request URL for the weather API.
String CITY = "Shenzhen";               // City to query for weather.
String url_xinzhi = "";             // Specific weather URL.
String Weather = "0";               // Stores the retrieved weather information.

long sum = 0;                        // A variable for calculation (currently unused).

// Define a black and white image array as a buffer for e-paper display.
uint8_t ImageBW[12480];

/* Create an HTTPClient instance. */
HTTPClient http;

// Build the weather query URL.
String GitURL(String api, String city)
{
  url_xinzhi =  "https://api.seniverse.com/v3/weather/now.json?key=";
  url_xinzhi += api;
  url_xinzhi += "&location=";
  url_xinzhi += city;
  url_xinzhi += "&language=zh-Hans&unit=c"; // Set language to Simplified Chinese and unit to Celsius.
  return url_xinzhi;
}

// Parse weather information.
void ParseWeather(String url)
{
  DynamicJsonDocument doc(1024); // Allocate memory, dynamically allocate 1024 bytes.
  http.begin(url);               // Start HTTP request.

  int httpGet = http.GET();      // Send GET request and get HTTP status code.
  if (httpGet > 0)
  {
    Serial.printf("HTTPGET is %d", httpGet); // Output HTTP status code.

    if (httpGet == HTTP_CODE_OK)
    {
      String json = http.getString(); // Get response content.
      Serial.println(json);

      deserializeJson(doc, json); // Parse JSON data.

      Weather = doc["results"][0]["now"]["text"].as<String>(); // Get weather information.
    }
    else
    {
      Serial.printf("ERROR1!!"); // HTTP request failed.
    }
  }
  else
  {
    Serial.printf("ERROR2!!"); // HTTP request error.
  }
  http.end(); // End HTTP request.
}

// Create a character array for displaying information.
char buffer[40];

// Clear the display.
void clear_all() {
  Paint_NewImage(ImageBW, EPD_W, EPD_H, 180, WHITE); // Create new image cache.
  Paint_Clear(WHITE); // Clear the canvas.
  EPD_FastInit(); // Fast initialization of e-paper display.
  EPD_Display_Clear(); // Clear the display content.
  EPD_Update(); // Update the display.
}

void setup()
{
  Serial.begin(115200); // Initialize serial communication.

  //==================WiFi connection==================
  Serial.println("WiFi:");
  Serial.println(ID);
  Serial.println("PASSWORLD:");
  Serial.println(PASSWORD);

  WiFi.begin(ID, PASSWORD); // Connect to WiFi network.

  while (WiFi.status()!= WL_CONNECTED) // Wait for connection success.
  {
    delay(500);
    Serial.println("Connecting...");
  }

  Serial.println("Connected successfully!");
  //==================WiFi connection==================

  WeatherURL = GitURL(API, CITY); // Build the weather query URL.
  // Set the screen power pin as output mode and set it to high to turn on the power.
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);
  EPD_GPIOInit();  // Initialize screen GPIO.
  Paint_NewImage(ImageBW, EPD_W, EPD_H, 180, WHITE);  // Create new image canvas.
  Paint_Clear(WHITE);  // Clear the canvas.

  EPD_FastInit();  // Fast initialization of screen.
  EPD_Display_Clear();  // Clear the screen display.
  EPD_Update();  // Update the screen display.

  Serial.println("Connected successfully!");


}

void loop()
{
    ParseWeather(WeatherURL); // Parse weather information.

  PW(WeatherURL);
   delay(1000*60*60); // Main loop delay for 1 hour.
}


void PW(String url)
{
  DynamicJsonDocument doc(1024); // Allocate 1024 bytes of memory for storing JSON data.
  http.begin(url); // Initialize HTTP request and specify the request URL.

  int httpGet = http.GET(); // Send GET request and get HTTP status code.
  if (httpGet > 0) // Check if the request is successful.
  {
    Serial.printf("HTTPGET is %d\n", httpGet); // Print HTTP status code.

    if (httpGet == HTTP_CODE_OK) // Check if HTTP status code is 200 (OK).
    {
      String json = http.getString(); // Get the response JSON string.
      Serial.println(json); // Print the JSON string.

      deserializeJson(doc, json); // Parse the JSON string into doc.

      // Parse and extract various elements from the JSON data.
      String location = doc["results"][0]["location"]["name"].as<String>(); // City name.
      String weatherText = doc["results"][0]["now"]["text"].as<String>(); // Current weather description.
      String temperature = doc["results"][0]["now"]["temperature"].as<String>(); // Current temperature.
      String humidity = doc["results"][0]["now"]["humidity"].as<String>(); // Current humidity.
      String windSpeed = doc["results"][0]["now"]["wind"]["speed"].as<String>(); // Current wind speed.

      String Country = doc["results"][0]["location"]["country"].as<String>(); // Country.
      String Timezone = doc["results"][0]["location"]["timezone"].as<String>(); // Time zone.
      String last_update = doc["results"][0]["last_update"].as<String>(); // Last update time.

      // Create a character array for storing information.
      char buffer[40];

      // Clear the image and initialize the e-ink screen.
      clear_all(); // Clear the screen.
      EPD_FastInit(); // Fast initialization of e-ink screen.
      EPD_ShowPicture(0, 0, 16, 16, home_small, WHITE); // Display small icon.

      // Update time UI.
      EPD_ShowPicture(0, 185, 32, 32, weather_2, WHITE); // Display weather icon.
      EPD_ShowString(35, 200, "Last Time :", 16, BLACK); // Display "Last Time :" label.

      memset(buffer, 0, sizeof(buffer)); // Clear the buffer.
      snprintf(buffer, sizeof(buffer), "%s ", last_update.c_str()); // Format the last update time.
      EPD_ShowString(10, 220, buffer, 16, BLACK); // Display the last update time.

      // Update city UI.
      EPD_ShowString(35, 40, "City :", 16, BLACK); // Display "City :" label.
      EPD_ShowPicture(0, 30, 32, 32, city_1, WHITE); // Display city icon.

      memset(buffer, 0, sizeof(buffer)); // Clear the buffer.
      if (strcmp(location.c_str(), "Shenzhen") == 0) // Check if the city name is "Shenzhen".
      {
        snprintf(buffer, sizeof(buffer), " %s", "Sheng Zhen"); // If so, display "Sheng Zhen".
      } else
      {
        snprintf(buffer, sizeof(buffer), "%s", "Null"); // Otherwise, display "Null".
      }
      EPD_ShowString(10, 60, buffer, 16, BLACK); // Display the city name.

      // Update time zone UI.
      EPD_ShowString(35, 120, "time zone :", 16, BLACK); // Display "time zone :" label.
      EPD_ShowPicture(0, 100, 32, 32, time_zone1, WHITE); // Display time zone icon.

      memset(buffer, 0, sizeof(buffer)); // Clear the buffer.
      snprintf(buffer, sizeof(buffer), "%s ", Timezone.c_str()); // Format the time zone information.
      EPD_ShowString(0, 140, buffer, 16, BLACK); // Display the time zone information.

      // Update temperature UI.
      EPD_ShowString(300, 190, "Temp :", 24, BLACK); // Display "Temp :" label.
      EPD_ShowPicture(260, 190, 32, 32, temp1, WHITE); // Display temperature icon.

      memset(buffer, 0, sizeof(buffer)); // Clear the buffer.
      snprintf(buffer, sizeof(buffer), "%s C", temperature.c_str()); // Format the temperature information.
      EPD_ShowString(320, 210, buffer, 24, BLACK); // Display the temperature information.

      // Update weather UI.
      memset(buffer, 0, sizeof(buffer)); // Clear the buffer.
      if (strcmp(weatherText.c_str(), "heavy rain") == 0) // Check the weather description.
      {
        snprintf(buffer, sizeof(buffer), "Weather: %s", "heavy rain"); // If it's "heavy rain", display "heavy rain".
        EPD_ShowPicture(260, 50, 80, 80, heavy_rain1, WHITE); // Display "heavy rain" icon.

      } else if (strcmp(weatherText.c_str(), "cloudy") == 0) // If it's "cloudy", display "Cloudy".
      {
        snprintf(buffer, sizeof(buffer), "Weather: %s", "Cloudy");
        EPD_ShowPicture(260, 50, 80, 80, cloudy1, WHITE); // Display "cloudy" icon.
      }
      else if (strcmp(weatherText.c_str(), "small rain") == 0) // If it's "small rain", display "small rain".
      {
        snprintf(buffer, sizeof(buffer), "Weather: %s", "small rain");
        EPD_ShowPicture(260, 50, 80, 80, small_rain1, WHITE); // Display "small rain" icon.
      } else if (strcmp(weatherText.c_str(), "clear") == 0) // If it's "clear", display "clear day".
      {
        snprintf(buffer, sizeof(buffer), "Weather: %s", "clear day");
        EPD_ShowPicture(260, 50, 80, 80, clear_day1, WHITE); // Display "clear day" icon.
      }

      EPD_ShowString(260, 140, buffer, 16, BLACK); // Display the weather description.

      // Update partition line UI.
      EPD_DrawLine(0, 30, 416, 30, BLACK); // Draw horizontal dividing line.
      EPD_DrawLine(230, 30, 230, 240, BLACK); // Draw vertical dividing line.
      EPD_DrawLine(0, 170, 416, 170, BLACK); // Draw another horizontal dividing line.
      EPD_DrawLine(0, 85, 230, 85, BLACK); // Draw another horizontal dividing line.

      // Update the e-ink screen display content.
      EPD_Display(ImageBW); // Refresh the screen display.
      EPD_Update(); // Refresh the screen.
      EPD_DeepSleep(); // Enter deep sleep mode.

      // Update the Weather variable.
      Weather = weatherText; // Store the current weather description in the Weather variable.
    }
    else
    {
      Serial.println("ERROR1!!"); // Print error message.
    }
  }
  else
  {
    Serial.println("ERROR2!!"); // Print error message.
  }
  http.end(); // End HTTP request.
}