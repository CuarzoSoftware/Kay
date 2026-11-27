#include <CZ/AK/Nodes/AKDisclosureSection.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZPointerEnterEvent.h>
#include <CZ/Core/Events/CZPointerLeaveEvent.h>
#include <CZ/Core/Events/CZColorSchemeEvent.h>
#include <linux/input-event-codes.h>

using namespace CZ;

AKDisclosureSection::AKDisclosureSection(const std::string &title, bool expanded, AKNode *parent) noexcept :
    AKContainer(YGFlexDirectionColumn, false, parent)
{
    layout().setFlexShrink(0.f);

    const SkRegion empty;
    separator.setInputRegion(&empty);
    separator.layout().setHeight(1.f);
    separator.layout().setFlexShrink(0.f);

    // Clicks land on the header
    arrow.setInputRegion(&empty);
    titleText.setInputRegion(&empty);
    titleText.setFontSize(TitleFontSize);
    titleText.setFontStyle(SkFontStyle(SkFontStyle::kBold_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
    titleText.setText(title);

    content.layout().setFlexShrink(0.f);
    content.layout().setPadding(YGEdgeHorizontal, ContentPadding);
    content.layout().setPadding(YGEdgeTop, 2.f);
    content.layout().setPadding(YGEdgeBottom, 10.f);

    header.updateColors();
    setExpanded(expanded);
}

void AKDisclosureSection::setExpanded(bool expanded) noexcept
{
    m_expanded = expanded;
    content.setVisible(expanded);
    arrow.setIconFromName(expanded ? "arrow_drop_down" : "arrow_right");
}

AKDisclosureSection::Header::Header(AKDisclosureSection &section) noexcept :
    AKRoundSolidColor(0, CZBorderRadius::Make(5), &section),
    m_section(section)
{
    layout().setFlexDirection(YGFlexDirectionRow);
    layout().setAlignItems(YGAlignCenter);
    layout().setHeight(HeaderHeight);
    layout().setFlexShrink(0.f);
    layout().setMargin(YGEdgeHorizontal, HeaderInset);
    layout().setMargin(YGEdgeVertical, 2.f);
    layout().setPadding(YGEdgeLeft, 2.f);
    layout().setGap(YGGutterColumn, 1.f);
    setCursor(CZCursorShape::Default);
}

void AKDisclosureSection::Header::pointerEnterEvent(const CZPointerEnterEvent &e)
{
    AKRoundSolidColor::pointerEnterEvent(e);
    m_hover = true;
    updateColors();
}

void AKDisclosureSection::Header::pointerLeaveEvent(const CZPointerLeaveEvent &e)
{
    AKRoundSolidColor::pointerLeaveEvent(e);
    m_hover = m_pressed = false;
    updateColors();
}

void AKDisclosureSection::Header::pointerButtonEvent(const CZPointerButtonEvent &e)
{
    AKRoundSolidColor::pointerButtonEvent(e);

    if (e.button != BTN_LEFT)
        return;

    if (e.pressed)
        m_pressed = true;
    else if (m_pressed)
    {
        m_pressed = false;
        updateColors();
        m_section.setExpanded(!m_section.expanded());
        m_section.onToggled.notify(m_section.expanded());
        return;
    }

    updateColors();
}

void AKDisclosureSection::Header::colorSchemeEvent(const CZColorSchemeEvent &e)
{
    AKRoundSolidColor::colorSchemeEvent(e);
    updateColors();
}

void AKDisclosureSection::Header::updateColors() noexcept
{
    const CZColorScheme scheme { colorScheme() };
    m_section.separator.setColor(AKTheme::Separator);
    m_section.titleText.setTextColor(AKTheme::Label);
    m_section.arrow.setColor(AKTheme::SecondaryLabel.resolve(scheme));

    if (m_pressed)
        setBackgroundColor(AKTheme::ControlPressedFill.resolve(scheme));
    else if (m_hover)
        setBackgroundColor(AKTheme::ControlHoverFill.resolve(scheme));
    else
        setBackgroundColor(SK_ColorTRANSPARENT);
}
