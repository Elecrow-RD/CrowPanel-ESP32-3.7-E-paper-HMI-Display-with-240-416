#include <Arduino.h>
#include "EPD.h"
#include "EPD_GUI.h"

// Define global variables to store button press counts
uint8_t ImageBW[12480];  // Buffer to store the image for display

// Button pin definitions
#define HOME_KEY 2
int HOME_NUM = 0;  // Counter for HOME button presses

#define EXIT_KEY 1
int EXIT_NUM = 0;  // Counter for EXIT button presses

#define PRV_KEY 6
int PRV_NUM = 0;   // Counter for PREVIOUS button presses

#define NEXT_KEY 4
int NEXT_NUM = 0;  // Counter for NEXT button presses

#define OK_KEY 5
int OK_NUM = 0;    // Counter for OK button presses

int NUM_btn[5] = {0};  // Array to store all button press counts

// Function: Update the button press count information to the screen
void count_btn(int NUM[5])
{
  char buffer[30];  // Buffer to store strings
  EPD_GPIOInit();   // Initialize screen GPIO
  Paint_NewImage(ImageBW, EPD_W, EPD_H, 180, WHITE);  // Create a new image with a white background
  Paint_Clear(WHITE); // Clear the canvas, prepare to draw new content

  EPD_FastInit();   // Fast initialize the screen
  EPD_Display_Clear();  // Clear the screen content
  EPD_Update();     // Update the display, prepare to show new content
  EPD_PartInit();   // Partial refresh initialization, prepare to draw text

  // Display the press count for each button
  int length = sprintf(buffer, "HOME_KEY_NUM:%d", NUM[0]);
  buffer[length] = '\0';
  EPD_ShowString(0, 0 + 0 * 20, buffer, 16, BLACK);  // Display the string at a specified position on the screen
  memset(buffer, 0, sizeof(buffer));  // Clear the buffer

  length = sprintf(buffer, "EXIT_KEY_NUM:%d", NUM[1]);
  buffer[length] = '\0';
  EPD_ShowString(0, 0 + 1 * 20, buffer, 16, BLACK);
  memset(buffer, 0, sizeof(buffer));

  length = sprintf(buffer, "PRV_KEY_NUM:%d", NUM[2]);
  buffer[length] = '\0';
  EPD_ShowString(0, 0 + 2 * 20, buffer, 16, BLACK);
  memset(buffer, 0, sizeof(buffer));

  length = sprintf(buffer, "NEXT_KEY_NUM:%d", NUM[3]);
  buffer[length] = '\0';
  EPD_ShowString(0, 0 + 3 * 20, buffer, 16, BLACK);
  memset(buffer, 0, sizeof(buffer));

  length = sprintf(buffer, "OK_KEY_NUM:%d", NUM[4]);
  buffer[length] = '\0';
  EPD_ShowString(0, 0 + 4 * 20, buffer, 16, BLACK);

  EPD_Display(ImageBW);  // Display the updated image on the screen
  EPD_Update();   // Update the display
  EPD_DeepSleep();  // Screen goes into deep sleep to save energy
}

void setup() {
  Serial.begin(115200);  // Initialize serial communication at 115200 baud rate

  // Set the screen power pin as output and set its level to high to power the screen
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);
  
  // Set button pins as input
  pinMode(HOME_KEY, INPUT);
  pinMode(EXIT_KEY, INPUT);
  pinMode(PRV_KEY, INPUT);
  pinMode(NEXT_KEY, INPUT);
  pinMode(OK_KEY, INPUT);
}

void loop() {
  // Main loop
  int flag = 0;  // Flag to indicate if any button has been pressed

  // Check if any button is pressed
  if (digitalRead(HOME_KEY) == 0)
  {
    delay(100);  // Delay for debouncing
    if (digitalRead(HOME_KEY) == 1)
    {
      Serial.println("HOME_KEY");  // Print the button name to the serial port
      HOME_NUM++;  // Increment the HOME button press counter
      flag = 1;  // Set the flag to 1, indicating a button was pressed
    }
  }
  else if (digitalRead(EXIT_KEY) == 0)
  {
    delay(100);
    if (digitalRead(EXIT_KEY) == 1)
    {
      Serial.println("EXIT_KEY");
      EXIT_NUM++;
      flag = 1;
    }
  }
  else if (digitalRead(PRV_KEY) == 0)
  {
    delay(100);
    if (digitalRead(PRV_KEY) == 1)
    {
      Serial.println("PRV_KEY");
      PRV_NUM++;
      flag = 1;
    }
  }
  else if (digitalRead(NEXT_KEY) == 0)
  {
    delay(100);
    if (digitalRead(NEXT_KEY) == 1)
    {
      Serial.println("NEXT_KEY");
      NEXT_NUM++;
      flag = 1;
    }
  }
  else if (digitalRead(OK_KEY) == 0)
  {
    delay(100);
    if (digitalRead(OK_KEY) == 1)
    {
      Serial.println("OK_KEY");
      OK_NUM++;
      flag = 1;
    }
  }

  // If a button was pressed, update the button press count information to the screen
  if (flag == 1)
  {
    NUM_btn[0] = HOME_NUM;
    NUM_btn[1] = EXIT_NUM;
    NUM_btn[2] = PRV_NUM;
    NUM_btn[3] = NEXT_NUM;
    NUM_btn[4] = OK_NUM;

    count_btn(NUM_btn);  // Call the function to display the updated button press count information on the screen
    flag = 0;  // Reset the flag
  }
}