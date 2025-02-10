#include <AK/input/AKKeymap.h>
#include <cstdlib>
#include <algorithm>
#include <AK/AKLog.h>
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
    assert("Failed to create XKB keymap" && m_keymap);
    m_state = xkb_state_new(m_keymap);
    assert("Failed to create XKB state" && m_state);
    loadComposeTable();
}

bool AKKeymap::setFromBuffer(const char *buffer, size_t size, xkb_keymap_format format) noexcept
{
    xkb_keymap *newKeymap { xkb_keymap_new_from_buffer(m_context, buffer, size, format, XKB_KEYMAP_COMPILE_NO_FLAGS) };
    if (!newKeymap) return false;

    xkb_state *newState {xkb_state_new(newKeymap) };
    if (!newState)
    {
        xkb_keymap_unref(newKeymap);
        return false;
    }

    xkb_state_unref(m_state);
    xkb_keymap_unref(m_keymap);
    m_keymap = newKeymap;
    m_state = newState;

    if (m_composeState)
        xkb_compose_state_reset(m_composeState);
    return true;
}

bool AKKeymap::setFromString(const char *str, xkb_keymap_format format) noexcept
{
    xkb_keymap *newKeymap { xkb_keymap_new_from_string(m_context, str, format, XKB_KEYMAP_COMPILE_NO_FLAGS) };
    if (!newKeymap) return false;

    xkb_state *newState {xkb_state_new(newKeymap) };
    if (!newState)
    {
        xkb_keymap_unref(newKeymap);
        return false;
    }

    xkb_state_unref(m_state);
    xkb_keymap_unref(m_keymap);
    m_keymap = newKeymap;
    m_state = newState;

    if (m_composeState)
        xkb_compose_state_reset(m_composeState);
    return true;
}

const char *AKKeymap::keyString(UInt32 code) const noexcept
{
    static char buffer[128];

    switch (composeStatus())
    {
    case XKB_COMPOSE_NOTHING:
        AKLog::debug("NOTHING");
        break;
    case XKB_COMPOSE_COMPOSED:
        AKLog::debug("COMPOSED");
        break;
    case XKB_COMPOSE_COMPOSING:
        AKLog::debug("COMPOSING");
        break;
    case XKB_COMPOSE_CANCELLED:
        AKLog::debug("CANCELLED");
        break;
    }

    switch (composeStatus())
    {
    case XKB_COMPOSE_NOTHING:
        xkb_state_key_get_utf8(m_state, code + 8, buffer, sizeof(buffer));
        break;
    case XKB_COMPOSE_COMPOSED:
        xkb_compose_state_get_utf8(m_composeState, buffer, sizeof(buffer));
        xkb_compose_state_reset(m_composeState);
        break;
    case XKB_COMPOSE_COMPOSING:
    case XKB_COMPOSE_CANCELLED:
        return "";
    }

    return buffer;
}

xkb_keysym_t AKKeymap::keySymbol(UInt32 code) const noexcept
{
    return xkb_state_key_get_one_sym(m_state, code + 8);
}

xkb_compose_status AKKeymap::composeStatus() const noexcept
{
    if (!m_composeState)
        return XKB_COMPOSE_NOTHING;

    return xkb_compose_state_get_status(m_composeState);
}

void AKKeymap::updateKeyState(UInt32 code, UInt32 state) noexcept
{
    auto it = std::find(m_pressedKeyCodes.begin(), m_pressedKeyCodes.end(), code);
    const UInt32 isPressed { it != m_pressedKeyCodes.end() };

    if (isPressed == state) return;

    xkb_state_update_key(m_state, code+8, (xkb_key_direction)state);

    if (state == xkb_key_direction::XKB_KEY_DOWN)
    {
        m_pressedKeyCodes.push_back(code);

        if (m_composeState)
        {
            const xkb_keysym_t symbol { keySymbol(code) };
            xkb_compose_state_feed(m_composeState, symbol);
        }
    }
    else
        m_pressedKeyCodes.erase(it);
}

bool AKKeymap::isKeyCodePressed(UInt32 keyCode) const noexcept
{
    return std::find(m_pressedKeyCodes.begin(), m_pressedKeyCodes.end(), keyCode) != m_pressedKeyCodes.end();
}

void AKKeymap::updateModifiers(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group) noexcept
{
    xkb_state_update_mask(m_state, depressed, latched, locked, 0, 0, group);
}

void AKKeymap::loadComposeTable(const char *locale) noexcept
{
    if (m_composeState)
    {
        xkb_compose_state_unref(m_composeState);
        m_composeState = nullptr;
    }

    if (m_composeTable)
    {
        xkb_compose_table_unref(m_composeTable);
        m_composeTable = nullptr;
    }

    if (!locale || !*locale)
        locale = getenv("LC_ALL");
    if (!locale || !*locale)
        locale = getenv("LC_CTYPE");
    if (!locale || !*locale)
        locale = getenv("LANG");
    if (!locale || !*locale)
        locale = "C";

    m_composeTable = xkb_compose_table_new_from_locale(m_context, locale, XKB_COMPOSE_COMPILE_NO_FLAGS);

    if (!m_composeTable)
        goto fail;

    m_composeState = xkb_compose_state_new(m_composeTable, XKB_COMPOSE_STATE_NO_FLAGS);

    if (!m_composeState)
    {
        xkb_compose_table_unref(m_composeTable);
        m_composeTable = nullptr;
        goto fail;
    }
    AKLog::debug("[AKKeymap] Using locale %s.", locale);
    return;
fail:
    AKLog::error("[AKKeymap] Failed to create compose table from locale %s.", locale);

    // Fallback
    if (strcmp(locale, "C") != 0)
        loadComposeTable("C");
}
