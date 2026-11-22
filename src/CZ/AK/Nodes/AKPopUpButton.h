#ifndef CZ_AKPOPUPBUTTON_H
#define CZ_AKPOPUPBUTTON_H

#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Nodes/AKFontIcon.h>
#include <CZ/AK/Nodes/AKText.h>
#include <CZ/Core/CZSignal.h>
#include <CZ/Core/CZAdaptive.h>
#include <string>
#include <vector>

/**
 * @brief A drop-down list: shows the selected item and lists them all when clicked, like macOS's
 *        pop-up buttons.
 * @ingroup AKNodes
 *
 * [selected item's text][up/down chevrons]
 *
 * Borderless, highlighted while hovered or pressed (e.g. at the trailing side of an AKFormRow).
 * It holds the items (setItems()) and the selected one (selectedIndex()), but Kay can't open
 * popup windows: clicking it calls openMenu(), which emits onMenuRequested. Toolkits override it
 * to show the list (e.g. Marco's MPopUpButton, as an MMenu) and call select() with the user's
 * choice, which emits onSelected.
 */
class CZ::AKPopUpButton : public AKRoundSolidColor
{
public:
    /** @brief Creates it with @p items, the first one selected (if any). */
    AKPopUpButton(const std::vector<std::string> &items = {}, AKNode *parent = nullptr) noexcept;

    static constexpr SkScalar Height   { 22.f };
    static constexpr SkScalar FontSize { 13.f };

    /// No item selected, see selectedIndex().
    static constexpr size_t None { SIZE_MAX };

    /** @brief Replaces the items, selecting @p selected (None: none). Doesn't emit onSelected. */
    void setItems(const std::vector<std::string> &items, size_t selected = 0) noexcept;
    const std::vector<std::string> &items() const noexcept { return m_items; }

    /** @brief Selects the item at @p index (None: none) without emitting onSelected. */
    void setSelectedIndex(size_t index) noexcept;

    /** @brief The selected item, None if none. */
    size_t selectedIndex() const noexcept { return m_selected; }

    /** @brief The selected item's text, empty if none. */
    const std::string &selectedText() const noexcept;

    /** @brief Selects the item at @p index as the user would (emits onSelected if it changes). */
    void select(size_t index) noexcept;

    /** @brief Highlighted as if pressed, e.g. while its menu is open (see MPopUpButton). */
    void setMenuOpen(bool open) noexcept;
    bool menuOpen() const noexcept { return m_menuOpen; }

    /// Fill while hovered.
    static inline CZAdaptiveColor HoverColor   { 0x1F000000, 0x24FFFFFF };
    /// Fill while pressed or its menu is open.
    static inline CZAdaptiveColor PressedColor { 0x33000000, 0x3DFFFFFF };

    /** @brief Dimmed and not clickable while disabled. */
    void setEnabled(bool enabled) noexcept;
    bool enabled() const noexcept { return m_enabled; }

    /** @brief The user selected another item (its index), see select(). */
    CZSignal<size_t> onSelected;

    /** @brief Clicked: the list should be shown (see openMenu()). */
    CZSignal<const CZPointerButtonEvent&> onMenuRequested;

    AKText label { "", this };
    AKFontIcon chevron { "unfold_more", 14, this };

protected:
    /**
     * @brief Shows the list (clicked). Emits onMenuRequested by default.
     *
     * @param event The click (the button release) that opened it (e.g. for a popup grab).
     */
    virtual void openMenu(const CZPointerButtonEvent &event) noexcept;

    void pointerEnterEvent(const CZPointerEnterEvent &e) override;
    void pointerLeaveEvent(const CZPointerLeaveEvent &e) override;
    void pointerButtonEvent(const CZPointerButtonEvent &e) override;
    void colorSchemeEvent(const CZColorSchemeEvent &e) override;
    /// @cond INTERNAL
    void updateColors() noexcept;
    std::vector<std::string> m_items;
    size_t m_selected { None };
    bool m_enabled { true }, m_hover { false }, m_pressed { false }, m_menuOpen { false };
    /// @endcond
};

#endif // CZ_AKPOPUPBUTTON_H
