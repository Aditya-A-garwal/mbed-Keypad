/**
 * @file                    Keypad.h
 * @author                  Aditya Agarwal (aditya.agarwal@dumblebots.com)
 *
 * @brief                   Simple Library to use a 4x4 Keypad with MBed OS asynchronously
 *
 * @copyright               Copyright (c) 2023
 *
 */


#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#include "mbed.h"
#include "platform/CircularBuffer.h"

/** Maximum number of presses/releases/long-presses to queue up before overwriting */
constexpr auto      KEYPAD_BUFFER_LEN   = 16;
/** Number of rows on a keypad */
constexpr auto      KEYPAD_NUM_ROWS     = 4;
/** Number of columns on a keypad */
constexpr auto      KEYPAD_NUM_COLS     = 4;

/**
 * @brief                   Class that provides a simple interface to use a 4x4 keypad asynchronously
 *
 * @remark                  At a time, only a single button on the keypad can be pressed, pressing multiple buttons
 *                          at the same time will only cause the earliest to be accepted, while all others are rejected
 */
class Keypad {


    /**
     * @brief               Enumeration consisting of all the possible states of the keypad state machine
     *
     * @todo                Add link to state machine diagram in README
     */
    enum ButtonState {

        RELEASED,
        PRESS_BOUNCING,
        PRESSED,
        RELEASE_BOUNCING,
        LONG_PRESSED
    };

    /** Pins connected to the Keypad's rows */
    DigitalOut          row[KEYPAD_NUM_ROWS];
    /** Pins connected to the Keypad's cols */
    InterruptIn         col[KEYPAD_NUM_COLS];

    /** Thread used to execute callback functions */
    Thread              *threadHandle {nullptr};

    /** Event Queue on which calls to the callback functions will be posted */
    EventQueue          queue;

    /** Whether the Callback on a button press is registered or not */
    bool                pressCbEnabled {false};
    /** Callback function that is called when a button is pressed */
    Callback<void(uint32_t, uint32_t)> onPress;

    /** Whether the Callback on a button release is registered or not */
    bool                releaseCbEnabled {false};
    /** Callback function that is called when a button is released */
    Callback<void(uint32_t, uint32_t)> onRelease;

    /** Whether the Callback on a button being long-pressed is registered or not */
    bool                longpressCbEnabled {false};
    /** Callback function that is called when a button is long-pressed */
    Callback<void(uint32_t, uint32_t)> onLongpress;

    /** State of the button in the state machine */
    ButtonState         state {ButtonState::RELEASED};

    /** Timeout to indicate when to switch from PRESS_BOUNCING (button bouncing after being pressed) to PRESSED/RELEASED */
    Timeout             toRowScan;
    /** Timeout to indicate when to switch from PRESSED/LONG_PRESSED to RELEASE_BOUNCING (button bouncing after being released) */
    Timeout             toButtonScan;
    /** Timeout to indicate when to switch from PRESSED to LONG_PRESSED */
    Timeout             toLongPressed;

    /** Confirmed Row on which the button was pressed (after row-scanning) */
    uint32_t            pressedRow {};
    /** Confirmed Column on which the button was pressed (after row-scanning) */
    uint32_t            pressedCol {};

public:

    Keypad() = delete;

    /**
     * @brief               Construct a new keypad object
     *
     * @param r0            Microcontroller Pin to which Row Pin 0 of the keypad is connected
     * @param r1            Microcontroller Pin to which Row Pin 1 of the keypad is connected
     * @param r2            Microcontroller Pin to which Row Pin 2 of the keypad is connected
     * @param r3            Microcontroller Pin to which Row Pin 3 of the keypad is connected
     * @param c0            Microcontroller Pin to which Column Pin 0 of the keypad is connected
     * @param c1            Microcontroller Pin to which Column Pin 1 of the keypad is connected
     * @param c2            Microcontroller Pin to which Column Pin 2 of the keypad is connected
     * @param c3            Microcontroller Pin to which Column Pin 3 of the keypad is connected
     *
     */
    Keypad(PinName r0, PinName r1, PinName r2, PinName r3, PinName c0, PinName c1, PinName c2, PinName c3);

    /**
     * @brief               Initializes the object by allocating and starting a thread to dispatch callbacks on
     *
     * @remark              If the thread was already initialized, then the Keypad::finalize() method must be called
     *                      before trying to re-initialize
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurrently
     *
     * @return              true if the thread was successfully allocated and started, false otherwise
     *
     */
    bool        initialize();

    /**
     * @brief               Finalizes the object by stopping and freeing the thread on which callbacks are dispatched
     *
     * @remark              If the thread was not initialized or finalized before, then the Keypad::initialize()
     *                      method must be called before trying to re-finalize it
     * @remark              Any registered callbacks are preserved when the Keypad::initialize() and Keypad::finalize()
     *                      methods are called, they must be explicitly disabled by calling their respective remove
     *                      methods
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurrently
     *
     * @return              true if the thread was successfully stopped and freed, false otherwise
     *
     */
    bool        finalize();

    /**
     * @brief               Checks if the object was initialized and callbacks can be dispatched correctly
     *
     * @remark              Any registered callbacks are preserved when the Keypad::initialize() and Keypad::finalize()
     *                      methods are called, they must be explicitly disabled by calling their respective remove
     *                      methods
     *
     * @attention           This function can be called from ISR context
     *
     * @return              true if the thread is running, false otherwise
     *
     */
    bool        is_initialized() const;

    /**
     * @brief               Register a callback function to be called whenever a button is pressed
     *
     * @remark              If a callback was already registered, then the current one replaces it
     * @remark              Any registered callbacks are preserved when the Keypad::initialize() and Keypad::finalize()
     *                      methods are called, they must be explicitly disabled by calling their respective remove
     *                      methods
     *
     * @attention           This function can be called from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurrently
     *
     *
     * @param cb            Callback when a button is pressed
     *                      , the first argument to the callback is the row which the pressed button belongs to
     *                      , the second argument to the callback is the column which the pressed button belongs to
     *
     */
    void        register_onpress(Callback<void(uint32_t, uint32_t)> cb);

    /**
     * @brief               Remove the previously registered callback function for when a button is pressed
     *
     * @remark              Has no effect if no callback was registered
     * @remark              Any registered callbacks are preserved when the Keypad::initialize() and Keypad::finalize()
     *                      methods are called, they must be explicitly disabled by calling their respective remove
     *                      methods
     *
     * @attention           This function can be called from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurrently
     *
     */
    void        remove_onpress();

    /**
     * @brief               Checks if a callback function is registered for when a button is pressed
     *
     * @remark              Any registered callbacks are preserved when the Keypad::initialize() and Keypad::finalize()
     *                      methods are called, they must be explicitly disabled by calling their respective remove
     *                      methods
     *
     * @return              true if a callback is currently registered, false otherwise
     *
     */
    bool        is_onpress_registered() const;

    /**
     * @brief               Register a callback function to be called whenever a button is released
     *
     * @remark              If a callback was already registered, then the current one replaces it
     * @remark              Any registered callbacks are preserved when the Keypad::initialize() and Keypad::finalize()
     *                      methods are called, they must be explicitly disabled by calling their respective remove
     *                      methods
     *
     * @attention           This function can be called from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurrently
     *
     * @param cb            Callback when a button is released
     *                      , the first argument to the callback is the row which the released button belongs to
     *                      , the second argument to the callback is the column which the released button belongs to
     *
     */
    void        register_onrelease(Callback<void(uint32_t, uint32_t)> cb);

    /**
     * @brief               Remove the previously registered callback function for when a button is released
     *
     * @remark              Has no effect if no callback was registered
     * @remark              Any registered callbacks are preserved when the Keypad::initialize() and Keypad::finalize()
     *                      methods are called, they must be explicitly disabled by calling their respective remove
     *                      methods
     *
     * @attention           This function can be called from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurrently
     *
     */
    void        remove_onrelease();

    /**
     * @brief               Checks if a callback function is registered for when a button is released
     *
     * @remark              Any registered callbacks are preserved when the Keypad::initialize() and Keypad::finalize()
     *                      methods are called, they must be explicitly disabled by calling their respective remove
     *                      methods
     *
     * @return              true if a callback is currently registered, false otherwise
     *
     */
    bool        is_onrelease_registered() const;

    /**
     * @brief               Register a callback function to be called whenever a button is long-pressed
     *
     * @remark              If a callback was already registered, then the current one replaces it
     * @remark              Any registered callbacks are preserved when the Keypad::initialize() and Keypad::finalize()
     *                      methods are called, they must be explicitly disabled by calling their respective remove
     *                      methods
     *
     * @attention           This function can be called from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurrently
     *
     * @param cb            Callback when a button is long-pressed
     *                      , the first argument to the callback is the row which the long-pressed button belongs to
     *                      , the second argument to the callback is the column which the long-pressed button belongs to
     *
     */
    void        register_onlongpress(Callback<void(uint32_t, uint32_t)> cb);

    /**
     * @brief               Remove the previously registered callback function for when a button was long-pressed
     *
     * @remark              Has no effect if no callback was registered
     * @remark              Any registered callbacks are preserved when the Keypad::initialize() and Keypad::finalize()
     *                      methods are called, they must be explicitly disabled by calling their respective remove
     *                      methods
     *
     * @attention           This function can be called from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurrently
     *
     */
    void        remove_onlongpress();

    /**
     * @brief               Checks if a callback function is registered for when a button is long-pressed
     *
     * @remark              Any registered callbacks are preserved when the Keypad::initialize() and Keypad::finalize()
     *                      methods are called, they must be explicitly disabled by calling their respective remove
     *                      methods
     *
     * @remark              Any registered callbacks are preserved when the Keypad::initialize() and Keypad::finalize()
     *                      methods are called, they must be explicitly disabled by calling their respective remove
     *                      methods
     *
     * @return              true if a callback is currently registered, false otherwise
     *
     */
    bool        is_onlongpress_registered() const;

private:

    /**
     * @brief               Handler for when a fall interrupt is received on a column pin
     *
     * @tparam curCol       The column of the keypad that this handler should cater to
     *
     */
    template <uint32_t curCol>
    void        fall_handler ();

    /**
     * @brief               Handler for a when a rise interrupt is received on a column pin
     *
     * @tparam curCol       The column of the keypad that this handler should cater to
     *
     */
    template <uint32_t curCol>
    void        rise_handler ();

    /**
     * @brief               Scans the rows after a button press on a column was detected to determine its row
     *
     * @tparam curCol       The column of the keypad that this handler should cater to
     *
     */
    template <uint32_t curCol>
    void        row_scan_handler ();

    /**
     * @brief               Scans the previously pressed button after a button release was detected to confirm
     *
     */
    void        button_scan_handler ();

    /**
     * @brief               Transitions a button to the long-pressed state onec it has been held down
     *                      past the threshold duration
     *
     */
    void        long_press_handler ();

    /**
     * @brief           Function for queue to run callbacks on
     *
     * @remark          Calling break_dispatch() on the queue will cause the thread to wait for graceful termination
     *
     */
    void        dispatch_events ();
};

#endif //__KEYPAD_H__
