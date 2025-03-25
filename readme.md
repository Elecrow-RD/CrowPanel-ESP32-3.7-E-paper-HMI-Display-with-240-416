### 1, Product picture

![esp32_3.7inch_e-paper_display](./esp32_3.7inch_e-paper_display.png)

### 2, Product version number

|      | Hardware | Software | Remark |
| ---- | -------- | -------- | ------ |
| 1    | V1.0     | V1.0     | latest |

### 3, product information

- # Display Module Specifications

  - # Display Module Specifications

    - **Size**: 3.7 inch

    - **MCU**: ESP32-S3-WROOM-1-N8R8, up to 240 MHz

    - **Flash**: 8 MB

    - **PSRAM**: 8 MB

    - **Material**: Active Matrix Electrophoretic Display (AM EPD)

    - **Driver Chip**: UC8253

    - **Resolution**: 240(H)*416(V) Pixel

    - **Pixel Pitch**: 0.196*0.196

    - **Viewing Angle**: Full Viewing Angle

    - **Communication Interface**: 3-/4-wire SPI, default 4-wire SPI
  
    - **Interface**:
      
      - UART0 x1
      - BAT x1
      - GPIO x1
    
      - TF Card x1
    - **Button**:
      - Dial Switch x1
      - Menu Button  x1
      - Back Button x1
      - REST Button  x1
  
      - BOOT Button x1
- **Development Environment**: Arduino IDE、ESP IDF、MicroPython
    
- **Display Color**: Black and white
    
- **Refresh Mode**: Partial refresh (saves more power)
    
- **Display Operation Voltage**: 2.2~3.7V
    
- **Case**: White Acrylic Case
    
  - **Operation Temperature**: -0~50℃
    
  - **Storage Temperature**: -25~70℃
    
    - **Active Area**: 47.04(H)\*81.54(L) (H*L)

### 4, Use the driver module

| Name   | dependency library     |
| ------ | ---------------------- |
| UC8253 | EPD<br />version=1.0.0 |

### 5,Quick Start

##### Arduino IDE starts

1.Download the library files used by this product to the 'libraries' folder.

C:\Users\Documents\Arduino\libraries\

![2](https://github.com/user-attachments/assets/86c568bb-3921-4a07-ae91-62d7ce752e50)



2.Open the Arduino IDE

![Snipaste_2025-03-24_18-24-00](./Snipaste_2025-03-24_18-24-00.jpg)

3.Open the code configuration environment and burn it

![setting](F:\wiki\CrowPanel E-paper\CrowPanel 4.2 E-paper\setting.png)

### 6,Folder structure.

|--3D file： Contains 3D model files (.stp) for the hardware. These files can be used for visualization, enclosure design, or integration into CAD software.

|--Datasheet: Includes datasheets for components used in the project, providing detailed specifications, electrical characteristics, and pin configurations.

|--Eagle_SCH&PCB: Contains **Eagle CAD** schematic (`.sch`) and PCB layout (`.brd`) files. These are used for circuit design and PCB manufacturing.

|--example: Provides example code and projects to demonstrate how to use the hardware and libraries. These examples help users get started quickly.

|--factory_firmware: Stores pre-compiled factory firmware that can be directly flashed onto the device. This ensures the device runs the default functionality.

|--factory_sourcecode: Contains the source code for the factory firmware, allowing users to modify and rebuild the firmware as needed.

### 7,Pin definition

#define SCK 12

#define MOSI 11

#define RES 47

#define DC 46

#define CS 45

#define BUSY 48