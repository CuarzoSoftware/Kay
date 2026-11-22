#include <CZ/AK/Nodes/AKPopUpButton.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZPointerEnterEvent.h>
#include <CZ/Core/Events/CZPointerLeaveEvent.h>
#include <CZ/Core/Events/CZColorSchemeEvent.h>
#include <linux/input-event-codes.h>

using namespace CZ;

AKPopUpButton::AKPopUpButton(const std::vector<std::string> &items, AKNode *parent) noexcept :
    AKRoundSolidColor(0, CZBorderRadius::Make(5), parent)
{
    layout().setFlexDirection(YGFlexDirectionRow);
    layout().setAlignItems(YGAlignCenter);
    layout().setHeight(Height);
    layout().setPadding(YGEdgeLeft, 7.f);
    layout().setPadding(YGEdgeRight, 3.f);
    layout().setGap(YGGutterColumn, 2.f);
    layout().setFlexShrink(0.f);
    setCursor(CZCursorShape::Default);

    const SkRegion empty;
    label.setInputRegion(&empty);
    label.setFontSize(FontSize);
    chevron.setInputRegion(&empty);

    setItems(items);
    updateColors();
}

void AKPopUpButton::setItems(const std::vector<std::string> &items, size_t selected) noexcept
{
    m_items = items;
    m_selected = None;
    setSelectedIndex(selected);
}

void AKPopUpButton::setSelectedIndex(size_t index) noexcept
{
    m_selected = index < m_items.size() ? index : None;
    label.setText(selectedText());
}

const std::string &AKPopUpButton::selectedText() const noexcept
{
    static const std::string none;
    return m_selected == None ? none : m_items[m_selected];
}

void AKPopUpButton::select(size_t index) noexcept
{
    if (index >= m_items.size() || index == m_selected)
        return;

    setSelectedIndex(index);
    onSelected.notify(index);
}

void AKPopUpButton::setMenuOpen(bool open) noexcept
{
    m_menuOpen = open;
    updateColors();
}

void AKPopUpButton::setEnabled(bool enabled) noexcept
{
    m_enabled = enabled;
    m_hover = m_pressed = false;
    setOpacity(enabled ? 1.f : 0.4f);
    updateColors();
}

void AKPopUpButton::openMenu(const CZPointerButtonEvent &event) noexcept
{
    onMenuRequested.notify(event);
}

void AKPopUpButton::pointerEnterEvent(const CZPointerEnterEvent &e)
{
    AKRoundSolidColor::pointerEnterEvent(e);
    m_hover = m_enabled;
    updateColors();
}

void AKPopUpButton::pointerLeaveEvent(const CZPointerLeaveEvent &e)
{
    AKRoundSolidColor::pointerLeaveEvent(e);
    m_hover = m_pressed = false;
    updateColors();
}

void AKPopUpButton::pointerButtonEvent(const CZPointerButtonEvent &e)
{
    AKRoundSolidColor::pointerButtonEvent(e);

    if (!m_enabled || e.button != BTN_LEFT)
        return;

    // Opened when clicked (released over it): opened on press, the release would land outside the
    // menu and dismiss it right away
    if (e.pressed)
        m_pressed = true;
    else if (m_pressed)
    {
        m_pressed = false;
        updateColors();
        openMenu(e);
        return;
    }

    updateColors();
}

void AKPopUpButton::colorSchemeEvent(const CZColorSchemeEvent &e)
{
    AKRoundSolidColor::colorSchemeEvent(e);
    updateColors();
}

void AKPopUpButton::updateColors() noexcept
{
    const CZColorScheme scheme { colorScheme() };
    label.setTextColor(AKTheme::Label);
    chevron.setColor(AKTheme::SecondaryLabel.resolve(scheme));

    if (m_pressed || m_menuOpen)
        setBackgroundColor(PressedColor.resolve(scheme));
    else if (m_hover)
        setBackgroundColor(HoverColor.resolve(scheme));
    else
        setBackgroundColor(SK_ColorTRANSPARENT);
}
