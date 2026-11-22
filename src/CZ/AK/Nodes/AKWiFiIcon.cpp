#include <CZ/AK/Nodes/AKWiFiIcon.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/Core/Events/CZColorSchemeEvent.h>

using namespace CZ;

AKWiFiIcon::AKWiFiIcon(UInt8 strength, UInt32 size, AKNode *parent) noexcept :
    AKContainer(YGFlexDirectionRow, false, parent),
    fan("wifi", size, this),
    bars("wifi", size, this),
    m_color(AKTheme::Label)
{
    const SkRegion empty;
    setInputRegion(&empty);
    fan.setInputRegion(&empty);
    bars.setInputRegion(&empty);
    layout().setFlexShrink(0.f);

    // Stacked
    bars.layout().setPositionType(YGPositionTypeAbsolute);
    bars.layout().setPosition(YGEdgeLeft, 0.f);
    bars.layout().setPosition(YGEdgeTop, 0.f);

    m_strength = 255; // Forces the update below
    setStrength(strength);
    updateColors();
}

void AKWiFiIcon::setStrength(UInt8 strength) noexcept
{
    if (strength == m_strength)
        return;

    m_strength = strength;
    const int lit { Bars(strength) };
    bars.setIconFromName(lit == 3 ? "wifi" : lit == 2 ? "wifi_2_bar" : "wifi_1_bar");
    fan.setOpacity(lit < 3 ? 1.f : 0.f); // Not hidden: it sizes the icon
}

void AKWiFiIcon::setColor(CZAdaptiveColor color) noexcept
{
    m_color = color;
    updateColors();
}

void AKWiFiIcon::colorSchemeEvent(const CZColorSchemeEvent &e)
{
    AKContainer::colorSchemeEvent(e);
    updateColors();
}

void AKWiFiIcon::updateColors() noexcept
{
    const CZColorScheme scheme { colorScheme() };
    fan.setColor(AKTheme::TertiaryLabel.resolve(scheme));
    bars.setColor(m_color.resolve(scheme));
}
