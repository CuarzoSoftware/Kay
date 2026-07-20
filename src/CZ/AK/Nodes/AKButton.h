#ifndef CZ_AKBUTTON_H
#define CZ_AKBUTTON_H

#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Nodes/AKText.h>
#include <CZ/AK/Nodes/AKFontIcon.h>
#include <CZ/Core/CZAdaptive.h>
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
        CHLast
    };

    AKButton(const std::string &text, AKNode *parent = nullptr) noexcept;
    AKButton(const std::string &text, const std::string &icon, AKNode *parent = nullptr) noexcept;

    AKText &textNode() noexcept { return m_text; }
    AKFontIcon &icon() noexcept { return m_icon; }

    /**
     * @brief Base fill color; hover/pressed state layers are blended over it.
     *
     * Accepts a @ref CZAdaptiveColor (distinct light/dark) or a single SkColor (implicitly
     * converted, used for both). Defaults to @ref AKTheme::ControlAccent, the macOS control tint.
     */
    void setBackgroundColor(CZAdaptiveColor color) noexcept;

    /** @brief The adaptive fill color. */
    CZAdaptiveColor backgroundColor() const noexcept { return m_backgroundColor; }

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

    void setHovered(bool hover) noexcept;
    bool hovered() const noexcept { return m_hovered; }

    CZSignal<const CZPointerButtonEvent&> onClick;

protected:
    void init() noexcept;
    void pointerButtonEvent(const CZPointerButtonEvent &event) override;
    void pointerEnterEvent(const CZPointerEnterEvent &event) override;
    void pointerLeaveEvent(const CZPointerLeaveEvent &event) override;
    void windowStateEvent(const CZWindowStateEvent &event) override;
    void updateStyleColor() noexcept;

    CZAdaptiveColor m_backgroundColor;
    CZAdaptiveColor m_contentColor;
    bool m_contentColorAuto { true };
    AKFontIcon m_icon;
    AKText m_text;
    bool m_enabled { true };
    bool m_pressed { false };
    bool m_hovered { false };
};

#endif // CZ_AKBUTTON_H
