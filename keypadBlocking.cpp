#include "keypadBlocking.h"

// Constructors

KeypadBlocking::KeypadBlocking(PinName r0, PinName r1, PinName r2, PinName r3, PinName c0, PinName c1, PinName c2,
                               PinName c3)
        : keypad(r0, r1, r2, r3, c0, c1, c2, c3)
{
    // register the callbacks that push keypad events from the internal object to the internal buffers

    keypad.register_onpress(callback(this, &KeypadBlocking::push_press));
    keypad.register_onrelease(callback(this, &KeypadBlocking::push_release));
    keypad.register_onlongpress(callback(this, &KeypadBlocking::push_longpress));
}

// Public Methods

bool
KeypadBlocking::initialize() {
    return keypad.initialize();
}

bool
KeypadBlocking::finalize() {
    return keypad.finalize();
}

bool
KeypadBlocking::is_initialized() const {
    return keypad.is_initialized();
}

uint32_t
KeypadBlocking::press_available() const {
    return pressBuf.size();
}

uint32_t
KeypadBlocking::release_available() const {
    return releaseBuf.size();
}

uint32_t
KeypadBlocking::longpress_available() const {
    return longpressBuf.size();
}

bool
KeypadBlocking::peek_press(uint32_t *rptr, uint32_t *cptr) const {

    // if the buffer is empty, return false, store the coordinates to the supplied locations otherwise

    struct button_coord cur {};

    if (!pressBuf.peek(cur)) {
        return false;
    }

    *rptr = cur.r;
    *cptr = cur.c;
    return true;
}

bool
KeypadBlocking::pop_press() {

    // if the buffer is empty, return false, store the coordinates to the supplied locations otherwise

    struct button_coord cur {};

    if (!pressBuf.pop(cur)) {
        return false;
    }

    //*rptr = cur.r;
    //*cptr = cur.c;
    return true;
}

bool
KeypadBlocking::peek_release(uint32_t *rptr, uint32_t *cptr) const {

    // if the buffer is empty, return false, store the coordinates to the supplied locations otherwise

    struct button_coord cur {};

    if (!releaseBuf.peek(cur)) {
        return false;
    }

    *rptr = cur.r;
    *cptr = cur.c;
    return true;
}

bool
KeypadBlocking::pop_release() {

    // if the buffer is empty, return false, store the coordinates to the supplied locations otherwise

    struct button_coord cur {};

    if (!releaseBuf.pop(cur)) {
        return false;
    }

    //*rptr = cur.r;
    //*cptr = cur.c;
    return true;
}

bool
KeypadBlocking::peek_longpress(uint32_t *rptr, uint32_t *cptr) const {

    // if the buffer is empty, return false, store the coordinates to the supplied locations otherwise

    struct button_coord cur {};

    if (!longpressBuf.peek(cur)) {
        return false;
    }

    *rptr = cur.r;
    *cptr = cur.c;
    return true;
}

bool
KeypadBlocking::pop_longpress() {

    // if the buffer is empty, return false, store the coordinates to the supplied locations otherwise

    struct button_coord cur {};

    if (!longpressBuf.pop(cur)) {
        return false;
    }

    //*rptr = cur.r;
    //*cptr = cur.c;
    return true;
}

// Private Methods

void
KeypadBlocking::push_press(uint32_t r, uint32_t c) {
    pressBuf.push({r, c});
}

void
KeypadBlocking::push_release(uint32_t r, uint32_t c) {
    releaseBuf.push({r, c});
}

void
KeypadBlocking::push_longpress(uint32_t r, uint32_t c) {
    longpressBuf.push({r, c});
}
