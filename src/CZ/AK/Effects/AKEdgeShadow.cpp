#include <CZ/AK/Effects/AKEdgeShadow.h>
#include <CZ/Events/CZLayoutEvent.h>
#include <CZ/AK/AKTheme.h>

using namespace CZ;

AKEdgeShadow::AKEdgeShadow(CZEdge edge, AKNode *parent) noexcept : AKImage(parent)
{
    layout().setPositionType(YGPositionTypeAbsolute);
    setEdge(edge);
    enableReplaceImageColor(true);
    setColor(AKTheme::EdgeShadowColor);
    setImage(theme()->edgeShadowImage(scale()));
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
