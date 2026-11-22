#ifndef CZ_AKSWITCH_H
#define CZ_AKSWITCH_H

#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/Core/CZAdaptive.h>
#include <CZ/Core/CZSignal.h>
#include <CZ/Core/CZLinearAnimation.h>

/**
 * @brief An on/off switch, like macOS's.
 * @ingroup AKNodes
 *
 * A pill (the accent color while on, gray while off) with a white knob that slides to the side
 * it's on. Clicking it toggles it (onToggled tells the new state); setOn() changes it from code
 * (without emitting onToggled).
 */
class CZ::AKSwitch : public AKRoundSolidColor
{
public:
    /** @brief Creates the switch, on or off. */
    AKSwitch(bool on = false, AKNode *parent = nullptr) noexcept;

    static constexpr SkScalar Width  { 38.f };
    static constexpr SkScalar Height { 22.f };
    static constexpr UInt32 AnimationMs { 160 };

    /** @brief Turns it on or off (animated by default). Doesn't emit onToggled. */
    void setOn(bool on, bool animated = true) noexcept;
    bool on() const noexcept { return m_on; }

    /** @brief Disabled switches are faded and ignore clicks. */
    void setEnabled(bool enabled) noexcept;
    bool enabled() const noexcept { return m_enabled; }

    /** @brief Color of the pill while on (the accent color by default). */
    void setOnColor(CZAdaptiveColor color) noexcept;

    /** @brief The user clicked it: its new state. */
    CZSignal<bool> onToggled;

    AKRoundSolidColor knob { SK_ColorWHITE, CZBorderRadius::Make(0), this };

protected:
    void pointerButtonEvent(const CZPointerButtonEvent &e) override;
    void pointerLeaveEvent(const CZPointerLeaveEvent &e) override;
    void colorSchemeEvent(const CZColorSchemeEvent &e) override;
    /// @cond INTERNAL
    void setProgress(SkScalar progress) noexcept;
    void updateColors() noexcept;
    CZAdaptiveColor m_onColor;
    CZLinearAnimation m_anim;
    SkScalar m_progress { 0.f }, m_from { 0.f }; // 0 = off, 1 = on
    bool m_on { false }, m_enabled { true }, m_pressed { false };
    /// @endcond
};

#endif // CZ_AKSWITCH_H
