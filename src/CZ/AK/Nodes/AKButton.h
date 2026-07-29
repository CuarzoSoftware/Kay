#ifndef CZ_AKBUTTON_H
#define CZ_AKBUTTON_H

#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Nodes/AKText.h>
#include <CZ/AK/Nodes/AKFontIcon.h>
#include <CZ/Core/CZAdaptive.h>
#include <CZ/Core/CZLinearAnimation.h>
#include <CZ/skia/core/SkFontStyle.h>

/**
 * @brief A solid, borderless button.
 *
 * The fill and content colors default to the theme's control accent / white; hover and pressed
 * are rendered as a translucent state layer over the fill, and disabled reduces the node opacity.
 * Size and font are plain layout() properties, so customize them directly.
 *
 * @ingroup AKNodes
 */
class CZ::AKButton : public AKRoundSolidColor
{
public:
    enum Changes
    {
        CHEnabled = AKRoundSolidColor::CHLast,
        CHPressed,
        CHToggled,
        CHLast
    };

    AKButton(const std::string &text, AKNode *parent = nullptr) noexcept;
    AKButton(const std::string &text, const std::string &icon, AKNode *parent = nullptr) noexcept;

    AKText &textNode() noexcept { return m_text; }
    AKFontIcon &icon() noexcept { return m_icon; }

    /**
     * @brief Idle fill while the button is ON (toggled on, or simply a non-toggle button).
     *
     * The background has one color per interaction state (idle / hover / pressed), split by the
     * toggle state (this "on" set and the offToggle* "off" set). While automatic background color is
     * enabled (default, @ref setAutoBackgroundColor), only the two idle colors are used and the
     * hover/pressed colors are derived from them by blending a translucent state layer; when
     * disabled, all six are used verbatim.
     *
     * Accepts a @ref CZAdaptiveColor (distinct light/dark) or a single SkColor (implicitly
     * converted, used for both). Defaults to @ref AKTheme::ControlAccent, the macOS control tint.
     */
    void setBackgroundColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor backgroundColor() const noexcept { return m_backgroundColor; }

    /** @brief Fill while ON and hovered. Used only while automatic background color is disabled. */
    void setHoverBackgroundColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor hoverBackgroundColor() const noexcept { return m_hoverBackgroundColor; }

    /** @brief Fill while ON and pressed. Used only while automatic background color is disabled. */
    void setPressedBackgroundColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor pressedBackgroundColor() const noexcept { return m_pressedBackgroundColor; }

    /**
     * @brief Idle fill while the button is a toggle and currently OFF (see setToggleable()).
     *
     * Ignored for non-toggle buttons. Defaults to @ref AKTheme::SystemFill.
     */
    void setOffToggleBackgroundColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor offToggleBackgroundColor() const noexcept { return m_offToggleBackgroundColor; }

    /** @brief Fill while OFF and hovered. Used only while automatic background color is disabled. */
    void setOffToggleHoverBackgroundColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor offToggleHoverBackgroundColor() const noexcept { return m_offToggleHoverBackgroundColor; }

    /** @brief Fill while OFF and pressed. Used only while automatic background color is disabled. */
    void setOffTogglePressedBackgroundColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor offTogglePressedBackgroundColor() const noexcept { return m_offTogglePressedBackgroundColor; }

    /**
     * @brief When enabled (default), the hover/pressed fills are derived from the idle fill by
     *        blending a translucent state layer over it. When disabled, the explicit per-state
     *        colors above are used as-is (this is what @ref AKButtonGroup does for its buttons).
     */
    void setAutoBackgroundColor(bool enabled) noexcept;
    bool hasAutoBackgroundColor() const noexcept { return m_backgroundColorAuto; }

    /**
     * @brief Label/icon color, as a @ref CZAdaptiveColor or a single SkColor (used for both).
     *
     * Ignored while automatic content color is enabled (@ref setAutoContentColor). Defaults to
     * @ref AKTheme::AlternateSelectedControlText (white), the color for text over a tinted fill.
     */
    void setContentColor(CZAdaptiveColor color) noexcept;

    /** @brief The adaptive content color as set (overridden while automatic mode is on). */
    CZAdaptiveColor contentColor() const noexcept { return m_contentColor; }

    /**
     * @brief When enabled (default), the content color is derived from the fill's lightness:
     *        @ref AKTheme::AlternateSelectedControlText (white) over dark fills, and
     *        @ref AKTheme::Label (the primary ink) over light fills. The set contentColor is ignored.
     */
    void setAutoContentColor(bool enabled) noexcept;
    bool hasAutoContentColor() const noexcept { return m_contentColorAuto; }

    void setEnabled(bool enabled) noexcept;
    bool enabled() const noexcept { return m_enabled; }

    void setPressed(bool pressed) noexcept;
    bool pressed() const noexcept { return m_pressed; };

    /**
     * @brief Makes the button behave as a toggle: each click flips toggled() and emits onToggled,
     *        and the base fill follows the state (backgroundColor() on, offToggleBackgroundColor() off).
     */
    void setToggleable(bool toggleable) noexcept;
    bool toggleable() const noexcept { return m_toggleable; }

    /** @brief The toggle state. Changing it (by click or programmatically) emits onToggled. */
    void setToggled(bool toggled) noexcept;
    bool toggled() const noexcept { return m_toggled; }

    void setHovered(bool hover) noexcept;
    bool hovered() const noexcept { return m_hovered; }

    CZSignal<const CZPointerButtonEvent&> onClick;

    /** @brief Emitted when the toggle state changes; carries the new state. */
    CZSignal<bool> onToggled;

protected:
    void init() noexcept;
    void pointerButtonEvent(const CZPointerButtonEvent &event) override;
    void pointerEnterEvent(const CZPointerEnterEvent &event) override;
    void pointerLeaveEvent(const CZPointerLeaveEvent &event) override;
    void windowStateEvent(const CZWindowStateEvent &event) override;
    void colorSchemeEvent(const CZColorSchemeEvent &event) override;
    void updateStyleColor() noexcept;
    // Applies fill/content colors to the nodes directly (no animation).
    void applyColors(SkColor background, SkColor content) noexcept;
    // Applies the eased, interpolated colors for the current animation value.
    void applyAnimatedColors() noexcept;
    // Starts a transition towards the recorded target colors if they differ from the displayed ones.
    void startColorTransition() noexcept;

    CZAdaptiveColor m_backgroundColor;
    CZAdaptiveColor m_hoverBackgroundColor;
    CZAdaptiveColor m_pressedBackgroundColor;
    CZAdaptiveColor m_offToggleBackgroundColor;
    CZAdaptiveColor m_offToggleHoverBackgroundColor;
    CZAdaptiveColor m_offTogglePressedBackgroundColor;
    CZAdaptiveColor m_contentColor;
    bool m_backgroundColorAuto { true };
    bool m_contentColorAuto { true };
    AKFontIcon m_icon;
    AKText m_text;
    bool m_enabled { true };
    bool m_pressed { false };
    bool m_hovered { false };
    bool m_toggleable { false };
    bool m_toggled { false };

    // Short, non-interruptible color transition. updateStyleColor() records the desired colors
    // (m_*Target); an in-flight animation is always allowed to finish, and on finish it restarts
    // towards the latest target if it changed meanwhile. Displayed = Mix(*To, *From, ease(value)),
    // and at rest the displayed color equals *To.
    CZLinearAnimation m_colorAnim;
    SkColor m_bgFrom { SK_ColorTRANSPARENT }, m_bgTo { SK_ColorTRANSPARENT }, m_bgTarget { SK_ColorTRANSPARENT };
    SkColor m_contentFrom { SK_ColorTRANSPARENT }, m_contentTo { SK_ColorTRANSPARENT }, m_contentTarget { SK_ColorTRANSPARENT };
    bool m_colorsSeeded { false };
};

#endif // CZ_AKBUTTON_H
