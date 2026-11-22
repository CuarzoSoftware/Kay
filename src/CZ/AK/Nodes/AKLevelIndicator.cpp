#include <CZ/AK/Nodes/AKLevelIndicator.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/Core/Events/CZColorSchemeEvent.h>
#include <algorithm>
#include <cmath>

using namespace CZ;

AKLevelIndicator::AKLevelIndicator(UInt32 count, AKNode *parent) noexcept :
    AKContainer(YGFlexDirectionRow, false, parent),
    m_onColor(AKTheme::ControlAccent)
{
    layout().setAlignItems(YGAlignCenter);
    layout().setGap(YGGutterColumn, Gap);
    layout().setFlexShrink(0.f);
    setCount(count);
}

void AKLevelIndicator::setValue(SkScalar value) noexcept
{
    m_value = std::clamp(value, 0.f, 1.f);
    const UInt32 lit { UInt32(std::lround(m_value * SkScalar(m_segments.size()))) };

    if (lit == m_lit)
        return;

    m_lit = lit;
    updateColors();
}

void AKLevelIndicator::setCount(UInt32 count) noexcept
{
    const SkRegion empty;

    while (m_segments.size() > count)
        m_segments.pop_back();

    while (m_segments.size() < count)
    {
        auto &s { m_segments.emplace_back(std::make_unique<AKRoundSolidColor>(0, CZBorderRadius::Make(Int32(SegmentWidth / 2.f)), this)) };
        s->setInputRegion(&empty);
        s->layout().setWidth(SegmentWidth);
        s->layout().setHeight(SegmentHeight);
        s->layout().setFlexShrink(0.f);
    }

    m_lit = UInt32(std::lround(m_value * SkScalar(m_segments.size())));
    updateColors();
}

void AKLevelIndicator::setOnColor(CZAdaptiveColor color) noexcept { m_onColor = color; updateColors(); }
void AKLevelIndicator::setOffColor(CZAdaptiveColor color) noexcept { m_offColor = color; updateColors(); }

void AKLevelIndicator::colorSchemeEvent(const CZColorSchemeEvent &e)
{
    AKContainer::colorSchemeEvent(e);
    updateColors();
}

void AKLevelIndicator::updateColors() noexcept
{
    const CZColorScheme scheme { colorScheme() };
    const SkColor on { m_onColor.resolve(scheme) }, off { m_offColor.resolve(scheme) };

    for (size_t i = 0; i < m_segments.size(); i++)
        m_segments[i]->setBackgroundColor(i < m_lit ? on : off);
}
