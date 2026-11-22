#include <CZ/AK/Nodes/AKIconTile.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/Core/Events/CZColorSchemeEvent.h>
#include <algorithm>

using namespace CZ;

AKIconTile::AKIconTile(const std::string &iconName, CZAdaptiveColor color, Int32 size, AKNode *parent) noexcept :
    AKRoundSolidColor(0, CZBorderRadius::Make(0), parent),
    m_tileColor(color)
{
    const SkRegion empty;
    setInputRegion(&empty);
    icon.setInputRegion(&empty);
    layout().setJustifyContent(YGJustifyCenter);
    layout().setAlignItems(YGAlignCenter);
    layout().setFlexShrink(0.f);
    icon.setIconFromName(iconName);
    setSize(size);
    updateColors();
}

void AKIconTile::setSize(Int32 size) noexcept
{
    size = std::max(size, 1);

    if (size == m_size)
        return;

    m_size = size;
    layout().setWidth(size);
    layout().setHeight(size);
    setBorderRadius(CZBorderRadius::Make(std::max(1, Int32(std::lround(size * 0.25f)))));
    icon.setSize(UInt32(std::max(1, Int32(std::lround(size * 0.7f)))));
}

void AKIconTile::setTileColor(CZAdaptiveColor color) noexcept
{
    m_tileColor = color;
    updateColors();
}

void AKIconTile::setIconColor(std::optional<CZAdaptiveColor> color) noexcept
{
    m_iconColor = color;
    updateColors();
}

void AKIconTile::colorSchemeEvent(const CZColorSchemeEvent &e)
{
    AKRoundSolidColor::colorSchemeEvent(e);
    updateColors();
}

void AKIconTile::updateColors() noexcept
{
    const CZColorScheme scheme { colorScheme() };
    const SkColor tile { m_tileColor.resolve(scheme) };
    setBackgroundColor(tile);

    if (m_iconColor)
        icon.setColor(m_iconColor->resolve(scheme));
    else
        icon.setColor(SkColorGetA(tile) == 0 ? AKTheme::Label.resolve(scheme) : SK_ColorWHITE);
}
