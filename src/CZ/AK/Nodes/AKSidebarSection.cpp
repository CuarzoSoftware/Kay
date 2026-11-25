#include <CZ/AK/Nodes/AKSidebarSection.h>
#include <CZ/AK/Nodes/AKSidebarList.h>
#include <CZ/AK/AKTheme.h>
#include <algorithm>

using namespace CZ;

AKSidebarSection::AKSidebarSection(AKSidebarList &list, const std::string &title, AKNode *parent) noexcept :
    AKContainer(YGFlexDirectionColumn, false, parent),
    m_list(list)
{
    layout().setFlexShrink(0.f);

    const SkRegion empty;
    titleText.setInputRegion(&empty);
    titleText.setFontSize(TitleFontSize);
    titleText.setFontStyle(SkFontStyle(SkFontStyle::kBold_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    titleText.setTextColor(AKTheme::TertiaryLabel);

    // Aligned with the rows' icons, at the bottom of its row
    titleText.layout().setMargin(YGEdgeLeft, 8.f);
    titleText.layout().setMargin(YGEdgeTop, TitleHeight - TitleFontSize * 1.3f - 4.f);
    titleText.layout().setMargin(YGEdgeBottom, 4.f);
    setTitle(title);
}

void AKSidebarSection::setTitle(const std::string &title) noexcept
{
    m_title = title;
    titleText.setText(title);
    titleText.setVisible(!title.empty());
}

AKSidebarItem &AKSidebarSection::addItem(const std::string &text, const std::string &iconName, CZAdaptiveColor tileColor, const std::string &id) noexcept
{
    auto &item { m_items.emplace_back(std::make_unique<AKSidebarItem>(text, iconName, tileColor, this)) };
    item->id = id;
    AKSidebarItem *raw { item.get() };
    raw->onClicked.subscribe(&m_list, [list = &m_list, raw]{ list->setSelectedItem(raw); });
    return *raw;
}

void AKSidebarSection::removeItem(AKSidebarItem *item) noexcept
{
    auto it { std::find_if(m_items.begin(), m_items.end(), [item](const auto &i){ return i.get() == item; }) };

    if (it == m_items.end())
        return;

    if (m_list.selectedItem() == item)
        m_list.setSelectedItem(nullptr);

    m_items.erase(it);
}

void AKSidebarSection::clear() noexcept
{
    while (!m_items.empty())
        removeItem(m_items.back().get());
}
