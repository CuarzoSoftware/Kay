#include <AK/effects/AKEdgeShadow.h>
#include <AK/events/AKLayoutEvent.h>
#include <AK/AKTheme.h>

using namespace AK;

AKEdgeShadow::AKEdgeShadow(AKNode *parent) noexcept : AKRenderableImage(parent)
{
    layout().setPositionType(YGPositionTypeAbsolute);
    layout().setWidthPercent(100);
    layout().setHeight(AKTheme::EdgeShadowRadius);
    layout().setPosition(YGEdgeBottom, -AKTheme::EdgeShadowRadius);
    enableCustomTextureColor(true);
    setColorWithAlpha(AKTheme::EdgeShadowColor);
    setImage(theme()->edgeShadowImage(scale()));
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
