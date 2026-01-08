# xiao_nrf52840_sense_peripheral_nus

Demo application for Xiao nRF52840 sense kit.
SDK is based on NRF Connect SDK.
BLE NUS based solution.

## TODOs

- ~~cli porting via USB CDC~~
  - ~~there was an issue due to timeout at gets/fget and so on~~
  - ~~applied console_getline() from nrf package and it fixes the issue~~
- ~~RGB LED via GPIO~~
- ~~Charger LED via GPIO~~
  - ~~using PWM~~
- ~~6-Axis Accelerometer (LSM6DS3TR) via I2C~~
  - ~~used zephyr driver, sensor subsystem~~
  - ~~refer prj.conf~~
  - ~~<https://www.st.com/resource/en/datasheet/lsm6ds3tr-c.pdf>~~
- MIC (MSM261D3526H1CPM) via PDM I/F
  - <https://files.seeedstudio.com/wiki/XIAO-BLE/mic-MSM261D3526H1CPM-ENG.pdf>
- Battery Charger (bq25100)
  - <https://www.ti.com/lit/gpn/bq25100>
- Battery voltage read via ADC
- Expansion Board
  - OLED LCD 0.96" (ZJY-2864KSWPG01) via I2C
    - <https://files.seeedstudio.com/wiki/Grove-OLED-Display-0.96-SSD1315-/res/OEL%20Display%20Module.pdf>
  - ~~RTC (PCF8563) via I2C~~
    - ~~<https://www.nxp.com/docs/en/data-sheet/PCF8563.pdf>~~
  - ~~User button input via GPIO~~
    - ~~app.overlay added to support this~~
  - Buzzer (LET5020CS-03L) via PWM
    - <https://www.micros.com.pl/mediaserver/info-pbsmb-5523sa.pdf>
  - SD Card
  - Grove interface
    - External UART/I2C/GPIOs
- ~~Define BLE protocols~~
  - ~~Tx/Rx feature via BLE, Phone or PC~~
  - ~~Refer NUS_MSG_EN in bsp.h~~
- ~~Simple BLE monitor application~~
  - ~~For Android or Windows~~
  - ~~flutter_xiao_nrf52840_nus_mon~~
- ~~NVS feature~~
  - ~~nvs init/read/write/reset added~~

## History

- 2025.12.22
  - First commit
- 2025.12.29
  - LED control via BLE App from Windows
    - Red/Green/Blue LED control via NUS is work
- 2026.01.02
  - CONFIG_BT_DEVICE_NAME is not applied correctly so modified code to use CONFIG_BT_DEVICE_NAME
  - PC util is updated universal_ble version to v1.0.0
- 2026.01.05
  - BLE link count increased to 4 from 1
  - Prd tick setting cmd added
  - GPIO Key input handler added
- 2026.01.06
  - IMU sensor
  - Zephyr sensor subsystem used
- 2026.01.07
  - RTC Get/Set added
    - works with cli and ble
- 2026.01.08
  - NVS init/read/write/reset added
  - Buzzer pwm added, cli/ble

## Info

- Author : Louiey <louiey.dev@gmail.com>
- HW platform
  - SeeedStudio xiao nRF52840 sense kit + expansion board
- SDK
  - NRF Connect SDK, v3.1.1
  - vscode

## Partition Manager (flash partitions) 🔧

- **Authoritative source:** Partitions are defined by Partition Manager (`pm.yml`) and take precedence over static DTS partitions for this build. ⚠️
- **Where to edit:** Update the appropriate PM source file under `nrf/subsys/partition_manager/` (files named `pm.yml.*`) or add an image-specific `pm.yml` in your application to override.
- **After changes:** Rebuild the project — the generated files `build/partitions.yml`, `build/pm.config`, and `include/generated/pm.yml` reflect the resolved layout.
