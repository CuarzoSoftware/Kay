#include <CZ/AK/Effects/AKEdgeShadow.h>
#include <CZ/Core/Events/CZLayoutEvent.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/Nodes/AKScroll.h>
#include <algorithm>

using namespace CZ;

AKEdgeShadow::AKEdgeShadow(CZEdge edge, AKNode *parent) noexcept :
    AKImage(parent),
    m_fade(FadeMs,
        [this](CZAnimation *a){ setLevel(m_fadeFrom + (m_fadeTo - m_fadeFrom) * SkScalar(a->value())); },
        [this](CZAnimation *){ setLevel(m_fadeTo); setVisible(m_fadeTo > 0.f); })
{
    const SkRegion empty;
    setInputRegion(&empty);
    layout().setPositionType(YGPositionTypeAbsolute);
    setEdge(edge);
    enableReplaceImageColor(true);
    setColor(AKTheme::EdgeShadowColor);
    setImage(theme()->edgeShadowImage(scale()));
}

AKEdgeShadow::~AKEdgeShadow() noexcept
{
    // The listener goes away with this object (its owner)
    m_fade.pause();
}

void AKEdgeShadow::showWhileScrolled(AKScroll *scroll) noexcept
{
    if (m_scroll.get() == scroll)
        return;

    // Gone with its scroll if that was destroyed
    if (m_scroll)
        delete m_scrollListener;

    m_scrollListener = nullptr;
    m_scroll.reset(scroll);

    if (scroll)
        m_scrollListener = scroll->onOffsetChanged.subscribe(this, [this]{ updateScrolled(true); });

    updateScrolled(false);
}

void AKEdgeShadow::updateScrolled(bool animated) noexcept
{
    const SkScalar target { !m_scroll || m_scroll->offsetY() < 0.f ? 1.f : 0.f };

    // Already there or on its way
    if (target == m_fadeTo && (m_fade.isRunning() || m_level == target))
        return;

    m_fadeTo = target;

    if (!animated)
    {
        m_fade.pause();
        setLevel(target);
        setVisible(target > 0.f);
        return;
    }

    m_fadeFrom = visible() ? m_level : 0.f;
    setVisible(true);
    m_fade.pause();
    m_fade.start();
}

void AKEdgeShadow::setStrength(SkScalar strength) noexcept
{
    m_strength = std::clamp(strength, 0.f, 1.f);
    setLevel(m_level);
}

void AKEdgeShadow::setLevel(SkScalar level) noexcept
{
    m_level = level;
    setOpacity(m_level * m_strength);
}

void AKEdgeShadow::setEdge(CZEdge edge) noexcept
{
    if (edge == m_edge)
        return;

    m_edge = edge;
    addChange(CHEdge);

    switch (edge) {
    case CZEdgeBottom:
        setSrcTransform(CZTransform::Normal);
        layout().setWidthPercent(100);
        layout().setHeight(AKTheme::EdgeShadowRadius);
        layout().setPosition(YGEdgeBottom, -AKTheme::EdgeShadowRadius);
        break;
    case CZEdgeTop:
        setSrcTransform(CZTransform::Rotated180);
        layout().setWidthPercent(100);
        layout().setHeight(AKTheme::EdgeShadowRadius);
        layout().setPosition(YGEdgeTop, -AKTheme::EdgeShadowRadius);
        break;
    case CZEdgeLeft:
        setSrcTransform(CZTransform::Rotated90);
        layout().setHeightPercent(100);
        layout().setWidth(AKTheme::EdgeShadowRadius);
        layout().setPosition(YGEdgeLeft, -AKTheme::EdgeShadowRadius);
        break;
    case CZEdgeRight:
        setSrcTransform(CZTransform::Rotated270);
        layout().setHeightPercent(100);
        layout().setWidth(AKTheme::EdgeShadowRadius);
        layout().setPosition(YGEdgeRight, -AKTheme::EdgeShadowRadius);
        break;
    default:
        break;
    }
}

void AKEdgeShadow::layoutEvent(const CZLayoutEvent &event)
{
    AKImage::layoutEvent(event);
    if (event.changes.has(CZLayoutChangeScale))
    {
        setImage(theme()->edgeShadowImage(scale()));
        addDamage(AK_IRECT_INF);
    }
    event.accept();
}
