#ifndef CZ_AKSIDEBARITEM_H
#define CZ_AKSIDEBARITEM_H

#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Nodes/AKIconTile.h>
#include <CZ/AK/Nodes/AKText.h>
#include <CZ/Core/CZAdaptive.h>
#include <CZ/Core/CZSignal.h>
#include <string>

/**
 * @brief A row of a sidebar (see AKSidebarList): an icon on a colored rounded tile and a label.
 * @ingroup AKNodes
 *
 * Like the rows of macOS's sidebars (System Settings, Finder...). While selected, it's filled with
 * the accent color and its label is white; while the window isn't activated, with a gray instead
 * (and the label keeps its color).
 *
 * The tile can be hidden (setTileColor() with a transparent color shows just the icon, tinted
 * with the label color).
 */
class CZ::AKSidebarItem : public AKRoundSolidColor
{
public:
    /**
     * @brief Creates the row.
     *
     * @param text     The label.
     * @param iconName An icon of the theme's icon font (e.g. "wifi").
     * @param tileColor The tile behind the icon (adaptive; transparent: no tile).
     */
    AKSidebarItem(const std::string &text, const std::string &iconName, CZAdaptiveColor tileColor, AKNode *parent = nullptr) noexcept;

    static constexpr SkScalar Height   { 28.f };
    static constexpr SkScalar TileSize { 20.f };
    static constexpr SkScalar FontSize { 13.f };

    /** @brief Optional identifier, for the app (e.g. the pane the row opens). */
    std::string id;

    /** @brief Highlights the row (normally done by its AKSidebarList). */
    void setSelected(bool selected) noexcept;
    bool selected() const noexcept { return m_selected; }

    /** @brief The tile behind the icon (adaptive; transparent: no tile, the icon gets the label color). */
    void setTileColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor tileColor() const noexcept { return m_tileColor; }

    /** @brief Emitted when the row is clicked (pressed and released on it). */
    CZSignal<> onClicked;

    AKIconTile tile { "", CZAdaptiveColor(0), Int32(TileSize), this }; ///< The icon on its colored square
    AKText label { "", this };                                    ///< The row's text

protected:
    void pointerButtonEvent(const CZPointerButtonEvent &e) override;
    void pointerLeaveEvent(const CZPointerLeaveEvent &e) override;
    void colorSchemeEvent(const CZColorSchemeEvent &e) override;
    void windowStateEvent(const CZWindowStateEvent &e) override;
    /// @cond INTERNAL
    void updateColors() noexcept;
    CZAdaptiveColor m_tileColor;
    bool m_selected { false };
    bool m_pressed { false };
    /// @endcond
};

#endif // CZ_AKSIDEBARITEM_H
