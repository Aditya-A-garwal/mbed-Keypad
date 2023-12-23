#include "keypad.h"

/** Duration of time a button spends bouncing before stabilizing */
constexpr auto DEBOUNCE_THRESH = 60ms;

/** Duration of time a button must be pressed to be considered long-pressed */
constexpr auto LONG_PRESS_THRESH = 300ms;

// Constructors

Keypad::Keypad(PinName r0, PinName r1, PinName r2, PinName r3, PinName c0, PinName c1, PinName c2, PinName c3)
        : row{{r0}, {r1}, {r2}, {r3}}
        , col{{c0}, {c1}, {c2}, {c3}}
{
    // Use the internal pullup resistors for all the interrupt pins (cols) and switch the rows off
    // whenever a button is pressed, its corresponding column is pulled low, i.e. a fall interrupt happens
    // whenever a button is lifted, its corresponding column is pulled high, i.e. a rise interrupt happens
    // register fall and rise handlers for each of the interrupt pins (cols)

    for (auto &e : col) {
        e.mode(PullUp);
    }

    for (auto &e : row) {
        e = 0;
    }

    static_assert(KEYPAD_NUM_COLS == 4, "Constructor does not initialize all columns!");

    col[0].fall(callback(this, &Keypad::fall_handler<0>));
    col[1].fall(callback(this, &Keypad::fall_handler<1>));
    col[2].fall(callback(this, &Keypad::fall_handler<2>));
    col[3].fall(callback(this, &Keypad::fall_handler<3>));

    col[0].rise(callback(this, &Keypad::rise_handler<0>));
    col[1].rise(callback(this, &Keypad::rise_handler<1>));
    col[2].rise(callback(this, &Keypad::rise_handler<2>));
    col[3].rise(callback(this, &Keypad::rise_handler<3>));
}

// Public Methods

bool
Keypad::initialize() {

    // return if the thread was already initialized; move forward otherwise
    // allocate the thread and return if the allocation fails; move forward otherwise
    // try to start the thread and return if successful
    // in case of failure, delete the allocated thread and return

    if (is_initialized()) {
        return false;
    }

    threadHandle = new (std::nothrow) Thread();
    if (threadHandle == nullptr) {
        return false;
    }

    auto status = threadHandle->start(callback(this, &Keypad::dispatch_events));

    if (status != osOK) {

        delete threadHandle;
        threadHandle = nullptr;

        return false;
    }

    return true;
}

bool
Keypad::finalize() {

    // return if the thread was not initialized; move forward otherwise
    // break the queues dispatch and use shouldTerminate to wait until the action is complete
    // terminate the thread and return if that fails
    // free the thread and return

    if (!is_initialized()) {
        return false;
    }

    queue.break_dispatch();
    threadHandle->join();

    delete threadHandle;
    threadHandle = nullptr;
    return true;
}

bool
Keypad::is_initialized() const {

    // if threadHandle is NULL, then the object was not initialized/finalized before
    // otherwise it is still in the initialized state

    return threadHandle != nullptr;
}

void
Keypad::register_onpress(Callback<void(uint32_t, uint32_t)> cb) {

    onPress = std::move(cb);
    pressCbEnabled = true;
}

void
Keypad::remove_onpress() {
    pressCbEnabled = false;
}

bool
Keypad::is_onpress_registered() const {
    return pressCbEnabled;
}

void
Keypad::register_onrelease(Callback<void(uint32_t, uint32_t)> cb) {

    onRelease = std::move(cb);
    releaseCbEnabled = true;
}

void
Keypad::remove_onrelease() {
    releaseCbEnabled = false;
}

bool
Keypad::is_onrelease_registered() const {
    return releaseCbEnabled;
}

void
Keypad::register_onlongpress(Callback<void(uint32_t, uint32_t)> cb) {

    onLongpress = std::move(cb);
    longpressCbEnabled = true;
}

void
Keypad::remove_onlongpress() {
    longpressCbEnabled = false;
}

bool
Keypad::is_onlongpress_registered() const {
    return longpressCbEnabled;
}

// Private Methods

template<uint32_t curCol>
void
Keypad::fall_handler() {

    if (state != ButtonState::RELEASED) {
        return;
    }

    //transition_state(ButtonState::RELEASED, ButtonState::PRESS_BOUNCING);
    state = ButtonState::PRESS_BOUNCING;
    toRowScan.attach(callback(this, &Keypad::row_scan_handler<curCol>), DEBOUNCE_THRESH);
}

template<uint32_t curCol>
void
Keypad::rise_handler() {

    if (state != ButtonState::PRESSED && state != ButtonState::LONG_PRESSED) {
        return;
    }

    //transition_state(ButtonState::PRESSED, ButtonState::RELEASE_BOUNCING)
    //|| transition_state(ButtonState::LONG_PRESSED, ButtonState::RELEASE_BOUNCING);
    state = ButtonState::RELEASE_BOUNCING;
    toButtonScan.attach(callback(this, &Keypad::button_scan_handler), DEBOUNCE_THRESH);
}

template<uint32_t curCol>
void
Keypad::row_scan_handler() {

    if (state != ButtonState::PRESS_BOUNCING) {
        return;
    }

    for (uint32_t i = 0; i < KEYPAD_NUM_ROWS; ++i) {

        row[i] = 1;
        auto on = col[curCol].read();
        row[i] = 0;

        if (!on) {
            continue;
        }

        //transition_state(ButtonState::PRESS_BOUNCING, ButtonState::PRESSED);
        state = ButtonState::PRESSED;
        pressedRow = i;
        pressedCol = curCol;

        toLongPressed.attach(callback(this, &Keypad::long_press_handler), LONG_PRESS_THRESH);

        if (pressCbEnabled) {
            queue.call([this, i]() { onPress(i, curCol); });
        }

        return;
    }

    //transition_state(ButtonState::PRESS_BOUNCING, ButtonState::RELEASED);
    state = ButtonState::RELEASED;
}

void
Keypad::button_scan_handler() {

    if (state != RELEASE_BOUNCING) {
        return;
    }

    auto curRow = pressedRow;
    auto curCol = pressedCol;

    if (!col[pressedCol].read()) {

        //transition_state(ButtonState::RELEASE_BOUNCING, ButtonState::PRESSED);
        state = ButtonState::PRESSED;
        return;
    }

    //transition_state(ButtonState::RELEASE_BOUNCING, ButtonState::RELEASED);
    state = ButtonState::RELEASED;
    if (toLongPressed.remaining_time().count() > 0) {
        toLongPressed.detach();
    }

    if (releaseCbEnabled) {
        queue.call(
            [this, curRow, curCol]() {
                onRelease(curRow, curCol);
            }
        );
    }
}

void
Keypad::long_press_handler() {

    if (state != ButtonState::PRESSED) {
        return;
    }

    auto curRow = pressedRow;
    auto curCol = pressedCol;

    //transition_state(ButtonState::PRESSED, ButtonState::LONG_PRESSED);
    state = ButtonState::LONG_PRESSED;

    if (longpressCbEnabled) {
        queue.call([this, curRow, curCol]() { onLongpress(curRow, curCol); });
    }
}

void
Keypad::dispatch_events() {

    queue.dispatch_forever();
}
