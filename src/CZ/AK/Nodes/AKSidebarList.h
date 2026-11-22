#ifndef CZ_AKSIDEBARLIST_H
#define CZ_AKSIDEBARLIST_H

#include <CZ/AK/Nodes/AKScroll.h>
#include <CZ/AK/Nodes/AKSidebarItem.h>
#include <CZ/Core/CZSignal.h>
#include <memory>
#include <vector>

/**
 * @brief A scrollable list of AKSidebarItem rows, in sections, with one of them selected.
 * @ingroup AKNodes
 *
 * Like the list of macOS's System Settings sidebar. Rows are added with addItem() (owned by the
 * list) and grouped with addSection(), which leaves a gap before the next row. Clicking a row
 * selects it (see onSelectionChanged). Its scroll bars have no rail (AKScrollBar::setRailVisible()),
 * just the knob over the sidebar's background.
 *
 * Other nodes can be added to the list too (e.g. a header above the rows): pass the list as their
 * parent, they're laid out in order with the rows.
 */
class CZ::AKSidebarList : public AKScroll
{
public:
    /** @brief Creates an empty list. */
    AKSidebarList(AKNode *parent = nullptr) noexcept;

    static constexpr SkScalar SectionGap { 14.f }; ///< Space between sections
    static constexpr SkScalar Padding    { 10.f }; ///< Horizontal padding of the rows

    /** @brief Adds a row (see AKSidebarItem's constructor). */
    AKSidebarItem &addItem(const std::string &text, const std::string &iconName, CZAdaptiveColor tileColor, const std::string &id = "") noexcept;

    /** @brief Starts a new section: the next row is SectionGap below the previous one. */
    void addSection() noexcept;

    /** @brief The rows, in order. */
    const std::vector<std::unique_ptr<AKSidebarItem>> &items() const noexcept { return m_items; }

    /** @brief The row with that id, or nullptr. */
    AKSidebarItem *item(const std::string &id) const noexcept;

    /** @brief Selects @p item (one of items(), or nullptr for none). Emits onSelectionChanged if it changes. */
    void setSelectedItem(AKSidebarItem *item) noexcept;

    /** @brief The selected row, or nullptr. */
    AKSidebarItem *selectedItem() const noexcept { return m_selected; }

    /** @brief Emitted after the selected row changes (nullptr: none). */
    CZSignal<AKSidebarItem*> onSelectionChanged;

private:
    std::vector<std::unique_ptr<AKSidebarItem>> m_items;
    AKSidebarItem *m_selected { nullptr };
    bool m_sectionPending { false };
};

#endif // CZ_AKSIDEBARLIST_H
