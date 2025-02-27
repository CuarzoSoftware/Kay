#include <AK/events/AKKeyboardKeyEvent.h>
#include <AK/input/AKKeyboard.h>

using namespace AK;

xkb_keysym_t AKKeyboardKeyEvent::keySymbol() const noexcept
{
    return akKeyboard().keySymbol(keyCode());
}

const char *AKKeyboardKeyEvent::keyString() const noexcept
{
    return akKeyboard().keyString(keyCode());
}
