#include <AK/nodes/AKSolidColor.h>
#include <AK/AKPainter.h>

using namespace AK;

void AKSolidColor::onRender(AKPainter *painter, const SkRegion &damage)
{
    painter->bindColorMode();
    painter->drawRegion(damage);
}

void AKSolidColor::onSceneBegin()
{
    const auto &c { changes() };

    if (c.test(Chg_Color) ||
        c.test(Chg_Opacity) ||
        c.test(Chg_ColorFactor) ||
        c.test(Chg_CustomBlendFuncEnabled) ||
        (customBlendFuncEnabled() && c.test(Chg_CustomBlendFunc)))
        addDamage(AK_IRECT_INF);

    setColorHint(opacity() < 1.f || colorFactor().fA < 1.f ? ColorHint::Translucent : ColorHint::Opaque);
}
