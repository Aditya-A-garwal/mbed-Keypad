cmake_minimum_required(VERSION 3.16)

project(mbed-Keypad
    DESCRIPTION
        "Set of libraries to use 4x4 Keypads with MBed OS using asynchronous and blocking APIs"
    LANGUAGES
        CXX
)

add_library(mbed-Keypad INTERFACE)

target_include_directories(mbed-Keypad
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(mbed-Keypad
    INTERFACE
        keypad.cpp
        keypadBlocking.cpp
)
