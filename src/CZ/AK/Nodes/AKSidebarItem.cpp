#include <CZ/AK/Nodes/AKSidebarItem.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKApp.h>
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

void AKSidebarItem::setIconColor(std::optional<CZAdaptiveColor> color) noexcept
{
    m_iconColor = color;
    updateColors();
}

AKIconButton *AKSidebarItem::setAccessory(const std::string &iconName) noexcept
{
    if (iconName.empty())
    {
        m_accessory.reset();
        return nullptr;
    }

    if (!m_accessory)
    {
        m_accessory = std::make_unique<AKIconButton>(iconName, Int32(TileSize), this);
        m_accessory->layout().setMarginAuto(YGEdgeLeft); // At the trailing end
        m_accessory->onClick.subscribe(this, [this](const auto &){ onAccessoryClicked.notify(); });
    }
    else
        m_accessory->icon.setIconFromName(iconName);

    updateColors();
    return m_accessory.get();
}

bool AKSidebarItem::overAccessory() const noexcept
{
    if (!m_accessory || !m_accessory->visible())
        return false;

    const SkPoint pos { AKApp::Get()->pointer().pos() };
    return SkRect::Make(m_accessory->worldRect()).contains(pos.x(), pos.y());
}

void AKSidebarItem::pointerButtonEvent(const CZPointerButtonEvent &e)
{
    AKRoundSolidColor::pointerButtonEvent(e);

    if (e.button != BTN_LEFT)
        return;

    // The accessory's own click (propagated here)
    if (overAccessory())
    {
        m_pressed = false;
        return;
    }

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
    tile.setIconColor(noTile && emphasized ? std::optional<CZAdaptiveColor>(AKTheme::White) : m_iconColor);

    if (m_accessory)
        m_accessory->setIconColor(emphasized ? AKTheme::White : AKTheme::SecondaryLabel);
}
