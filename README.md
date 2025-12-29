# xiao_nrf52840_sense_peripheral_nus

Demo application for Xiao nRF52840 sense kit.
SDK is based on NRF Connect SDK.
BLE NUS based solution.

## TODOs

- ~~cli porting via USB CDC~~
  - ~~there was an issue due to timeout at gets/fget and so on~~
  - ~~applied console_getline() from nrf package and it fixes the issue~~
- ~~RGB LED via GPIO~~
- Charger LED via GPIO
- 6-Axis Accelerometer (LSM6DS3TR) via I2C
  - <https://www.st.com/resource/en/datasheet/lsm6ds3tr-c.pdf>
- MIC (MSM261D3526H1CPM) via PDM I/F
  - <https://files.seeedstudio.com/wiki/XIAO-BLE/mic-MSM261D3526H1CPM-ENG.pdf>
- Battery Charger (bq25100)
  - <https://www.ti.com/lit/gpn/bq25100>
- Battery voltage read via ADC
- Expansion Board
  - OLED LCD 0.96" (ZJY-2864KSWPG01) via I2C
    - <https://files.seeedstudio.com/wiki/Grove-OLED-Display-0.96-SSD1315-/res/OEL%20Display%20Module.pdf>
  - RTC (PCF8563) via I2C
    - <https://www.nxp.com/docs/en/data-sheet/PCF8563.pdf>
  - User button input via GPIO
  - Buzzer (LET5020CS-03L) via PWM
    - <https://www.micros.com.pl/mediaserver/info-pbsmb-5523sa.pdf>
  - SD Card
  - Grove interface
    - External UART/I2C/GPIOs
- Define BLE protocols
  - Tx/Rx feature via BLE, Phone or PC
  - Refer NUS_MSG_EN in bsp.h
- Simple BLE monitor application
  - For Android or Windows
  - flutter_xiao_nrf52840_nus_mon

## History

- 2025.12.22
  - First commit
- 2025.12.29
  - LED control via BLE App from Windows
    - Red/Green/Blue LED control via NUS is work

## Info

- Author : Louiey <louiey.dev@gmail.com>
- HW platform
  - SeeedStudio xiao nRF52840 sense kit + expansion board
- SDK
  - NRF Connect SDK, v3.1.1
  - vscode
