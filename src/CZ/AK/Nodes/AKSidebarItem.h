#ifndef CZ_AKSIDEBARITEM_H
#define CZ_AKSIDEBARITEM_H

#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Nodes/AKIconTile.h>
#include <CZ/AK/Nodes/AKText.h>
#include <CZ/AK/Nodes/AKIconButton.h>
#include <CZ/Core/CZAdaptive.h>
#include <CZ/Core/CZSignal.h>
#include <memory>
#include <optional>
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
 * with the label color, or setIconColor()'s, like the accent colored icons of Finder's sidebar).
 *
 * A small button can be shown at its trailing end with setAccessory() (e.g. Finder's eject
 * button next to a removable disk). Clicking it doesn't click the row.
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

    /**
     * @brief Highlights the row (a rounded fill, AKTheme::ControlHoverFill) while something
     *        dragged over it would be dropped into it (e.g. files over a folder).
     */
    void setDropTarget(bool target) noexcept;
    bool dropTarget() const noexcept { return m_dropTarget; }

    /** @brief The tile behind the icon (adaptive; transparent: no tile, the icon gets the label color). */
    void setTileColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor tileColor() const noexcept { return m_tileColor; }

    /**
     * @brief Color of the icon while the row isn't highlighted (std::nullopt: automatic).
     *
     * Automatic is white over a tile, the label color without one. Highlighted (selected in an
     * activated window) an icon without a tile is always white.
     */
    void setIconColor(std::optional<CZAdaptiveColor> color) noexcept;
    std::optional<CZAdaptiveColor> iconColor() const noexcept { return m_iconColor; }

    /**
     * @brief Shows a button with the icon @p iconName (of the theme's icon font) at the trailing end.
     *
     * Created the first time, then its icon is replaced. Clicks on it emit onAccessoryClicked, not
     * onClicked. An empty @p iconName removes it.
     *
     * @return The button, or nullptr if removed.
     */
    AKIconButton *setAccessory(const std::string &iconName) noexcept;

    /** @brief The trailing button (see setAccessory()), or nullptr. */
    AKIconButton *accessory() const noexcept { return m_accessory.get(); }

    /** @brief Emitted when the row is clicked (pressed and released on it). */
    CZSignal<> onClicked;

    /** @brief Emitted when the accessory button is clicked (see setAccessory()). */
    CZSignal<> onAccessoryClicked;

    AKIconTile tile { "", CZAdaptiveColor(0), Int32(TileSize), this }; ///< The icon on its colored square
    AKText label { "", this };                                    ///< The row's text

protected:
    void pointerButtonEvent(const CZPointerButtonEvent &e) override;
    void pointerLeaveEvent(const CZPointerLeaveEvent &e) override;
    void colorSchemeEvent(const CZColorSchemeEvent &e) override;
    void windowStateEvent(const CZWindowStateEvent &e) override;
    /// @cond INTERNAL
    void updateColors() noexcept;
    bool overAccessory() const noexcept;
    std::unique_ptr<AKIconButton> m_accessory;
    std::optional<CZAdaptiveColor> m_iconColor;
    CZAdaptiveColor m_tileColor;
    bool m_selected { false };
    bool m_pressed { false };
    bool m_dropTarget { false };
    /// @endcond
};

#endif // CZ_AKSIDEBARITEM_H
