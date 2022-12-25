cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(thermometer C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(thermometer
    src/dht22.c
    src/main.c
)

pico_enable_stdio_uart(thermometer 1)
pico_enable_stdio_usb(thermometer 1)

pico_add_extra_outputs(thermometer)

target_compile_options(thermometer PRIVATE -Wall -Wextra -Werror)
target_link_libraries(thermometer pico_stdlib)
