# mbed-Keypad

![GitHub License](https://img.shields.io/github/license/Aditya-A-garwal/mbed-Keypad)
![GitHub forks](https://img.shields.io/github/forks/Aditya-A-garwal/mbed-Keypad?style=flat-square&color=blue)
![GitHub Repo stars](https://img.shields.io/github/stars/Aditya-A-garwal/mbed-Keypad?style=flat-square&color=blue)
![GitHub issues](https://img.shields.io/github/issues-raw/Aditya-A-garwal/mbed-Keypad?style=flat-square&color=indianred)
![GitHub closed issues](https://img.shields.io/github/issues-closed-raw/Aditya-A-garwal/mbed-Keypad?style=flat-square)
![GitHub pull requests](https://img.shields.io/github/issues-pr/Aditya-A-garwal/mbed-Keypad?style=flat-square&color=indianred)
![GitHub closed pull requests](https://img.shields.io/github/issues-pr-closed/Aditya-A-garwal/mbed-Keypad?style=flat-square)

## Overview

This repository contains a simple library to use a *4x4 Keypad* with MBed OS. The library **automatically debounces** the button presses and **can distinguish between short and long presses**. The library's design allows only recognizes a single button being pressed at a time. Pressing multiple buttons while a single one is held down will cause the latter pressed to be rejected. A state-machine diagram is included in this README to explain this.

The library provides *blocking* & *non-blocking* APIs, and allows different keypads connected to the same MCU to use different techniques at the same time. The dimensions of they keypad can also be increased or reduced as required. A more detailed explanation is found in the next sections.

## Usage

The process to use the library in your own *MBed CLI/MBed CE* projects consists of a few simple steps. This section assumes that CMake is being used as the build system. For other build systems/IDEs, use the source and header files directly and refer to their manuals.

The steps to use the library in your own projects are shown below -

1. Create a file named ```external/CMakeLists.txt``` in your project root with the following content. This will fetch the repository and add its targets to the project.

    ```diff
    FetchContent_Declare(mbed-Keypad
        GIT_REPOSITORY
            https://github.com/Aditya-A-garwal/mbed-Keypad.git
        GIT_TAG
            latest
    )

    FetchContent_MakeAvailable(mbed-Keypad)
    ```

    ```latest``` after ```GIT_TAG``` uses the latest commit to the main branch of this repository. Replace it with the hash of the commit or the tag of the release that needs to be used in your project.

    More information about the FetchContent set of commands can be found [in the official CMake Docs](https://cmake.org/cmake/help/latest/module/FetchContent.html).

2. Add the following line to to the ```CMakeLists.txt``` in your project root. This will add the ```external``` directory to your project. Make sure to insert it before creating the ```APP_TARGET``` executable and after including the MBed OS directory.

    ```diff
    add_subdirectory(${MBED_PATH})

    + add_subdirectory(external)

    add_executable(${APP_TARGET}
        main.cpp
    )
    ```

3. Link the library with APP_TARGET (or other targets as required) by adding updating the following line -

    ```diff
    - target_link_libraries(${APP_TARGET} mbed-os)
    + target_link_libraries(${APP_TARGET} mbed-os mbed-Keypad)
    ```

    This also updates the include path for the linked targets.

4. Configure and build the project by running the following commands in the root-directory of the project. This fetches the repository and makes the code available to the intellisense and autocomplete features in most IDEs.

    ```bash
    # Configure (fetches the repository and prepares the build-system)
    mbed-tools configure --toolchain <TOOLCHAIN> --mbed-target <TARGET_NAME> --profile <PROFILE>
    cmake -S . -B cmake_build/<TARGET_NAME>/<PROFILE>/<TOOLCHAIN>

    # Builds the code
    cmake --build cmake_build/<TARGET_NAME>/<PROFILE>/<TOOLCHAIN>
    ```

    Make sure to replace ```<TOOLCHAIN>```, ```<TARGET_NAME>``` and ```<PROFILE>``` with the appropriate
    values.

    **To change the version of the library being used, update the ```GIT_TAG``` parameter in ```external/CMakeLists.txt``` and re-run the configure and build commands. Re-building the project is not enough as the ```FetchContent_Declare``` command fetches the library while configuring the project only, and not while building.**

5. Include the header files in ```main.cpp``` (and other files as required) -

    ```cpp
    #include "keypad.h" // non-blocking APIs
    #include "keypadBlocking.h" // blocking APIs
    ```

## Organization of the Library

The library consists of two header files - ```keypad.h``` for non-blocking APIs and ```keypadBlocking.h``` for blocking APIs, each containing a single class. The classes are implemented in two source files - ```keypad.cpp``` and ```keypadBlocking.cpp```.

The steps to use the Blocking APIs are as follows -

1. Instantiate the ```KeypadBlocking``` class.
2. Initialize the object by calling the ```initialize()``` method.
3. Get the number of unread events using the ```press_available()```, ```release_available()``` and ```longpress_available()``` methods. **These return immediately the number of events.**
4. Peek the coordinates of each event using the ```peek_press(*r, *c)```, ```peek_release(*r, *c)``` and ```peek_longpress(*r, *c)``` methods. Events are processed in FIFO-order. **These calls return immediately, and return false if the requested event has not occured since the last call to its respective *pop* method.**
5. Pop each event using the ```pop_press(*r, *c)```, ```pop_release(*r, *c)``` and ```pop_longpress(*r, *c)``` methods. Events are processed in the same order as the peek methods. **These calls return immediately, and return false if the requested event has not occured since the last call to its respective *pop* method.**
6. Finalize the object by calling the ```finalize()``` method. **Missing this step before the destructor is called will cause memory-leaks and zombie-threads.**
7. The object is destructed.

The steps to use the Non-Blocking APIs are as follows -

1. Instantiate the ```Keypad``` class.
2. Initialize the object by calling the ```initialize()``` method.
3. Register callbacks for each type of event using the ```register_onpress(cb)```, ```register_onrelease(cb)``` and ```register_onlongpress(cb)``` methods. **These methods register the callback and return immediately. If any of the above events occur, the callback is called in a separate thread and the event processed.**
4. Finalize the object by calling the ```finalize()``` method. **Missing this step before the destructor is called will cause memory-leaks and zombie-threads.**
5. The object is destructed.

The library defines the following constants, whose values can be altered to change its behaviour -

1. Number of rows on a keypad (present in ```keypad.h```)
    ```cpp
    /** Number of rows on a keypad */
    constexpr auto      KEYPAD_NUM_ROWS     = 4;
    ```
2. Number of columns on a keypad (presnet in ```keypad.h```)
    ```cpp
    /** Number of columns on a keypad */
    constexpr auto      KEYPAD_NUM_COLS     = 4;
    ```
3. Duration of time between a button bounces for while transitioning between pressed and released states (found in ```keypad.cpp```)
    ```cpp
    /** Duration of time a button spends bouncing before stabilizing */
    constexpr auto DEBOUNCE_THRESH = 60ms;
    ```
4. Duration of time a button needs to be held-down before the press is considered as a long-press (found in ```keypad.cpp```)
    ```cpp
    /** Duration of time a button must be pressed to be considered long-pressed */
    constexpr auto LONG_PRESS_THRESH = 300ms;
    ```
5. Number of events of each type (press/release/longpress) to queue up per keypad in blocking mode before old values are overwritten (found in ```keypad.h```)
    ```cpp
    /** Maximum number of presses/releases/long-presses to queue up before overwriting */
    constexpr auto      KEYPAD_BUFFER_LEN   = 16;
    ```

Detailed information is available as inline documentation within the header files.

## Design of the Library

The library recognizes three distinct events on the keypad -

|Event|Meaning|
|-|-|
| Button Release | A button that was pressed just got released (all buttons were previously not held-down except one) |
| Button Press | A button just got pressed (all buttons were previously not held-down) |
| Button Longpress | A button that was previously pressed has just been held down beyond a threshold point. |

A state-machine is used to manage these events along with debouncing and callbacks/

![State Machine Diagram](/state_machine.png)

The state machine has the following states -

|State|Meaning|
|-|-|
| ```RELEASED``` | All buttons are released |
| ```PRESSED_BOUNCING``` | A button was pressed and is now bouncing |
| ```PRESSED``` | A button that was previously bouncing is confirmed to be held-down |
| ```LONG_PRESSED``` | A button that was held-down has been held-down for beyond a threshold |
| ```RELEASED_BOUNCING``` | A button that was held-down has been released and is now bouncing |

Because of the design of the state-machine, only a single button maybe pressed at a time. Pressing multiple buttons at the same will cause only the first one to be accepted (the keypad will not be in the ```RELEASED``` state after the first button is pressed).

A separate thread is used by the Keypad object for servicing callbacks, which is spawned in the ```initialize()``` method and joined in the ```finalize()``` method. **This might be an important consideration for many applications.**

Multiple keypad objects maybe declared at the same time for different keypads connected to the microcontroller, but each keypad will spawn its own thread to run callbacks.

## Documentation

The ```.h``` header files contain inline documentation for all classes, structs, functions and enums within it. This repository uses the Doxygen standard for inline-documentation. Regular comments explaining implementation details can be found in the ```.cpp``` source files.
