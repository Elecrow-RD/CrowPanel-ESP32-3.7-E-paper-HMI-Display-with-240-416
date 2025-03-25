#include <WiFi.h>               // Include the WiFi library for wireless network functionality
#include <HTTPClient.h>         // Include the HTTPClient library for handling HTTP requests
#include <Arduino_JSON.h>       // Include the Arduino_JSON library for JSON data processing
#include "EPD.h"                // Include the EPD library for controlling the electronic ink screen (E-Paper Display)
#include "EPD_GUI.h"            // Include the EPD_GUI library for graphical user interface (GUI) operations

#include "pic.h"                // Include image resources

// Define a black and white image array as the buffer for the e-paper display
uint8_t ImageBW[12480];        // Define the size based on the resolution of the e-paper display

const char* ssid = "yanfa_software";      // WiFi network name
const char* password = "yanfa-123456";    // WiFi password

String openWeatherMapApiKey = "You-API"; // OpenWeatherMap API key
String city = "London";  // City name
String countryCode = "2643743";  // Country code (city ID)

unsigned long lastTime = 0;         // Last update time
unsigned long timerDelay = 10000;    // Timer set to 10 seconds (10000 milliseconds)

String jsonBuffer;  // String to store JSON data
int httpResponseCode;  // HTTP response code
JSONVar myObject;  // JSON object for parsing JSON data

String weather;         // Weather condition
String temperature;     // Temperature
String humidity;        // Humidity
String sea_level;       // Sea level pressure
String wind_speed;      // Wind speed
String city_js;         // City information
int weather_flag = 0;   // Weather flag

// Clear the display
void clear_all() {
  Paint_NewImage(ImageBW, EPD_W, EPD_H, 180, WHITE); // Create a new image buffer
  Paint_Clear(WHITE); // Clear the canvas
  EPD_FastInit(); // Fast initialize the e-paper display
  EPD_Display_Clear(); // Clear the display content
  EPD_Update(); // Update the display
}

// Display the weather forecast interface
void UI_weather_forecast() {
  char buffer[40];  // Character array to store information

  EPD_GPIOInit();  // Initialize screen GPIO
  Paint_NewImage(ImageBW, EPD_W, EPD_H, 180, WHITE);  // Create a new image canvas
  Paint_Clear(WHITE);  // Clear the canvas

  EPD_FastInit();  // Fast initialize the screen
  EPD_Display_Clear();  // Clear the screen display
  EPD_Update();  // Update the screen display
  EPD_FastInit();  // Fast initialize the screen
  // Display background image and weather icon
  EPD_ShowPicture(0, 0, 416, 240, pic, WHITE);
  EPD_ShowPicture(14, 2, 184, 184, Weather_Num[weather_flag], WHITE);

  // Draw partition lines
  EPD_DrawLine(0, 180, 416, 180, BLACK); // Draw horizontal line
  EPD_DrawLine(215, 0, 215, 180, BLACK); // Draw vertical line
  EPD_DrawLine(215, 94, 416, 94, BLACK); // Draw horizontal line

  // Display city information
  memset(buffer, 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer), "%s ", city_js); // Format city information as a string
  EPD_ShowString(301, 51, buffer, 16, BLACK); // Display city name

  // Display temperature
  memset(buffer, 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer), "%s C", temperature); // Format temperature as a string
  EPD_ShowString(168, 217, buffer, 16, BLACK); // Display temperature

  // Display humidity
  memset(buffer, 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer), "%s ", humidity); // Format humidity as a string
  EPD_ShowString(301, 148, buffer, 16, BLACK); // Display humidity

  // Display wind speed
  memset(buffer, 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer), "%s m/s", wind_speed); // Format wind speed as a string
  EPD_ShowString(62, 217, buffer, 16, BLACK); // Display wind speed

  // Display sea level pressure
  memset(buffer, 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer), "%s ", sea_level); // Format sea level pressure as a string
  EPD_ShowString(324, 217, buffer, 16, BLACK); // Display sea level pressure

  // Update the e-ink display content
  EPD_Display(ImageBW); // Refresh the screen display
  EPD_Update(); // Refresh the screen
  EPD_DeepSleep(); // Enter deep sleep mode
}

void setup() {
  Serial.begin(115200);  // Initialize serial communication

  WiFi.begin(ssid, password);  // Connect to the WiFi network
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {  // Wait until connected to the WiFi network
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());  // Output the connected IP address

  Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");
  pinMode(7, OUTPUT);  // Set GPIO 7 to output mode
  digitalWrite(7, HIGH);  // Set GPIO 7 to high level to enable power

  // Initialize the e-paper display
  EPD_GPIOInit();  // Initialize the GPIO pins for the e-paper
}

void loop() {
  js_analysis();  // Call the function to parse JSON data (function body not shown)
  UI_weather_forecast();  // Display the weather forecast interface
  delay(1000 * 60 * 60); // Main loop delay for 1 hour
}

void js_analysis()
{
  // Check if WiFi is connected
  if (WiFi.status() == WL_CONNECTED) {
    // Construct the request path including city, country code, API key, and unit parameters
    String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey + "&units=metric";

    // Loop until data is successfully retrieved (HTTP response code is 200)
    while (httpResponseCode != 200)
    {
      // Send an HTTP GET request and get the returned JSON data
      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer); // Print the retrieved JSON data
      myObject = JSON.parse(jsonBuffer); // Parse the JSON data

      // Check if JSON parsing was successful
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!"); // Print an error message if parsing fails
        return; // Exit the function
      }
      delay(2000); // Wait 2 seconds before retrying
    }

    // Extract the required data from the JSON object
    weather = JSON.stringify(myObject["weather"][0]["main"]); // Weather condition
    temperature = JSON.stringify(myObject["main"]["temp"]); // Temperature
    humidity = JSON.stringify(myObject["main"]["humidity"]); // Humidity
    sea_level = JSON.stringify(myObject["main"]["sea_level"]); // Sea level pressure
    wind_speed = JSON.stringify(myObject["wind"]["speed"]); // Wind speed
    city_js = JSON.stringify(myObject["name"]); // City name

    // Print the extracted data
    Serial.print("String weather: ");
    Serial.println(weather);
    Serial.print("String Temperature: ");
    Serial.println(temperature);
    Serial.print("String humidity: ");
    Serial.println(humidity);
    Serial.print("String sea_level: ");
    Serial.println(sea_level);
    Serial.print("String wind_speed: ");
    Serial.println(wind_speed);
    Serial.print("String city_js: ");
    Serial.println(city_js);

    // Set the weather flag based on the weather condition
    if (weather.indexOf("clouds") != -1 || weather.indexOf("Clouds") != -1) {
      weather_flag = 1; // Cloudy
    } else if (weather.indexOf("clear sky") != -1 || weather.indexOf("Clear sky") != -1) {
      weather_flag = 3; // Sunny
    } else if (weather.indexOf("rain") != -1 || weather.indexOf("Rain") != -1) {
      weather_flag = 5; // Rainy
    } else if (weather.indexOf("thunderstorm") != -1 || weather.indexOf("Thunderstorm") != -1) {
      weather_flag = 2; // Thunderstorm
    } else if (weather.indexOf("snow") != -1 || weather.indexOf("Snow") != -1) {
      weather_flag = 4; // Snowy
    } else if (weather.indexOf("mist") != -1 || weather.indexOf("Mist") != -1) {
      weather_flag = 0; // Foggy
    }
  }
  else {
    Serial.println("WiFi Disconnected"); // Print an error message if WiFi is disconnected
  }
}

// HTTP GET request function
String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;

  // Initialize the HTTP client and specify the server address for the request
  http.begin(client, serverName);

  // Send the HTTP GET request
  httpResponseCode = http.GET();

  String payload = "{}"; // Default to an empty JSON object

  // Check the HTTP response code
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode); // Print the response code
    payload = http.getString(); // Get the response data
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode); // Print the error code
  }

  // Release resources
  http.end();

  return payload; // Return the response data
}