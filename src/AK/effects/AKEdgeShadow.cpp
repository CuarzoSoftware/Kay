#include <AK/effects/AKEdgeShadow.h>
#include <AK/events/AKLayoutEvent.h>
#include <AK/AKTheme.h>

using namespace AK;

AKEdgeShadow::AKEdgeShadow(AKEdge edge, AKNode *parent) noexcept : AKRenderableImage(parent)
{
    layout().setPositionType(YGPositionTypeAbsolute);
    setEdge(edge);
    enableCustomTextureColor(true);
    setColorWithAlpha(AKTheme::EdgeShadowColor);
    setImage(theme()->edgeShadowImage(scale()));
}

void AKEdgeShadow::setEdge(AKEdge edge) noexcept
{
    if (edge == m_edge)
        return;

    m_edge = edge;
    addChange(CHEdge);

    switch (edge) {
    case AKEdgeBottom:
        setSrcTransform(AKTransform::Normal);
        layout().setWidthPercent(100);
        layout().setHeight(AKTheme::EdgeShadowRadius);
        layout().setPosition(YGEdgeBottom, -AKTheme::EdgeShadowRadius);
        break;
    case AKEdgeTop:
        setSrcTransform(AKTransform::Rotated180);
        layout().setWidthPercent(100);
        layout().setHeight(AKTheme::EdgeShadowRadius);
        layout().setPosition(YGEdgeTop, -AKTheme::EdgeShadowRadius);
        break;
    case AKEdgeLeft:
        setSrcTransform(AKTransform::Rotated90);
        layout().setHeightPercent(100);
        layout().setWidth(AKTheme::EdgeShadowRadius);
        layout().setPosition(YGEdgeLeft, -AKTheme::EdgeShadowRadius);
        break;
    case AKEdgeRight:
        setSrcTransform(AKTransform::Rotated270);
        layout().setHeightPercent(100);
        layout().setWidth(AKTheme::EdgeShadowRadius);
        layout().setPosition(YGEdgeRight, -AKTheme::EdgeShadowRadius);
        break;
    default:
        break;
    }
}

void AKEdgeShadow::layoutEvent(const AKLayoutEvent &event)
{
    AKRenderableImage::layoutEvent(event);
    if (event.changes().check(AKLayoutEvent::Changes::Scale))
    {
        setImage(theme()->edgeShadowImage(scale()));
        addDamage(AK_IRECT_INF);
    }
    event.accept();
}
