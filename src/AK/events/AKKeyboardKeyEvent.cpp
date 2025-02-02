#include <AK/events/AKKeyboardKeyEvent.h>
#include <AK/input/AKKeymap.h>

using namespace AK;

xkb_keysym_t AKKeyboardKeyEvent::keySymbol() const noexcept
{
    return keymap()->keySymbol(keyCode());
}

const char *AKKeyboardKeyEvent::keyString() const noexcept
{
    return keymap()->keyString(keyCode());
}
