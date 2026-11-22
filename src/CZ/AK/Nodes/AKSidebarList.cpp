#include <CZ/AK/Nodes/AKSidebarList.h>

using namespace CZ;

AKSidebarList::AKSidebarList(AKNode *parent) noexcept : AKScroll(parent)
{
    slot()->layout().setPadding(YGEdgeHorizontal, Padding);
    slot()->layout().setPadding(YGEdgeBottom, Padding);

    // Knob-only scrollers over the sidebar's background, like macOS's
    verticalBar().setRailVisible(false);
    horizontalBar().setRailVisible(false);
}

AKSidebarItem &AKSidebarList::addItem(const std::string &text, const std::string &iconName, CZAdaptiveColor tileColor, const std::string &id) noexcept
{
    auto &item { m_items.emplace_back(std::make_unique<AKSidebarItem>(text, iconName, tileColor, this)) };
    item->id = id;

    if (m_sectionPending)
    {
        m_sectionPending = false;
        item->layout().setMargin(YGEdgeTop, SectionGap);
    }

    AKSidebarItem *raw { item.get() };
    raw->onClicked.subscribe(this, [this, raw]{ setSelectedItem(raw); });
    return *raw;
}

void AKSidebarList::addSection() noexcept
{
    // Not before the first row
    m_sectionPending = !m_items.empty();
}

AKSidebarItem *AKSidebarList::item(const std::string &id) const noexcept
{
    for (const auto &item : m_items)
        if (item->id == id)
            return item.get();

    return nullptr;
}

void AKSidebarList::setSelectedItem(AKSidebarItem *item) noexcept
{
    if (item == m_selected)
        return;

    if (m_selected)
        m_selected->setSelected(false);

    m_selected = item;

    if (m_selected)
        m_selected->setSelected(true);

    onSelectionChanged.notify(m_selected);
}
