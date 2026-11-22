#ifndef CZ_AKICONTILE_H
#define CZ_AKICONTILE_H

#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Nodes/AKFontIcon.h>
#include <CZ/Core/CZAdaptive.h>
#include <optional>

/**
 * @brief An icon of the theme's icon font on a colored rounded square, like macOS's settings icons.
 * @ingroup AKNodes
 *
 * The icon is white over the tile. With a transparent tile color only the icon is drawn, colored
 * like a label (AKTheme::Label). setIconColor() overrides either.
 */
class CZ::AKIconTile : public AKRoundSolidColor
{
public:
    /** @brief Creates a @p size units tile showing @p iconName over @p color. */
    AKIconTile(const std::string &iconName, CZAdaptiveColor color, Int32 size = 20, AKNode *parent = nullptr) noexcept;

    /** @brief Sets the size (the corner radius and the icon scale with it). */
    void setSize(Int32 size) noexcept;
    Int32 size() const noexcept { return m_size; }

    /** @brief The tile's color (transparent: no tile). */
    void setTileColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor tileColor() const noexcept { return m_tileColor; }

    /** @brief Overrides the icon's color (std::nullopt: automatic, see the class description). */
    void setIconColor(std::optional<CZAdaptiveColor> color) noexcept;

    /** @brief The icon (e.g. icon.setIconFromName()). */
    AKFontIcon icon { "", 14, this };

protected:
    void colorSchemeEvent(const CZColorSchemeEvent &e) override;
    /// @cond INTERNAL
    void updateColors() noexcept;
    CZAdaptiveColor m_tileColor;
    std::optional<CZAdaptiveColor> m_iconColor;
    Int32 m_size { 0 };
    /// @endcond
};

#endif // CZ_AKICONTILE_H
