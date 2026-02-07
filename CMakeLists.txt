cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD pico2_w)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_examples C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "2.2.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.2.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

set(BTSTACK_ROOT ${PICO_SDK_PATH}/lib/btstack)

add_executable(foo
    server.c
    ble_server.c
    )
target_include_directories(foo PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(foo
    pico_stdlib
    pico_btstack_ble
    pico_btstack_cyw43
    pico_cyw43_arch_none
    hardware_adc
    )

pico_btstack_make_gatt_header(foo PRIVATE "${CMAKE_CURRENT_LIST_DIR}/foo.gatt")

pico_add_extra_outputs(foo)
