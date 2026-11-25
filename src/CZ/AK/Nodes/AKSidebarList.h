#ifndef CZ_AKSIDEBARLIST_H
#define CZ_AKSIDEBARLIST_H

#include <CZ/AK/Nodes/AKScroll.h>
#include <CZ/AK/Nodes/AKSidebarItem.h>
#include <CZ/AK/Nodes/AKSidebarSection.h>
#include <CZ/Core/CZSignal.h>
#include <memory>
#include <vector>

/**
 * @brief A scrollable list of AKSidebarItem rows, in sections, with one of them selected.
 * @ingroup AKNodes
 *
 * Like the lists of macOS's sidebars (System Settings, Finder). Rows are grouped in sections
 * (AKSidebarSection, owned by the list), separated by a gap and optionally titled (e.g. Finder's
 * "Favorites" and "Locations"):
 *
 * @code
 * AKSidebarList list { &sidebar.content };
 * auto &favorites { list.addSection("Favorites") };
 * favorites.addItem("Desktop", "desktop_windows", CZAdaptiveColor(0));
 * auto &locations { list.addSection("Locations") };
 * @endcode
 *
 * addItem() adds a row to the last section (creating an untitled one if there's none yet). Rows can
 * be added to and removed from any section later.
 *
 * Clicking a row selects it (see onSelectionChanged). Its scroll bars have no rail
 * (AKScrollBar::setRailVisible()), just the knob over the sidebar's background.
 *
 * Other nodes can be added to the list too (e.g. a header above the rows): pass the list as their
 * parent, they're laid out in order with the sections.
 */
class CZ::AKSidebarList : public AKScroll
{
public:
    /** @brief Creates an empty list. */
    AKSidebarList(AKNode *parent = nullptr) noexcept;
    ~AKSidebarList() noexcept;

    static constexpr SkScalar SectionGap { 14.f }; ///< Space between sections
    static constexpr SkScalar Padding    { 10.f }; ///< Horizontal padding of the rows

    /** @brief Adds a row to the last section (see AKSidebarSection::addItem()). */
    AKSidebarItem &addItem(const std::string &text, const std::string &iconName, CZAdaptiveColor tileColor, const std::string &id = "") noexcept;

    /**
     * @brief Starts a new section, after the others.
     *
     * Untitled sections are just a SectionGap below the previous one (the first one has no gap).
     */
    AKSidebarSection &addSection(const std::string &title = "") noexcept;

    /** @brief The sections, in order. */
    const std::vector<std::unique_ptr<AKSidebarSection>> &sections() const noexcept { return m_sections; }

    /** @brief Every row, in order. */
    std::vector<AKSidebarItem*> items() const noexcept;

    /** @brief The row with that id, or nullptr. */
    AKSidebarItem *item(const std::string &id) const noexcept;

    /** @brief Selects @p item (one of items(), or nullptr for none). Emits onSelectionChanged if it changes. */
    void setSelectedItem(AKSidebarItem *item) noexcept;

    /** @brief The selected row, or nullptr. */
    AKSidebarItem *selectedItem() const noexcept { return m_selected; }

    /** @brief Emitted after the selected row changes (nullptr: none). */
    CZSignal<AKSidebarItem*> onSelectionChanged;

private:
    std::vector<std::unique_ptr<AKSidebarSection>> m_sections;
    AKSidebarItem *m_selected { nullptr };
};

#endif // CZ_AKSIDEBARLIST_H
