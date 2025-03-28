#include <Arduino.h>
#include "EPD.h"
#include "EPD_GUI.h"
#include "pic_scenario.h"
uint8_t ImageBW[12480];

void setup() {
  // Put your setup code here, to run once:
  // Screen power
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);

  EPD_GPIOInit();
  Paint_NewImage(ImageBW, EPD_W, EPD_H, Rotation, WHITE);
  Paint_Clear(WHITE); // Clear the canvas

  EPD_FastInit();
  EPD_Display_Clear();
  EPD_Update(); 

  // Boot screen
  //  EPD_PartInit();
  EPD_FastInit();
  EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_scenario_home, WHITE);
  EPD_Display(ImageBW);
  EPD_Update();
  EPD_DeepSleep();
}

void loop() {
  // Put your main code here, to run repeatedly:

}