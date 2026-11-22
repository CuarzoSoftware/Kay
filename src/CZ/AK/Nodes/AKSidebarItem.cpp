#include <CZ/AK/Nodes/AKSidebarItem.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZPointerLeaveEvent.h>
#include <CZ/Core/Events/CZColorSchemeEvent.h>
#include <CZ/Core/Events/CZWindowStateEvent.h>
#include <linux/input-event-codes.h>

using namespace CZ;

// Selected while the window isn't activated
static constexpr CZAdaptiveColor InactiveSelection { 0x1F000000, 0x29FFFFFF };

AKSidebarItem::AKSidebarItem(const std::string &text, const std::string &iconName, CZAdaptiveColor tileColor, AKNode *parent) noexcept :
    AKRoundSolidColor(0, CZBorderRadius::Make(6), parent),
    m_tileColor(tileColor)
{
    layout().setFlexDirection(YGFlexDirectionRow);
    layout().setAlignItems(YGAlignCenter);
    layout().setHeight(Height);
    layout().setFlexShrink(0.f);
    layout().setPadding(YGEdgeHorizontal, 6.f);
    layout().setGap(YGGutterColumn, 7.f);

    const SkRegion empty;
    tile.icon.setIconFromName(iconName);

    label.setInputRegion(&empty);
    label.setFontSize(FontSize);
    label.setText(text);
    label.layout().setFlexShrink(1.f);

    updateColors();
}

void AKSidebarItem::setSelected(bool selected) noexcept
{
    if (m_selected == selected)
        return;

    m_selected = selected;
    updateColors();
}

void AKSidebarItem::setTileColor(CZAdaptiveColor color) noexcept
{
    m_tileColor = color;
    updateColors();
}

void AKSidebarItem::pointerButtonEvent(const CZPointerButtonEvent &e)
{
    AKRoundSolidColor::pointerButtonEvent(e);

    if (e.button != BTN_LEFT)
        return;

    if (e.pressed)
        m_pressed = true;
    else if (m_pressed)
    {
        m_pressed = false;
        onClicked.notify();
    }
}

void AKSidebarItem::pointerLeaveEvent(const CZPointerLeaveEvent &e)
{
    AKRoundSolidColor::pointerLeaveEvent(e);
    m_pressed = false;
}

void AKSidebarItem::colorSchemeEvent(const CZColorSchemeEvent &e)
{
    AKRoundSolidColor::colorSchemeEvent(e);
    updateColors();
}

void AKSidebarItem::windowStateEvent(const CZWindowStateEvent &e)
{
    AKRoundSolidColor::windowStateEvent(e);
    updateColors();
}

void AKSidebarItem::updateColors() noexcept
{
    const CZColorScheme scheme { colorScheme() };
    const bool emphasized { m_selected && activated() };

    if (!m_selected)
        setBackgroundColor(SK_ColorTRANSPARENT);
    else if (emphasized)
        setBackgroundColor(AKTheme::SelectedContentBackground.resolve(scheme));
    else
        setBackgroundColor(InactiveSelection.resolve(scheme));

    label.setTextColor(emphasized ? AKTheme::White : AKTheme::Label);

    // Without a tile the icon is drawn like the label (white over the selection)
    tile.setTileColor(m_tileColor);
    const bool noTile { SkColorGetA(m_tileColor.resolve(scheme)) == 0 };
    tile.setIconColor(noTile && emphasized ? std::optional<CZAdaptiveColor>(AKTheme::White) : std::nullopt);
}
