#ifndef CZ_AKBUTTONGROUP_H
#define CZ_AKBUTTONGROUP_H

#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Nodes/AKButton.h>
#include <CZ/Core/CZAdaptive.h>
#include <CZ/Core/CZLinearAnimation.h>
#include <unordered_map>

/**
 * @brief A container that visually groups a row of @ref AKButton "AKButtons".
 *
 * Add buttons by making them children (they are packed side by side, without gaps). On add, the
 * group styles each button (via @ref AKNode::onChildAdded); on removal it reverts the style it
 * applied (via @ref AKNode::onChildRemoved). Only children that are AKButtons are affected.
 *
 * The group can optionally turn its buttons into toggles (see @ref ToggleMode): either independent
 * toggles or a single-selection (radio) set where turning one on turns the others off.
 *
 * The group itself is transparent with small rounded corners, and fades in a subtle fill while the
 * pointer is over it.
 *
 * @ingroup AKNodes
 */
class CZ::AKButtonGroup : public AKRoundSolidColor
{
public:
    enum class ToggleMode
    {
        None,     ///< Buttons are only grouped visually; their toggle behavior is untouched.
        Single,   ///< Buttons are toggles; at most one may be on at a time (radio-like).
        Multiple  ///< Buttons are toggles, each independent of the others.
    };

    enum Changes
    {
        CHToggleMode = AKRoundSolidColor::CHLast,
        CHLast
    };

    AKButtonGroup(AKNode *parent = nullptr) noexcept;

    /**
     * @brief Sets how the grouped buttons behave as toggles (default @ref ToggleMode::None).
     *
     * Applies to the buttons currently in the group and to any added later.
     */
    void setToggleMode(ToggleMode mode) noexcept;
    ToggleMode toggleMode() const noexcept { return m_toggleMode; }

    /**
     * @brief Adaptive fill shown while the pointer is over the group (animated in from transparent).
     *
     * Defaults to @ref AKTheme::SystemFill, the standard subtle state layer.
     */
    void setHoverColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor hoverColor() const noexcept { return m_hoverColor; }

    /**
     * @brief Adaptive tint used to fill the grouped buttons.
     *
     * Each button's background is this color at a state-dependent opacity: transparent at rest,
     * a subtle tint on hover or while toggled on, and a stronger tint while pressed (or toggled and
     * hovered). Defaults to an adaptive ink (dark in light schemes, light in dark schemes).
     */
    void setButtonColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor buttonColor() const noexcept { return m_buttonColor; }

    /** @brief Sets the group's corner radius and keeps the grouped buttons' radius equal to it. */
    void setBorderRadius(const CZBorderRadius &borderRadius) noexcept;

protected:
    // The inherited setter is not part of the group's public API — it wouldn't keep the buttons in
    // sync. The group's own setBorderRadius above hides it; this keeps it reachable internally.
    using AKRoundSolidColor::setBorderRadius;

    void onSceneBegin() override;
    void pointerEnterEvent(const CZPointerEnterEvent &event) override;
    void pointerLeaveEvent(const CZPointerLeaveEvent &event) override;
    void windowStateEvent(const CZWindowStateEvent &event) override;

    // Child styling (only AKButton children; verified via dynamic_cast in the signal handlers).
    void styleButton(AKButton *button) noexcept;
    void unstyleButton(AKButton *button) noexcept;
    // Pushes the six per-state background colors derived from m_buttonColor onto a member button.
    void applyButtonColors(AKButton *button) const noexcept;

    // Group hover-fill fade (short, interruptible; restarts from the current color on interruption).
    void setHovered(bool hovered) noexcept;
    void startHoverTransition() noexcept;
    void applyAnimatedHover() noexcept;

    ToggleMode m_toggleMode { ToggleMode::None };
    CZAdaptiveColor m_hoverColor;
    // Adaptive ink used for the button fills: dark in light schemes, light in dark schemes.
    CZAdaptiveColor m_buttonColor { CZAdaptiveColor(0xFF000000, 0xFFFFFFFF) };
    bool m_hovered { false };

    CZLinearAnimation m_hoverAnim;
    SkColor m_hoverFrom { SK_ColorTRANSPARENT };
    SkColor m_hoverTo { SK_ColorTRANSPARENT };
    SkColor m_hoverTarget { SK_ColorTRANSPARENT };

    // The button properties the group overrode, restored verbatim on removal.
    struct ButtonStyle
    {
        bool toggleable;
        CZBorderRadius borderRadius;
        bool autoContentColor;
        CZAdaptiveColor contentColor;
        bool autoBackgroundColor;
        CZAdaptiveColor backgroundColor;
        CZAdaptiveColor hoverBackgroundColor;
        CZAdaptiveColor pressedBackgroundColor;
        CZAdaptiveColor offToggleBackgroundColor;
        CZAdaptiveColor offToggleHoverBackgroundColor;
        CZAdaptiveColor offTogglePressedBackgroundColor;
    };
    std::unordered_map<AKButton*, ButtonStyle> m_prevStyle;
};

#endif // CZ_AKBUTTONGROUP_H
