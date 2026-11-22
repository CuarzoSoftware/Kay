#include <CZ/AK/Nodes/AKIconButton.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZPointerEnterEvent.h>
#include <CZ/Core/Events/CZPointerLeaveEvent.h>
#include <CZ/Core/Events/CZColorSchemeEvent.h>
#include <linux/input-event-codes.h>

using namespace CZ;

AKIconButton::AKIconButton(const std::string &iconName, Int32 size, AKNode *parent) noexcept :
    AKRoundSolidColor(0, CZBorderRadius::Make(size / 2), parent),
    m_iconColor(AKTheme::SecondaryLabel)
{
    layout().setWidth(size);
    layout().setHeight(size);
    layout().setFlexShrink(0.f);
    layout().setJustifyContent(YGJustifyCenter);
    layout().setAlignItems(YGAlignCenter);

    const SkRegion empty;
    icon.setInputRegion(&empty);
    icon.setIconFromName(iconName);
    icon.setSize(UInt32(std::max(1, Int32(size * 0.72f))));
    updateColors();
}

void AKIconButton::setOutlined(bool outlined) noexcept
{
    m_outlined = outlined;

    // The icon shrinks inside the ring
    const Int32 size { Int32(layout().width().value) };
    icon.setSize(UInt32(std::max(1, Int32(size * (outlined ? 0.6f : 0.72f)))));
    updateColors();
}

void AKIconButton::setIconColor(CZAdaptiveColor color) noexcept
{
    m_iconColor = color;
    updateColors();
}

void AKIconButton::setEnabled(bool enabled) noexcept
{
    m_enabled = enabled;
    m_pressed = m_hover = false;
    setOpacity(enabled ? 1.f : 0.4f);
    icon.setOpacity(enabled ? 1.f : 0.4f);
    updateColors();
}

void AKIconButton::pointerEnterEvent(const CZPointerEnterEvent &e)
{
    AKRoundSolidColor::pointerEnterEvent(e);
    m_hover = m_enabled;
    updateColors();
}

void AKIconButton::pointerLeaveEvent(const CZPointerLeaveEvent &e)
{
    AKRoundSolidColor::pointerLeaveEvent(e);
    m_hover = m_pressed = false;
    updateColors();
}

void AKIconButton::pointerButtonEvent(const CZPointerButtonEvent &e)
{
    AKRoundSolidColor::pointerButtonEvent(e);

    if (!m_enabled || e.button != BTN_LEFT)
        return;

    if (e.pressed)
        m_pressed = true;
    else if (m_pressed)
    {
        m_pressed = false;
        onClick.notify(e);
    }

    updateColors();
}

void AKIconButton::colorSchemeEvent(const CZColorSchemeEvent &e)
{
    AKRoundSolidColor::colorSchemeEvent(e);
    updateColors();
}

void AKIconButton::updateColors() noexcept
{
    const CZColorScheme scheme { colorScheme() };
    const SkColor iconColor { m_iconColor.resolve(scheme) };
    icon.setColor(iconColor);

    if (m_pressed)
        setBackgroundColor(AKTheme::SystemFill.resolve(scheme));
    else if (m_hover)
        setBackgroundColor(AKTheme::TertiarySystemFill.resolve(scheme));
    else
        setBackgroundColor(SK_ColorTRANSPARENT);

    setStrokeWidth(m_outlined ? 1 : 0);
    setStrokeColor(m_outlined ? iconColor : SK_ColorTRANSPARENT);
}
