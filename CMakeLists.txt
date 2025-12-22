cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(xiao_nrf52840_sense_peripheral_nus)

target_include_directories(app PRIVATE
    src/bsp)

target_sources(app PRIVATE
        src/main.c
        src/getline.c
        src/cli/cli_command.c
        src/cli/cli.c
        src/bsp/bsp.c
        src/bsp/driver/bsp_gpio.c
)
