#include <CZ/AK/Nodes/AKFormRow.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZPointerEnterEvent.h>
#include <CZ/Core/Events/CZPointerLeaveEvent.h>
#include <CZ/Core/Events/CZColorSchemeEvent.h>
#include <linux/input-event-codes.h>

using namespace CZ;

AKFormRow::AKFormRow(const std::string &titleText, AKNode *parent) noexcept :
    AKRoundSolidColor(0, CZBorderRadius::Make(6), parent)
{
    layout().setFlexDirection(YGFlexDirectionRow);
    layout().setAlignItems(YGAlignCenter);
    layout().setMinHeight(MinHeight);
    layout().setPadding(YGEdgeHorizontal, Padding);
    layout().setPadding(YGEdgeVertical, 8.f);
    layout().setGap(YGGutterColumn, 10.f);
    layout().setFlexShrink(0.f);

    const SkRegion empty;
    for (AKNode *n : std::initializer_list<AKNode*>{ &separator, &texts, &title, &subtitle })
        n->setInputRegion(&empty);

    separator.layout().setPositionType(YGPositionTypeAbsolute);
    separator.layout().setPosition(YGEdgeTop, 0.f);
    separator.layout().setPosition(YGEdgeLeft, Padding);
    separator.layout().setPosition(YGEdgeRight, Padding);
    separator.layout().setHeight(1.f);
    separator.setVisible(false);

    leading.layout().setAlignItems(YGAlignCenter);
    leading.layout().setGap(YGGutterColumn, 8.f);
    leading.layout().setFlexShrink(0.f);

    texts.layout().setFlex(1.f);
    texts.layout().setGap(YGGutterRow, 2.f);

    title.setFontSize(13.f);
    title.setText(titleText);
    subtitle.setFontSize(11.f);
    subtitle.setVisible(false);

    trailing.layout().setAlignItems(YGAlignCenter);
    trailing.layout().setGap(YGGutterColumn, 10.f);
    trailing.layout().setFlexShrink(0.f);

    updateColors();
}

void AKFormRow::setSubtitle(const std::string &text) noexcept
{
    subtitle.setText(text);
    subtitle.setVisible(!text.empty());
}

void AKFormRow::setClickable(bool clickable) noexcept
{
    if (m_clickable == clickable)
        return;

    m_clickable = clickable;
    m_hover = m_pressed = false;
    updateColors();
}

void AKFormRow::pointerEnterEvent(const CZPointerEnterEvent &e)
{
    AKRoundSolidColor::pointerEnterEvent(e);
    m_hover = m_clickable;
    updateColors();
}

void AKFormRow::pointerLeaveEvent(const CZPointerLeaveEvent &e)
{
    AKRoundSolidColor::pointerLeaveEvent(e);
    m_hover = m_pressed = false;
    updateColors();
}

void AKFormRow::pointerButtonEvent(const CZPointerButtonEvent &e)
{
    AKRoundSolidColor::pointerButtonEvent(e);

    if (!m_clickable || e.button != BTN_LEFT)
        return;

    if (e.pressed)
        m_pressed = true;
    else if (m_pressed)
    {
        m_pressed = false;
        onClicked.notify(e);
    }

    updateColors();
}

void AKFormRow::colorSchemeEvent(const CZColorSchemeEvent &e)
{
    AKRoundSolidColor::colorSchemeEvent(e);
    updateColors();
}

void AKFormRow::updateColors() noexcept
{
    const CZColorScheme scheme { colorScheme() };
    separator.setColor(AKTheme::Separator);
    title.setTextColor(AKTheme::Label);
    subtitle.setTextColor(AKTheme::SecondaryLabel);

    if (m_pressed)
        setBackgroundColor(AKTheme::ControlPressedFill.resolve(scheme));
    else if (m_hover)
        setBackgroundColor(AKTheme::ControlHoverFill.resolve(scheme));
    else
        setBackgroundColor(SK_ColorTRANSPARENT);
}
