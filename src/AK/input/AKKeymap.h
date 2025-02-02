#ifndef AKKEYMAP_H
#define AKKEYMAP_H

#include <AK/AKObject.h>
#include <xkbcommon/xkbcommon.h>

class AK::AKKeymap : public AKObject
{
public:
    AKKeymap() noexcept;
    const char *keyString(UInt32 code) const noexcept;
    xkb_keysym_t keySymbol(UInt32 code) const noexcept;
    void updateKeyState(UInt32 code, UInt32 state) noexcept;
    void updateModifiers(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group) noexcept;
private:
    xkb_context *m_context { nullptr };
    xkb_keymap *m_keymap { nullptr };
    xkb_state *m_state { nullptr };
};

#endif // AKKEYMAP_H
