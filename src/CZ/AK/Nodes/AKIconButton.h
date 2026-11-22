#ifndef CZ_AKICONBUTTON_H
#define CZ_AKICONBUTTON_H

#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Nodes/AKFontIcon.h>
#include <CZ/Core/CZAdaptive.h>
#include <CZ/Core/CZSignal.h>

/**
 * @brief A round, borderless button showing an icon of the theme's icon font.
 * @ingroup AKNodes
 *
 * E.g. the "…" (more_horiz) buttons that open a row's menu in macOS's settings. Hovering it shows
 * a faint circle behind the icon, pressing it a darker one. setOutlined() draws a ring around the
 * icon (like SF Symbols' "ellipsis.circle").
 */
class CZ::AKIconButton : public AKRoundSolidColor
{
public:
    /** @brief Creates a @p size units button showing @p iconName. */
    AKIconButton(const std::string &iconName, Int32 size = 22, AKNode *parent = nullptr) noexcept;

    /** @brief Draws a ring around the icon. */
    void setOutlined(bool outlined) noexcept;
    bool outlined() const noexcept { return m_outlined; }

    /** @brief Color of the icon (and ring). AKTheme::SecondaryLabel by default. */
    void setIconColor(CZAdaptiveColor color) noexcept;

    void setEnabled(bool enabled) noexcept;
    bool enabled() const noexcept { return m_enabled; }

    /** @brief Clicked (released over it): the release event (e.g. to grab a popup with it). */
    CZSignal<const CZPointerButtonEvent&> onClick;

    AKFontIcon icon { "", 16, this };

protected:
    void pointerEnterEvent(const CZPointerEnterEvent &e) override;
    void pointerLeaveEvent(const CZPointerLeaveEvent &e) override;
    void pointerButtonEvent(const CZPointerButtonEvent &e) override;
    void colorSchemeEvent(const CZColorSchemeEvent &e) override;
    /// @cond INTERNAL
    void updateColors() noexcept;
    CZAdaptiveColor m_iconColor;
    bool m_outlined { false }, m_enabled { true }, m_hover { false }, m_pressed { false };
    /// @endcond
};

#endif // CZ_AKICONBUTTON_H
