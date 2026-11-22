#include <CZ/AK/Nodes/AKFormSection.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/Core/Events/CZLayoutEvent.h>
#include <CZ/Core/Events/CZColorSchemeEvent.h>

using namespace CZ;

AKFormSection::AKFormSection(const std::string &titleText, AKNode *parent) noexcept :
    AKContainer(YGFlexDirectionColumn, false, parent)
{
    layout().setGap(YGGutterRow, 8.f);
    layout().setFlexShrink(0.f);

    const SkRegion empty;
    title.setInputRegion(&empty);
    title.setFontSize(13.f);
    title.setFontStyle(SkFontStyle(SkFontStyle::kBold_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    title.setTextColor(AKTheme::Label);
    title.layout().setMargin(YGEdgeLeft, AKFormRow::Padding);
    setTitle(titleText);

    card.onChildAdded.subscribe(this, [this](AKNode*, AKNode*){ updateSeparators(); });
    card.onChildRemoved.subscribe(this, [this](AKNode*){ updateSeparators(); });
}

void AKFormSection::setTitle(const std::string &text) noexcept
{
    title.setText(text);
    title.setVisible(!text.empty());
}

void AKFormSection::updateSeparators() noexcept
{
    bool first { true };

    for (AKNode *child : card.children())
    {
        auto *row { dynamic_cast<AKFormRow*>(child) };

        if (!row || !row->visible())
            continue;

        row->separator.setVisible(!first);
        first = false;
    }
}

AKFormSection::Card::Card(AKFormSection &section) noexcept :
    AKRoundSolidColor(0, CZBorderRadius::Make(10), &section),
    m_section(section)
{
    layout().setFlexDirection(YGFlexDirectionColumn);
    layout().setPadding(YGEdgeAll, 1.f); // The hairline
    setStrokeWidth(1);
    updateColors();
}

void AKFormSection::Card::layoutEvent(const CZLayoutEvent &e)
{
    AKRoundSolidColor::layoutEvent(e);

    // Rows shown or hidden change its size
    if (e.changes.has(CZLayoutChangeSize))
        m_section.updateSeparators();
}

void AKFormSection::Card::colorSchemeEvent(const CZColorSchemeEvent &e)
{
    AKRoundSolidColor::colorSchemeEvent(e);
    updateColors();
}

void AKFormSection::Card::updateColors() noexcept
{
    setBackgroundColor(CardColor.resolve(colorScheme()));
    setStrokeColor(CardBorder.resolve(colorScheme()));
}
