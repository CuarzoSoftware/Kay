#include <CZ/AK/Nodes/AKSegmentedControl.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKApp.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZPointerMoveEvent.h>
#include <CZ/Core/Events/CZPointerLeaveEvent.h>
#include <CZ/Core/Events/CZColorSchemeEvent.h>
#include <linux/input-event-codes.h>

using namespace CZ;

AKSegmentedControl::Segment::Segment(const std::string &text, bool isIcon, AKNode *parent) noexcept :
    bg(0, CZBorderRadius::Make(5), parent),
    label(isIcon ? std::string() : text, &bg)
{
    const SkRegion empty;
    bg.setInputRegion(&empty);
    label.setInputRegion(&empty);
    bg.layout().setFlex(1.f);
    bg.layout().setFlexBasis(0.f);
    bg.layout().setJustifyContent(YGJustifyCenter);
    bg.layout().setAlignItems(YGAlignCenter);
    bg.layout().setPadding(YGEdgeHorizontal, 10.f);
    label.setFontSize(FontSize);

    if (isIcon)
    {
        label.setVisible(false);
        icon = std::make_unique<AKFontIcon>(text, IconSize, &bg);
        icon->setInputRegion(&empty);
    }
}

AKSegmentedControl::AKSegmentedControl(const std::vector<std::string> &items, AKNode *parent) noexcept :
    AKRoundSolidColor(0, CZBorderRadius::Make(7), parent)
{
    layout().setFlexDirection(YGFlexDirectionRow);
    layout().setHeight(Height);
    layout().setPadding(YGEdgeAll, 2.f);
    layout().setGap(YGGutterColumn, 2.f);
    layout().setFlexShrink(0.f);
    setCursor(CZCursorShape::Default);
    setItems(items);
}

void AKSegmentedControl::setItems(const std::vector<std::string> &items, size_t selected) noexcept
{
    m_segments.clear();

    for (const auto &item : items)
        m_segments.emplace_back(std::make_unique<Segment>(item, false, this));

    m_pressed = m_hover = None;
    m_selected = None;
    setSelectedIndex(selected);
}

void AKSegmentedControl::setIconItems(const std::vector<std::string> &iconNames, size_t selected) noexcept
{
    m_segments.clear();

    for (const auto &name : iconNames)
        m_segments.emplace_back(std::make_unique<Segment>(name, true, this));

    m_pressed = m_hover = None;
    m_selected = None;
    setSelectedIndex(selected);
}

void AKSegmentedControl::setSelectedIndex(size_t index) noexcept
{
    m_selected = index < m_segments.size() ? index : None;
    updateColors();
}

void AKSegmentedControl::setEnabled(bool enabled) noexcept
{
    m_enabled = enabled;
    m_pressed = m_hover = None;
    setOpacity(enabled ? 1.f : 0.4f);
    updateColors();
}

size_t AKSegmentedControl::segmentAt(SkScalar x) const noexcept
{
    for (size_t i = 0; i < m_segments.size(); i++)
    {
        const SkIRect r { m_segments[i]->bg.worldRect() };

        // Up to the next one (the gaps belong to the segment on their left)
        if (x < SkScalar(r.right()) + 2.f || i + 1 == m_segments.size())
            return i;
    }

    return None;
}

void AKSegmentedControl::pointerButtonEvent(const CZPointerButtonEvent &e)
{
    AKRoundSolidColor::pointerButtonEvent(e);

    if (!m_enabled || e.button != BTN_LEFT)
        return;

    const size_t at { segmentAt(AKApp::Get()->pointer().pos().x()) };

    if (e.pressed)
        m_pressed = at;
    else
    {
        const bool clicked { m_pressed != None && m_pressed == at && at != m_selected };
        m_pressed = None;

        if (clicked)
        {
            m_selected = at;
            updateColors();
            onSelected.notify(at);
            return;
        }
    }

    updateColors();
}

void AKSegmentedControl::pointerMoveEvent(const CZPointerMoveEvent &e)
{
    AKRoundSolidColor::pointerMoveEvent(e);

    if (!m_enabled)
        return;

    const size_t at { segmentAt(AKApp::Get()->pointer().pos().x()) };

    if (at != m_hover)
    {
        m_hover = at;
        updateColors();
    }
}

void AKSegmentedControl::pointerLeaveEvent(const CZPointerLeaveEvent &e)
{
    AKRoundSolidColor::pointerLeaveEvent(e);
    m_pressed = m_hover = None;
    updateColors();
}

void AKSegmentedControl::colorSchemeEvent(const CZColorSchemeEvent &e)
{
    AKRoundSolidColor::colorSchemeEvent(e);
    updateColors();
}

void AKSegmentedControl::updateColors() noexcept
{
    const CZColorScheme scheme { colorScheme() };
    static const CZAdaptiveColor Track    { 0x14000000, 0x1AFFFFFF };
    static const CZAdaptiveColor Selected { 0xFFFFFFFF, 0x4DFFFFFF };
    static const CZAdaptiveColor Border   { 0x1F000000, 0x14FFFFFF };
    const CZAdaptiveColor &Hover   { AKTheme::ControlHoverFill };
    const CZAdaptiveColor &Pressed { AKTheme::ControlPressedFill };

    setBackgroundColor(Track.resolve(scheme));

    for (size_t i = 0; i < m_segments.size(); i++)
    {
        auto &s { *m_segments[i] };
        s.label.setTextColor(AKTheme::Label);

        if (s.icon)
            s.icon->setColor(AKTheme::Label.resolve(scheme));

        if (i == m_selected)
        {
            s.bg.setBackgroundColor(Selected.resolve(scheme));
            s.bg.setStrokeColor(Border.resolve(scheme));
            s.bg.setStrokeWidth(1);
        }
        else
        {
            s.bg.setBackgroundColor(i == m_pressed ? Pressed.resolve(scheme) : i == m_hover ? Hover.resolve(scheme) : SK_ColorTRANSPARENT);
            s.bg.setStrokeWidth(0);
        }
    }
}
