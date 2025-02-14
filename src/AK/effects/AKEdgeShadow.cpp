#include <AK/effects/AKEdgeShadow.h>
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

    signalLayoutChanged.subscribe(this, [this](auto changes){
        if (changes.check(LayoutChanges::Scale))
        {
            setImage(theme()->edgeShadowImage(scale()));
            addDamage(AK_IRECT_INF);
        }
    });
}
