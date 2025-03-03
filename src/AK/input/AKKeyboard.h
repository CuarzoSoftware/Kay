#ifndef AKKEYMAP_H
#define AKKEYMAP_H

#include <AK/AKObject.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>

class AK::AKKeyboard : public AKObject
{
public:
    bool setFromBuffer(const char *buffer, size_t size, xkb_keymap_format format) noexcept;
    bool setFromString(const char *str, xkb_keymap_format format) noexcept;

    const char *keyString(UInt32 code) const noexcept;
    xkb_keysym_t keySymbol(UInt32 code) const noexcept;
    xkb_compose_status composeStatus() const noexcept;

    void updateKeyState(UInt32 code, UInt32 state) noexcept;
    const std::vector<UInt32> &pressedKeyCodes() const noexcept { return m_pressedKeyCodes; };
    bool isKeyCodePressed(UInt32 keyCode) const noexcept;
    void updateModifiers(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group) noexcept;

    void setKeyRepeatInfo(Int32 delayMs, Int32 rateMs) noexcept
    {
        m_keyRepeatDelayMs = delayMs;
        m_keyRepeatRateMs = rateMs;
    }
    Int32 keyRepeatRateMs() const noexcept { return m_keyRepeatRateMs; }
    Int32 keyRepeatDelayMs() const noexcept { return m_keyRepeatDelayMs; }

protected:
    AKKeyboard() noexcept;
    AKCLASS_NO_COPY(AKKeyboard)

private:
    friend class AKApplication;
    xkb_context *m_context { nullptr };
    xkb_keymap *m_keymap { nullptr };
    xkb_state *m_state { nullptr };
    xkb_compose_table *m_composeTable { nullptr };
    xkb_compose_state *m_composeState { nullptr };
    Int32 m_keyRepeatRateMs { 32 };
    Int32 m_keyRepeatDelayMs { 500 };
    std::vector<UInt32> m_pressedKeyCodes;
    void loadComposeTable(const char *locale = nullptr) noexcept;
};

#endif // AKKEYMAP_H
