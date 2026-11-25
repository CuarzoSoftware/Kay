#ifndef CZ_AKSIDEBARSECTION_H
#define CZ_AKSIDEBARSECTION_H

#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKSidebarItem.h>
#include <CZ/AK/Nodes/AKText.h>
#include <memory>
#include <string>
#include <vector>

/**
 * @brief A group of rows of an AKSidebarList, with an optional title above them.
 * @ingroup AKNodes
 *
 * Like the "Favorites" and "Locations" groups of Finder's sidebar. Created by
 * AKSidebarList::addSection(), which owns it. Rows can be added and removed at any time (e.g. the
 * disks of a "Locations" section as they're plugged and unplugged): the list keeps track of the
 * selected one.
 */
class CZ::AKSidebarSection : public AKContainer
{
public:
    static constexpr SkScalar TitleFontSize { 11.f };
    static constexpr SkScalar TitleHeight   { 26.f }; ///< Height of the title row

    /** @brief Sets the title shown above the rows (empty: none). */
    void setTitle(const std::string &title) noexcept;
    const std::string &title() const noexcept { return m_title; }

    /** @brief Adds a row at the end (see AKSidebarItem's constructor). */
    AKSidebarItem &addItem(const std::string &text, const std::string &iconName, CZAdaptiveColor tileColor, const std::string &id = "") noexcept;

    /**
     * @brief Removes and destroys @p item (one of items()).
     *
     * If it was selected the list is left without a selection (AKSidebarList::onSelectionChanged
     * is emitted with nullptr).
     */
    void removeItem(AKSidebarItem *item) noexcept;

    /** @brief Removes every row (see removeItem()). */
    void clear() noexcept;

    /** @brief The rows, in order. */
    const std::vector<std::unique_ptr<AKSidebarItem>> &items() const noexcept { return m_items; }

    /** @brief The list it belongs to. */
    AKSidebarList &list() const noexcept { return m_list; }

    /** @brief The title (hidden while empty). */
    AKText titleText { "", this };

private:
    friend class AKSidebarList;
    AKSidebarSection(AKSidebarList &list, const std::string &title, AKNode *parent) noexcept;
    AKSidebarList &m_list;
    std::vector<std::unique_ptr<AKSidebarItem>> m_items;
    std::string m_title;
};

#endif // CZ_AKSIDEBARSECTION_H
