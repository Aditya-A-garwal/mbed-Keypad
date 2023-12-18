#ifndef __KEYPADBLOCKING_H__
#define __KEYPADBLOCKING_H__

#include "keypad.h"

/**
 * @brief                   Class that provides a simple interface to use a 4x4 keypad
 *
 * @remark                  At a time, only a single button on the keypad can be pressed, pressing multiple buttons
 *                          at the same time will only cause the earliest to be accepted, while all others are rejected
 */
class KeypadBlocking {

    /**
     * @brief               Structure to encapsulate the coordinates of a button
     *
     */
    struct button_coord {

        /** Row */
        uint32_t r;
        /** Column */
        uint32_t c;
    };

    /** Keypad instance that is internally used */
    Keypad                                          keypad;

    /** Ordered list of coordinates of buttons that were pressed */
    CircularBuffer<button_coord, KEYPAD_BUFFER_LEN> pressBuf;
    /** Ordered list of coordinates of buttons that were released */
    CircularBuffer<button_coord, KEYPAD_BUFFER_LEN> releaseBuf;
    /** Ordered list of coordinates of buttons that were long-pressed */
    CircularBuffer<button_coord, KEYPAD_BUFFER_LEN> longpressBuf;

    Mutex                                           pressMutex;
    Mutex                                           releaseMutex;
    Mutex                                           longpressMutex;

public:

    /**
     * @brief               Construct a new KeypadBlocking object
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
    KeypadBlocking(PinName r0, PinName r1, PinName r2, PinName r3, PinName c0, PinName c1, PinName c2, PinName c3);

    /**
     * @brief               Initializes the internal keypad object (See Keypad::initialize())
     *
     * @remark              This method must be called before the object can start recording keypad events
     * @remark              If the thread was already initialized, then the KeypadBlocking::finalize() method must be called
     *                      before trying to initialize it again
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurrently
     *
     * @return              true if the object could be initialized, false otherwise
     *
     */
    bool            initialize();

    /**
     * @brief               Finalizes the internal HCSR04 object (see HCSR04::finalize())
     *
     * @remark              After calling this method, the object can not measure keypad events
     * @remark              If the thread was not initialized or previously finalized, then the KeypadBlocking::initialize() method must be called
     *                      before trying to finalize it again
     * @remark              Any unconsumed events (presses, releases, long-presses) are preserved after calling this method
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurrently
     *
     * @return              true if the object could be finalized, false otherwise
     *
     */
    bool            finalize();

    /**
     * @brief               Checks if the object was initialized and callbacks can be dispatched correctly
     *
     * @attention           This function can be called from ISR context
     *
     * @return              true if the thread is running, false otherwise
     *
     */
    bool            is_initialized() const;

    /**
     * @brief               Checks the number of unconsumed button presses
     *
     * @attention           This function can be called from ISR context
     *
     * @return              Number of button presses have not been popped yet
     *
     */
    uint32_t    press_available() const;

    /**
     * @brief               Checks the number of unconsumed button releases
     *
     * @attention           This function can be called from ISR context
     *
     * @return              Number of button releases have not been popped yet
     *
     */
    uint32_t    release_available() const;

    /**
     * @brief               Checks the number of unconsumed button long-presses
     *
     * @attention           This function can be called from ISR context
     *
     * @return              Number of button long-presses have not been popped yet
     *
     */
    uint32_t    longpress_available() const;

    /**
     * @brief               Returns the earliest unconsumed press without removing it from the internal buffer
     *
     * @attention           This function can be called from ISR context
     *
     * @param rptr          Location of variable where the row should be stored (only used if a button press was available)
     * @param cptr          Location of variable where the column should be stored (only used if a button press was available)
     *
     * @return              true if a button press was available to peek, false otherwise
     *
     */
    bool        peek_press(uint32_t *rptr, uint32_t *cptr) const;

    /**
     * @brief               Returns the earliest unconsumed press and removes it from the internal buffer
     *
     * @attention           This function can be called from ISR context
     * @attention           It is not safe to call this method from multiple threads concurrently
     *
     * @param rptr          Location of variable where the row should be stored (only used if a button press was available)
     * @param cptr          Location of variable where the column should be stored (only used if a button press was available)
     *
     * @return              true if a button press was available to pop, false otherwise
     *
     */
    bool        pop_press();

    /**
     * @brief               Returns the earliest unconsumed release without removing it from the internal buffer
     *
     * @attention           This function can be called from ISR context
     *
     * @param rptr          Location of variable where the row should be stored (only used if a button press was available)
     * @param cptr          Location of variable where the column should be stored (only used if a button press was available)
     *
     * @return              true if a button release was available to peek, false otherwise
     *
     */
    bool        peek_release(uint32_t *rptr, uint32_t *cptr) const;

    /**
     * @brief               Returns the earliest unconsumed release and removes it from the internal buffer
     *
     * @attention           This function can be called from ISR context
     * @attention           It is not safe to call this method from multiple threads concurrently
     *
     * @param rptr          Location of variable where the row should be stored (only used if a button press was available)
     * @param cptr          Location of variable where the column should be stored (only used if a button press was available)
     *
     * @return              true if a button release was available to pop, false otherwise
     *
     */
    bool        pop_release();

    /**
     * @brief               Returns the earliest unconsumed long-press without removing it from the internal buffer
     *
     * @attention           This function can be called from ISR context
     *
     * @param rptr          Location of variable where the row should be stored (only used if a button press was available)
     * @param cptr          Location of variable where the column should be stored (only used if a button press was available)
     *
     * @return              true if a long-press was available to peek, false otherwise
     *
     */
    bool        peek_longpress(uint32_t *rptr, uint32_t *cptr) const;

    /**
     * @brief               Returns the earliest unconsumed long-press and removes it from the internal buffer
     *
     * @attention           This function can be called from ISR context
     * @attention           It is not safe to call this method from multiple threads concurrently
     *
     * @param rptr          Location of variable where the row should be stored (only used if a button press was available)
     * @param cptr          Location of variable where the column should be stored (only used if a button press was available)
     *
     * @return              true if a button long-press was available to pop, false otherwise
     *
     */
    bool        pop_longpress();

private:

    /**
     * @brief               Adds presses to the internal buffer (used as a callback by the internal keypad object)
     *
     * @param r             Row of the button that was pressed
     * @param c             Column of the button that was released
     *
     */
    void        push_press(uint32_t r, uint32_t c);

    /**
     * @brief               Adds releases to the internal buffer (used as a callback by the internal keypad object)
     *
     * @param r             Row of the button that was released
     * @param c             Column of the button that was released
     *
     */
    void        push_release(uint32_t r, uint32_t c);

    /**
     * @brief               Adds long-presses to the internal buffer (used as a callback by the internal keypad object)
     *
     * @param r             Row of the button that was long-pressed
     * @param c             Column of the button that was long-pressed
     *
     */
    void        push_longpress(uint32_t r, uint32_t c);
};

#endif //__KEYPADBLOCKING_H__
