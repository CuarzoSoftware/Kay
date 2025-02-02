#include <AK/input/AKKeymap.h>
#include <cstdlib>
#include <cstring>

using namespace AK;

AKKeymap::AKKeymap() noexcept
{
    const xkb_rule_names names {
        getenv("XKB_DEFAULT_RULES"),
        getenv("XKB_DEFAULT_MODEL"),
        getenv("XKB_DEFAULT_LAYOUT"),
        getenv("XKB_DEFAULT_VARIANT"),
        getenv("XKB_DEFAULT_OPTIONS") };
    m_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    assert("Failed to create XKB context" && m_context);
    m_keymap = xkb_keymap_new_from_names(m_context, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
    assert("Failed to create XKB keymap" && m_context);
    m_state = xkb_state_new(m_keymap);
    assert("Failed to create XKB state" && m_context);
}

const char *AKKeymap::keyString(UInt32 code) const noexcept
{
    static char buffer[128];

    switch (keySymbol(code))
    {
    case XKB_KEY_space:
        strcpy(buffer, " ");
        break;
    case XKB_KEY_Tab:
        strcpy(buffer, "\t");
        break;
    default:
        xkb_state_key_get_utf8(m_state, code + 8, buffer, sizeof(buffer));
        break;
    }

    return buffer;
}

xkb_keysym_t AKKeymap::keySymbol(UInt32 code) const noexcept
{
    return xkb_state_key_get_one_sym(m_state, code + 8);
}

void AKKeymap::updateKeyState(UInt32 code, UInt32 state) noexcept
{
    xkb_state_update_key(m_state, code+8, (xkb_key_direction)state);
}

void AKKeymap::updateModifiers(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group) noexcept
{
    xkb_state_update_mask(m_state, depressed, latched, locked, 0, 0, group);
}
