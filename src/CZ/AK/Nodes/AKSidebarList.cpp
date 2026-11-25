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

AKSidebarList::~AKSidebarList() noexcept
{
    // Rows go first: nothing to notify about anymore
    m_selected = nullptr;
    m_sections.clear();
}

AKSidebarItem &AKSidebarList::addItem(const std::string &text, const std::string &iconName, CZAdaptiveColor tileColor, const std::string &id) noexcept
{
    if (m_sections.empty())
        addSection();

    return m_sections.back()->addItem(text, iconName, tileColor, id);
}

AKSidebarSection &AKSidebarList::addSection(const std::string &title) noexcept
{
    const bool first { m_sections.empty() };
    auto &section { m_sections.emplace_back(new AKSidebarSection(*this, title, slot())) };

    if (!first)
        section->layout().setMargin(YGEdgeTop, title.empty() ? SectionGap : SectionGap - 8.f);

    return *section;
}

std::vector<AKSidebarItem*> AKSidebarList::items() const noexcept
{
    std::vector<AKSidebarItem*> items;

    for (const auto &section : m_sections)
        for (const auto &item : section->items())
            items.emplace_back(item.get());

    return items;
}

AKSidebarItem *AKSidebarList::item(const std::string &id) const noexcept
{
    for (const auto &section : m_sections)
        for (const auto &item : section->items())
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
