#include <Arduino.h>
#include "EPD.h"
#include "EPD_GUI.h"
#include "Pic_boot_screen.h"
#include "pic_menu.h"
#include "pic_home.h"
#include "pic_description.h"
#include "pic_image.h"
#include "pic_scenario.h"

#include "FS.h"
#include "SD.h"
#include <WiFi.h>

#define FILE_NAME "/book.txt"
#define SD_MOSI 40
#define SD_MISO 13
#define SD_SCK 39
#define SD_CS 10
SPIClass SD_SPI = SPIClass(HSPI);

#include "BLEDevice.h"              //BLE驱动库
#include "BLEServer.h"              //BLE蓝牙服务器库
#include "BLEUtils.h"               //BLE实用程序库
#include "BLE2902.h"                //特征添加描述符库
BLECharacteristic *pCharacteristic;
BLEServer *pServer;
BLEService *pService;
bool deviceConnected = false;
char BLEbuf[32] = {0};
uint32_t cnt = 0;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("------> BLE connect .");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("------> BLE disconnect .");
      pServer->startAdvertising(); // restart advertising
      Serial.println("start advertising");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.print("------>Received Value: ");

        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        Serial.println();

        if (rxValue.find("A") != -1) {
          Serial.print("Rx A!");
        }
        else if (rxValue.find("B") != -1) {
          Serial.print("Rx B!");
        }
        Serial.println();
      }
    }
};

//打印唤醒原因
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}

//主页键
#define HOME_KEY 2
//退出键
#define EXIT_KEY 1

//轮播开关
//上一页
#define PRV_KEY 6
//下一页
#define NEXT_KEY 4
//确认
#define OK_KEY 5

int peg_pos;  //页游标
int slide_pos; //界面游标
int8_t prv_menu_pos;
int three_interface_flag; //三级界面标志
int scenario_ctl;  //开关标志
int scenario_ctl1;
int scenario_ctl2;
int scenario_ctl3;
int scenario_ctl4;
int sleep_num; //进去睡眠计数
bool test_flag;
char val;
int KEY_NUM;

uint8_t ImageBW[12480];

void clear_all()
{
  Paint_NewImage(ImageBW, EPD_W, EPD_H, Rotation, WHITE);
  Paint_Clear(WHITE); //清空画布
  EPD_FastInit();
  EPD_Display_Clear();
  EPD_Update();  //局刷之前先对E-Paper进行清屏操作
}

void clear_all1()
{
  Paint_NewImage(ImageBW, EPD_W, EPD_H, Rotation, WHITE);
  Paint_Clear(WHITE); //清空画布
  EPD_PartInit();
  EPD_Display_Clear();
  EPD_Update();  //局刷之前先对E-Paper进行清屏操作
}

int KEY_SCAN()
{

  if (test_flag != false)
  {
    //    Serial.println(" ....... ");
    return 0;
  }

  KEY_NUM = 0;
  if (digitalRead(HOME_KEY) == 0)
  {
    delay(50);
    int i = 0;
    if (digitalRead(HOME_KEY) == 0)
    {
      Serial.println("HOME_KEY");
      sleep_num = 0;
      Serial.println("sleep now clear");
      while (digitalRead(HOME_KEY) == 0)  //长按5s清屏
      {
        i++;
        Serial.print("HOME_KEY KEEP ");
        Serial.println(i);
        delay(10);
        if (i == 500)
        {
          Serial.println("Display_Clear ....... ");
          clear_all();
          EPD_DeepSleep();
          while(1);
          return 0;
        }
      }
      KEY_NUM = 1;
    }
  }
  else if (digitalRead(EXIT_KEY) == 0)
  {
    delay(50);
    if (digitalRead(EXIT_KEY) == 0)
    {
      Serial.println("EXIT_KEY");
      KEY_NUM = 2;
    }
  }
  else if (digitalRead(PRV_KEY) == 0)
  {
    delay(50);
    if (digitalRead(PRV_KEY) == 0)
    {
      Serial.println("PRV_KEY");
      KEY_NUM = 3;
    }
  }
  else if (digitalRead(NEXT_KEY) == 0)
  {
    delay(50);
    if (digitalRead(NEXT_KEY) == 0)
    {
      Serial.println("NEXT_KEY");
      KEY_NUM = 4;
    }
  }
  else if (digitalRead(OK_KEY) == 0)
  {
    delay(50);
    if (digitalRead(OK_KEY) == 0)
    {
      Serial.println("OK_KEY");
      KEY_NUM = 5;
    }
  }

  return KEY_NUM;
}

void show_home_menu()
{
  clear_all();
  EPD_PartInit();
  EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_menu, WHITE);
  EPD_Display(ImageBW);
  EPD_Update();
  EPD_DeepSleep();

  peg_pos = 1;
  slide_pos = 0;
  prv_menu_pos = 0;
}

void show_home_menu1()
{
  clear_all();
  EPD_PartInit();
  EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_menu1, WHITE);
  EPD_Display(ImageBW);
  EPD_Update();
  EPD_DeepSleep();

  peg_pos = 1;
  slide_pos = 1;
  prv_menu_pos = 1;
}

void show_home_menu2()
{
  clear_all();
  EPD_PartInit();
  EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_menu2, WHITE);
  EPD_Display(ImageBW);
  EPD_Update();
  EPD_DeepSleep();

  peg_pos = 1;
  slide_pos = 2;
  prv_menu_pos = 2;
}

void show_home_menu_pos(int show_pos)
{
  switch (show_pos)
  {
    case 0:
      EPD_PartInit();
      EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_menu, WHITE);
      EPD_Display(ImageBW);
      EPD_Update();
      EPD_DeepSleep();

      peg_pos = 1;
      slide_pos = 0;
      prv_menu_pos = 0;
      break;
    case 1:
      EPD_PartInit();
      EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_menu1, WHITE);
      EPD_Display(ImageBW);
      EPD_Update();
      EPD_DeepSleep();

      peg_pos = 1;
      slide_pos = 1;
      prv_menu_pos = 1;
      break;
    case 2:
      EPD_PartInit();
      EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_menu2, WHITE);
      EPD_Display(ImageBW);
      EPD_Update();
      EPD_DeepSleep();

      peg_pos = 1;
      slide_pos = 2;
      prv_menu_pos = 2;
      break;
    default:
      break;
  }
}

void show_description(int show_pos)
{
  if (show_pos == 0)
  {
    slide_pos = 0;
  }
  switch (show_pos)
  {
    case 0:
      //第一页介绍
      //      clear_all1();
      EPD_FastInit();
      EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_description, WHITE);
      EPD_Display(ImageBW);
      EPD_Update();
      EPD_DeepSleep();

      peg_pos = 2;
      slide_pos = 0;
      prv_menu_pos = 0;
      three_interface_flag = 0;
      test_flag = false;

      break;
    case 1:
      //      clear_all1();
      EPD_FastInit();
      EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_description1, WHITE);
      EPD_Display(ImageBW);
      EPD_Update();
      EPD_DeepSleep();

      peg_pos = 2;
      slide_pos = 1;
      prv_menu_pos = 0;
      three_interface_flag = 0;
      test_flag = false;

      break;
    case 2:
      //      clear_all1();
      EPD_FastInit();
      EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_description2, WHITE);
      EPD_Display(ImageBW);
      EPD_Update();
      EPD_DeepSleep();

      peg_pos = 2;
      slide_pos = 2;
      prv_menu_pos = 0;
      three_interface_flag = 0;
      test_flag = false;

      break;
    case 3:
      //      clear_all1();
      EPD_FastInit();
      EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_description3, WHITE);
      EPD_Display(ImageBW);
      EPD_Update();
      EPD_DeepSleep();

      peg_pos = 2;
      slide_pos = 3;
      prv_menu_pos = 0;
      three_interface_flag = 0;
      test_flag = false;

      break;
    default:
      break;
  }
}

void show_image_example(int show_pos)
{
  if (show_pos == 0)
  {
    slide_pos = 0;
  }
  switch (show_pos)
  {
    case 0:
//      clear_all1();
      EPD_FastInit();
      EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_image, WHITE);
      EPD_Display(ImageBW);
      EPD_Update();
      EPD_DeepSleep();

      peg_pos = 2;
      prv_menu_pos = 1;
      slide_pos = 0;
      three_interface_flag = 1;
      test_flag = false;
      break;
    case 1:
//      clear_all1();
      EPD_FastInit();
      EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_image1, WHITE);
      EPD_Display(ImageBW);
      EPD_Update();
      EPD_DeepSleep();

      peg_pos = 2;
      prv_menu_pos = 1;
      slide_pos = 1;
      three_interface_flag = 1;
      test_flag = false;
      break;
    case 2:
//      clear_all1();
      EPD_FastInit();
      EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_image2, WHITE);
      EPD_Display(ImageBW);
      EPD_Update();
      EPD_DeepSleep();

      peg_pos = 2;
      prv_menu_pos = 1;
      slide_pos = 2;
      three_interface_flag = 1;
      test_flag = false;
      break;
    case 3:
//      clear_all1();
      EPD_FastInit();
      EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_image3, WHITE);
      EPD_Display(ImageBW);
      EPD_Update();
      EPD_DeepSleep();

      peg_pos = 2;
      prv_menu_pos = 1;
      slide_pos = 3;
      three_interface_flag = 1;
      test_flag = false;
      break;
    case 4:
//      clear_all1();
      EPD_FastInit();
      EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_image4, WHITE);
      EPD_Display(ImageBW);
      EPD_Update();
      EPD_DeepSleep();

      peg_pos = 2;
      prv_menu_pos = 1;
      slide_pos = 4;
      three_interface_flag = 1;
      test_flag = false;
      break;
    case 5:
//      clear_all1();
      EPD_FastInit();
      EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_image5, WHITE);
      EPD_Display(ImageBW);
      EPD_Update();
      EPD_DeepSleep();

      peg_pos = 2;
      prv_menu_pos = 1;
      slide_pos = 5;
      three_interface_flag = 1;
      test_flag = false;
      break;
    case 6:
//      clear_all1();
      EPD_FastInit();
      EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_image6, WHITE);
      EPD_Display(ImageBW);
      EPD_Update();
      EPD_DeepSleep();

      peg_pos = 2;
      prv_menu_pos = 1;
      slide_pos = 6;
      three_interface_flag = 1;
      test_flag = false;
      break;
    default:
      break;
  }
}

void show_scenario_example_pos(int show_pos)  //scenario 显示游标
{
  if (show_pos == 0)
  {
    slide_pos = 0;
  }
  switch (show_pos)
  {
    case 0:
      EPD_FastInit();
      EPD_ShowPicture(40, 388, 160, 8, gImage_scenario_pos, WHITE);
      EPD_Display(ImageBW);
      EPD_Update();
      EPD_DeepSleep();

      slide_pos = 0;
      peg_pos = 2;
      prv_menu_pos = 2;
      three_interface_flag = 2;
      test_flag = false;
      break;
    case 1:
      EPD_FastInit();
      EPD_ShowPicture(40, 388, 160, 8, gImage_scenario_pos1, WHITE);
      EPD_Display(ImageBW);
      EPD_Update();
      EPD_DeepSleep();

      slide_pos = 1;
      peg_pos = 2;
      prv_menu_pos = 2;
      three_interface_flag = 2;
      test_flag = false;
      break;
    case 2:
      EPD_FastInit();
      EPD_ShowPicture(40, 388, 160, 8, gImage_scenario_pos2, WHITE);
      EPD_Display(ImageBW);
      EPD_Update();
      EPD_DeepSleep();

      slide_pos = 2;
      peg_pos = 2;
      prv_menu_pos = 2;
      three_interface_flag = 2;
      test_flag = false;
      break;
    case 3:
      EPD_FastInit();
      EPD_ShowPicture(40, 388, 160, 8, gImage_scenario_pos3, WHITE);
      EPD_Display(ImageBW);
      EPD_Update();
      EPD_DeepSleep();

      slide_pos = 3;
      peg_pos = 2;
      prv_menu_pos = 2;
      three_interface_flag = 2;
      test_flag = false;
      break;
    case 4:
      EPD_FastInit();
      EPD_ShowPicture(40, 388, 160, 8, gImage_scenario_pos4, WHITE);
      EPD_Display(ImageBW);
      EPD_Update();
      EPD_DeepSleep();

      slide_pos = 4;
      prv_menu_pos = 2;
      peg_pos = 2;
      three_interface_flag = 2;
      test_flag = false;
      break;
    default:
      break;
  }
}

void show_scenario_example_ctl(int show_pos) //scenario 显示开关
{
  if (show_pos == 0)
  {
    slide_pos = 0;
  }

  switch (show_pos)
  {
    case 0:
      if (scenario_ctl == 0)
      {
        EPD_FastInit();
        EPD_ShowPicture(40, 39, 24, 160, gImage_scenario_ctl_on, WHITE);
        EPD_Display(ImageBW);
        EPD_Update();
        EPD_DeepSleep();

        scenario_ctl = 1;
      }
      else if (scenario_ctl == 1)
      {
        EPD_FastInit();
        EPD_ShowPicture(40, 39, 24, 160, gImage_scenario_ctl_off, WHITE);
        EPD_Display(ImageBW);
        EPD_Update();
        EPD_DeepSleep();

        scenario_ctl = 0;
      }
      break;
    case 1:
      if (scenario_ctl1 == 0)
      {
        EPD_FastInit();
        EPD_ShowPicture(74, 39, 24, 160, gImage_scenario_ctl_on, WHITE);
        EPD_Display(ImageBW);
        EPD_Update();
        EPD_DeepSleep();

        scenario_ctl1 = 1;
      }
      else if (scenario_ctl1 == 1)
      {
        EPD_FastInit();
        EPD_ShowPicture(74, 39, 24, 160, gImage_scenario_ctl_off, WHITE);
        EPD_Display(ImageBW);
        EPD_Update();
        EPD_DeepSleep();

        scenario_ctl1 = 0;
      }
      break;
    case 2:
      if (scenario_ctl2 == 0)
      {
        EPD_FastInit();
        EPD_ShowPicture(108, 39, 24, 160, gImage_scenario_ctl_on, WHITE);
        EPD_Display(ImageBW);
        EPD_Update();
        EPD_DeepSleep();

        scenario_ctl2 = 1;
      }
      else if (scenario_ctl2 == 1)
      {
        EPD_FastInit();
        EPD_ShowPicture(108, 39, 24, 160, gImage_scenario_ctl_off, WHITE);
        EPD_Display(ImageBW);
        EPD_Update();
        EPD_DeepSleep();

        scenario_ctl2 = 0;
      }
      break;
    case 3:
      if (scenario_ctl3 == 0)
      {
        EPD_FastInit();
        EPD_ShowPicture(142, 39, 24, 160, gImage_scenario_ctl_on, WHITE);
        EPD_Display(ImageBW);
        EPD_Update();
        EPD_DeepSleep();

        scenario_ctl3 = 1;
      }
      else if (scenario_ctl3 == 1)
      {
        EPD_FastInit();
        EPD_ShowPicture(142, 39, 24, 160, gImage_scenario_ctl_off, WHITE);
        EPD_Display(ImageBW);
        EPD_Update();
        EPD_DeepSleep();

        scenario_ctl3 = 0;
      }
      break;
    case 4:
      if (scenario_ctl4 == 0)
      {
        EPD_FastInit();
        EPD_ShowPicture(176, 39, 24, 160, gImage_scenario_ctl_on, WHITE);
        EPD_Display(ImageBW);
        EPD_Update();
        EPD_DeepSleep();

        scenario_ctl4 = 1;
      }
      else if (scenario_ctl4 == 1)
      {
        EPD_FastInit();
        EPD_ShowPicture(176, 39, 24, 160, gImage_scenario_ctl_off, WHITE);
        EPD_Display(ImageBW);
        EPD_Update();
        EPD_DeepSleep();

        scenario_ctl4 = 0;
      }
      break;
    default:
      break;
  }
}

void setup()
{
  sleep_num = 0;
  Serial.begin(115200);
  //POWER灯
  pinMode(41, OUTPUT);
  digitalWrite(41, HIGH);
  //屏电源
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);
  //SD卡电源
  pinMode(42, OUTPUT);
  digitalWrite(42, HIGH);

  pinMode(HOME_KEY, INPUT);
  pinMode(EXIT_KEY, INPUT);
  pinMode(PRV_KEY, INPUT);
  pinMode(NEXT_KEY, INPUT);
  pinMode(OK_KEY, INPUT);
  //GPIO输出模式
  pinMode(8, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(20, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(38, OUTPUT);
  pinMode(21, OUTPUT);
  //  pinMode(40, OUTPUT);
  //  pinMode(39, OUTPUT);

  //Print the wakeup reason for ESP32
  print_wakeup_reason();
  //First we configure the wake up source
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 0); //1 = High, 0 = Low

  // Create the BLE Device
  BLEDevice::init("CrowPanel3-7");
  // 创建蓝牙服务器
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // 创建广播服务的UUID
  pService = pServer->createService(SERVICE_UUID);
  //  //创建广播服务的UUID
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());
  //  //  pServer->getAdvertising()->start();

  EPD_GPIOInit();
  Paint_NewImage(ImageBW, EPD_W, EPD_H, Rotation, WHITE);
  Paint_Clear(WHITE); //清空画布

  EPD_FastInit();
  EPD_Display_Clear();
  EPD_Update();  //局刷之前先对E-Paper进行清屏操作

  //开机界面
  //  EPD_PartInit();
  EPD_FastInit();
  EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_boot_setup, WHITE);
  EPD_Display(ImageBW);
  EPD_Update();
  EPD_DeepSleep();

  delay(500);

  //  EPD_PartInit();
  EPD_FastInit();
  EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_boot_setup_1, WHITE);
  EPD_Display(ImageBW);
  EPD_Update();
  EPD_DeepSleep();

  delay(500);
  //  EPD_PartInit();
  EPD_FastInit();
  EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_boot_setup_info, WHITE);
  EPD_Display(ImageBW);
  EPD_Update();
  EPD_DeepSleep();

  delay(500);
  
  clear_all();
  EPD_PartInit();
  //  EPD_FastInit();
  EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_menu, WHITE);
  EPD_Display(ImageBW);
  EPD_Update();
  EPD_DeepSleep();


  peg_pos = 1;
  slide_pos = 0;
  prv_menu_pos = 0;
  three_interface_flag = 0;
  test_flag = false;
  scenario_ctl = 0;
  scenario_ctl1 = 0;
  scenario_ctl2 = 0;
  scenario_ctl3 = 0;
  scenario_ctl4 = 0;

}

void test_title()
{
  //  clear_all();

  EPD_GPIOInit();
  Paint_NewImage(ImageBW, EPD_W, EPD_H, 180, WHITE);
  Paint_Clear(WHITE); //清空画布

  EPD_FastInit();
  EPD_Display_Clear();
  EPD_Update();  //局刷之前先对E-Paper进行清屏操作

  EPD_PartInit();
  EPD_ShowString(130, 100, "Test Program", 24, BLACK);
  EPD_Display(ImageBW);
  EPD_Update();
  EPD_DeepSleep();

}
void key_test()
{
  char command[64] = {0};   //串口收到的命令
  char count = 0;           //命令的长度
  while (1)
  {
    while (Serial.available())
    {
      val = Serial.read();
      Serial.print("read key com data: ");
      Serial.print(val);
      Serial.println("");
      command[count] = val;
      count++;
    }
    if (command[0] == 'c')
    {
      Serial.println("EXIT key test");
      break;
    }

    if (digitalRead(HOME_KEY) == 0)
    {
      delay(100);
      if (digitalRead(HOME_KEY) == 0)
      {
        Serial.println("HOME_KEY");
      }
    }
    else if (digitalRead(EXIT_KEY) == 0)
    {
      delay(100);
      if (digitalRead(EXIT_KEY) == 0)
      {
        Serial.println("EXIT_KEY");
      }
    }
    else if (digitalRead(PRV_KEY) == 0)
    {
      delay(100);
      if (digitalRead(PRV_KEY) == 0)
      {
        Serial.println("PRV_KEY");
      }
    }
    else if (digitalRead(NEXT_KEY) == 0)
    {
      delay(100);
      if (digitalRead(NEXT_KEY) == 0)
      {
        Serial.println("NEXT_KEY");
      }
    }
    else if (digitalRead(OK_KEY) == 0)
    {
      delay(100);
      if (digitalRead(OK_KEY) == 0)
      {
        Serial.println("OK_KEY");
      }
    }
  }
}

void wifi_test()
{
  char command[64] = {0};   //串口收到的命令
  char count = 0;           //命令的长度
  bool flag = false;
  bool timeout_flag = false;
  while (1)
  {
    while (Serial.available())
    {
      String ssid = Serial.readStringUntil(','); // 读取 SSID
      String password = Serial.readStringUntil('\n'); // 读取密码
      // 在这里处理 SSID 和密码
      Serial.print("SSID: ");
      Serial.println(ssid);
      Serial.print("Password: ");
      Serial.println(password);
      WiFi.begin(ssid, password);
      int timeout = 0;
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        timeout++;
        if (timeout == 40)
        {
          Serial.println("Connection timeout exit");
          Serial.println("EXIT WIFI test");
          char command[64] = {0};   //串口收到的命令
          char count = 0;           //命令的长度
          return;
        }
      }
      if (timeout_flag == false)
      {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        char command[64] = {0};   //串口收到的命令
        char count = 0;           //命令的长度
        flag = true;
      }
    }
    if (flag == true)
    {
      Serial.println("EXIT WIFI test");
      char command[64] = {0};   //串口收到的命令
      char count = 0;           //命令的长度
      break;
    }
    delay(10);
  }
}

void ble_test()
{
  char command[64] = {0};   //串口收到的命令
  char count = 0;           //命令的长度
  // Create the BLE Device
  //  BLEDevice::init("CrowPanel-ESP32S3");
  //  // 创建蓝牙服务器
  //  BLEServer *pServer = BLEDevice::createServer();
  //  pServer->setCallbacks(new MyServerCallbacks());
  //  // 创建广播服务的UUID
  //  BLEService *pService = pServer->createService(SERVICE_UUID);
  //  //创建广播服务的UUID
  //  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  //  pCharacteristic->addDescriptor(new BLE2902());
  //  BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
  //  pCharacteristic->setCallbacks(new MyCallbacks());

  // 开始蓝牙服务
  pService->start();
  // 开始广播
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

  while (1)
  {
    while (Serial.available())
    {
      val = Serial.read();
      Serial.print("read ble com data: ");
      Serial.print(val);
      Serial.println("");
      command[count] = val;
      count++;
    }
    if (command[0] == 'b')
    {
      Serial.println("EXIT ble test");
      pServer->getAdvertising()->stop();//停止广播
      pService->stop();
      break;
    }

    if (deviceConnected) {//设备连接后，每秒钟发送txValue。
      memset(BLEbuf, 0, 32);
      memcpy(BLEbuf, (char*)"Hello BLE APP!", 32);
      pCharacteristic->setValue(BLEbuf);

      pCharacteristic->notify(); // Send the value to the app!
      Serial.print("*** Sent Value: ");
      Serial.print(BLEbuf);
      Serial.println(" ***");
    }
    delay(1000);
  }
}

//测试程序任务
void test_task()
{
  char command[64] = {0};   //串口收到的命令
  char count = 0;           //命令的长度
  while (Serial.available())
  {
    val = Serial.read();
    Serial.print("read com data: ");
    Serial.print(val);
    Serial.println("");
    command[count] = val;
    count++;
  }
  switch (command[0])
  {
    case 'T':
      Serial.println("Entry test");
      test_flag = true;
      test_title();
      break;
    case 'I':
      Serial.println("Exit test");
      test_flag = false;
      esp_restart();  //退出测试重启
      break;
    case 'U':
      if (test_flag == true)
      {
        Serial.println("Display test");
        EPD_GPIOInit();
        clear_all();
        EPD_PartInit();
        EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_image, WHITE);
        EPD_Display(ImageBW);
        EPD_Update();
        EPD_DeepSleep();
      }
      break;
    case 'u':
      if (test_flag == true)
      {
        Serial.println("Exit display test");
        test_title();
      }
      break;
    case 'C':
      if (test_flag == true)
      {
        Serial.println("Key test");
        key_test();
      }
      break;
    case 'P':
      if (test_flag == true)
      {
        Serial.println("GPIO test out high");
        //GPIO测试
        digitalWrite(8, HIGH);
        digitalWrite(3, HIGH);
        digitalWrite(14, HIGH);
        digitalWrite(9, HIGH);
        digitalWrite(16, HIGH);
        digitalWrite(15, HIGH);
        digitalWrite(18, HIGH);
        digitalWrite(17, HIGH);
        digitalWrite(20, HIGH);
        digitalWrite(19, HIGH);
        digitalWrite(38, HIGH);
        digitalWrite(21, HIGH);
        //        digitalWrite(40, HIGH);
        //        digitalWrite(39, HIGH);
      }
      break;
    case 'p':
      if (test_flag == true)
      {
        Serial.println("Exit gpio test out low");
        digitalWrite(8, LOW);
        digitalWrite(3, LOW);
        digitalWrite(14, LOW);
        digitalWrite(9, LOW);
        digitalWrite(16, LOW);
        digitalWrite(15, LOW);
        digitalWrite(18, LOW);
        digitalWrite(17, LOW);
        digitalWrite(20, LOW);
        digitalWrite(19, LOW);
        digitalWrite(38, LOW);
        digitalWrite(21, LOW);
        //        digitalWrite(40, LOW);
        //        digitalWrite(39, LOW);
      }
      break;
    case 'S':
      if (test_flag == true)
      {
        Serial.println("SD test");
        SD_SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
        if (!SD.begin(SD_CS, SD_SPI, 80000000))
        {
          Serial.println(F("ERROR: File system mount failed!"));
        }
        else
        {
          Serial.printf("SD Size： %lluMB \n", SD.cardSize() / (1024 * 1024));
        }
      }
      break;
    case 's':
      if (test_flag == true)
      {
        Serial.println("Exit SD test");
        SD.end();
      }
      break;
    case 'A':
      if (test_flag == true)
      {
        Serial.println("WIFI test");
        delay(1000);
        wifi_test();
      }
      break;
    case 'B':
      if (test_flag == true)
      {
        Serial.println("BLE test");
        ble_test();
      }
      break;
    case 'L':
      if (test_flag == true)
      {
        Serial.println("LED test");
        digitalWrite(41, LOW);
      }
      break;
    case 'l':
      if (test_flag == true)
      {
        Serial.println("Exit LED test");
        digitalWrite(41, HIGH);
      }
      break;
    case 'G':
      if (test_flag == true)
      {
        Serial.println("Pressure test");
        // 开始蓝牙服务
        pService->start();
        // 开始广播
        pServer->getAdvertising()->start();
        //        WiFi.mode(WIFI_AP_STA);
        if (!WiFi.softAP("CrowPanel3-7", "")) {
          return ;
        }
        bool out = false;
        while (1)
        {
          char command[64] = {0};   //串口收到的命令
          char count = 0;           //命令的长度
          while (Serial.available())
          {
            val = Serial.read();
            Serial.print("read com data: ");
            Serial.print(val);
            Serial.println("");
            command[count] = val;
            count++;
          }
          if (val == 'g')
          {
            Serial.println("Exit pressure test");
            pServer->getAdvertising()->stop();//停止广播
            pService->stop();

            digitalWrite(8, LOW);
            digitalWrite(3, LOW);
            digitalWrite(14, LOW);
            digitalWrite(9, LOW);
            digitalWrite(16, LOW);
            digitalWrite(15, LOW);
            digitalWrite(18, LOW);
            digitalWrite(17, LOW);
            digitalWrite(20, LOW);
            digitalWrite(19, LOW);
            digitalWrite(38, LOW);
            digitalWrite(21, LOW);
            //            digitalWrite(40, LOW);
            //            digitalWrite(39, LOW);
            return;
          }

          if (out == false)
          {
            digitalWrite(8, HIGH);
            digitalWrite(3, HIGH);
            digitalWrite(14, HIGH);
            digitalWrite(9, HIGH);
            digitalWrite(16, HIGH);
            digitalWrite(15, HIGH);
            digitalWrite(18, HIGH);
            digitalWrite(17, HIGH);
            digitalWrite(20, HIGH);
            digitalWrite(19, HIGH);
            digitalWrite(38, HIGH);
            digitalWrite(21, HIGH);
            //            digitalWrite(40, HIGH);
            //            digitalWrite(39, HIGH);
            out = true;
          }
          else
          {
            digitalWrite(8, LOW);
            digitalWrite(3, LOW);
            digitalWrite(14, LOW);
            digitalWrite(9, LOW);
            digitalWrite(16, LOW);
            digitalWrite(15, LOW);
            digitalWrite(18, LOW);
            digitalWrite(17, LOW);
            digitalWrite(20, LOW);
            digitalWrite(19, LOW);
            digitalWrite(38, LOW);
            digitalWrite(21, LOW);
            //            digitalWrite(40, LOW);
            //            digitalWrite(39, LOW);
            out = false;
          }
          delay(500);
        }
      }
      break;
    default:
      break;
  }

}

void loop()
{
  //  Serial.println("******************************  home");
  //测试串口检测
  test_task();
  if (KEY_SCAN() != 0 && test_flag == false)
  {
    sleep_num = 0;
    Serial.print("KEY_NUM CODE: ");
    Serial.println(KEY_NUM);
    switch (KEY_NUM)
    {
      case 1:
        //返回主页
        //        clear_all();
        show_home_menu();
        Serial.println("return home");
        three_interface_flag = 0;
        scenario_ctl = 0;
        scenario_ctl1 = 0;
        scenario_ctl2 = 0;
        scenario_ctl3 = 0;
        scenario_ctl4 = 0;
        peg_pos = 1; //进入2级界面，功能页选择
        slide_pos = 0;
        prv_menu_pos = 0;
        three_interface_flag = 0;
        Serial.print("peg_pos : ");
        Serial.println(peg_pos);
        break;
      case 2:
        //返回
        if (peg_pos == 1 || peg_pos == 2)
        {
          peg_pos = 1;
          if (prv_menu_pos == 0)
          {
            slide_pos = 0;
            prv_menu_pos = 0;
            three_interface_flag = 0;
            scenario_ctl = 0;
            scenario_ctl1 = 0;
            scenario_ctl2 = 0;
            scenario_ctl3 = 0;
            scenario_ctl4 = 0;
            show_home_menu();
          }
          else if (prv_menu_pos == 1)
          {
            slide_pos = 1;
            prv_menu_pos = 1;
            three_interface_flag = 0;
            scenario_ctl = 0;
            scenario_ctl1 = 0;
            scenario_ctl2 = 0;
            scenario_ctl3 = 0;
            scenario_ctl4 = 0;
            show_home_menu1();
          }
          else if (prv_menu_pos == 2)
          {
            slide_pos = 2;
            prv_menu_pos = 2;
            three_interface_flag = 0;
            scenario_ctl = 0;
            scenario_ctl1 = 0;
            scenario_ctl2 = 0;
            scenario_ctl3 = 0;
            scenario_ctl4 = 0;
            show_home_menu2();
          }
        }
        break;
      case 3:
        //2级界面上滑
        if (peg_pos == 1)
        {
          slide_pos = slide_pos - 1;
          if (slide_pos < 0)
          {
            slide_pos = 0;
            return ;
          }
          Serial.print("show_home_menu_pos-> slide_pos : ");
          Serial.println(slide_pos);
          Serial.println("");
          show_home_menu_pos(slide_pos);
        }
        //在3级界面上滑
        else if (peg_pos == 2)
        {
          slide_pos = slide_pos - 1;
          if (slide_pos < 0)
          {
            slide_pos = 0;
            return ;
          }
          if (three_interface_flag == 0)
          {
            Serial.print("show_description-> slide_pos : ");
            Serial.println(slide_pos);
            Serial.println("");
            show_description(slide_pos);
          }
          else if (three_interface_flag == 1)
          {
            Serial.print("show_image_example-> slide_pos : ");
            Serial.println(slide_pos);
            Serial.println("");
            show_image_example(slide_pos);
          }
          else if (three_interface_flag == 2)
          {
            Serial.print("show_scenario_example_pos-> slide_pos : ");
            Serial.println(slide_pos);
            Serial.println("");
            show_scenario_example_pos(slide_pos);
          }
        }
        break;
      case 4:
        //2级界面下滑
        if (peg_pos == 1)
        {
          slide_pos = slide_pos + 1;
          if (slide_pos > 2)
          {
            slide_pos = 2;
            return ;
          }
          Serial.print("show_home_menu_pos -> slide_pos : ");
          Serial.println(slide_pos);
          Serial.println("");
          show_home_menu_pos(slide_pos);
        }
        //3级界面下滑
        else if (peg_pos == 2)
        {
          if (three_interface_flag == 0)
          {
            slide_pos = slide_pos + 1;
            if (slide_pos > 3)
            {
              slide_pos = 3;
              return ;
            }
            Serial.print("show_description -> slide_pos : ");
            Serial.println(slide_pos);
            Serial.println("");
            show_description(slide_pos);
          }
          else if (three_interface_flag == 1)
          {
            slide_pos = slide_pos + 1;
            if (slide_pos > 6)
            {
              slide_pos = 6;
              return ;
            }
            Serial.print("show_image_example -> slide_pos : ");
            Serial.println(slide_pos);
            Serial.println("");
            show_image_example(slide_pos);
          }
          else if (three_interface_flag == 2)
          {
            slide_pos = slide_pos + 1;
            if (slide_pos > 4)
            {
              slide_pos = 4;
              return ;
            }
            Serial.print("show_scenario_example_pos -> slide_pos : ");
            Serial.println(slide_pos);
            Serial.println("");
            show_scenario_example_pos(slide_pos);
          }
        }
        break;
      case 5:
        //确定
        //2级界面确认
        if (peg_pos == 1)
        {
          Serial.println("ture into 2");
          if (slide_pos == 0)
          {
            Serial.println("into show description");
            Serial.print("show_description -> slide_pos : ");
            Serial.println(slide_pos);
            Serial.println("");
            clear_all1();
            show_description(slide_pos);
            slide_pos = 0;
            prv_menu_pos = 0;
            peg_pos = 2;
            three_interface_flag = 0;
          }
          else if (slide_pos == 1)
          {
            Serial.println("into show image example");
            clear_all1();
            show_image_example(0);
            prv_menu_pos = 1;
            slide_pos = 0;
            peg_pos = 2;
            three_interface_flag = 1;
          }
          else if (slide_pos == 2)
          {
            Serial.println("into show scenario example");
            clear_all();
            EPD_PartInit();
            EPD_ShowPicture(0, 0, EPD_W, EPD_H, gImage_scenario_home, WHITE);
            EPD_Display(ImageBW);
            EPD_Update();
            EPD_DeepSleep();

            slide_pos = 0;
            prv_menu_pos = 2;
            peg_pos = 2;
            three_interface_flag = 2;
          }
        }
        //3级界面确认
        else if (peg_pos == 2)
        {
          Serial.println("ture into 3");
          if (three_interface_flag == 2) //在灯光控制界面
          {
            if (slide_pos == 0)
            {
              show_scenario_example_ctl(slide_pos);
            }
            else if (slide_pos == 1)
            {
              show_scenario_example_ctl(slide_pos);
            }
            else if (slide_pos == 2)
            {
              show_scenario_example_ctl(slide_pos);
            }
            else if (slide_pos == 3)
            {
              show_scenario_example_ctl(slide_pos);
            }
            else if (slide_pos == 4)
            {
              show_scenario_example_ctl(slide_pos);
            }
          }
        }
        break;
      default:
        break;
    }
  }
  else if (KEY_SCAN() == 0 && test_flag == false)
  {
    delay(10);
    //    Serial.println("sleep_num +++  ....");
    sleep_num++;
    if (sleep_num == 6000)
    {
      EPD_HW_RESET();
      EPD_DeepSleep();

      sleep_num = 0;
      //Go to sleep now
      Serial.println("Going to sleep now ....");
      esp_deep_sleep_start();
    }
  }

  //  delay(60);
}
