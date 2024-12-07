#include <include/core/SkCanvas.h>
#include <AK/nodes/AKImage.h>
#include <AK/AKPen.h>
#include <AK/AKTarget.h>
#include <AK/AKPainter.h>

using namespace AK;

void AKImage::onRender(AKPainter *painter, const SkRegion &damage)
{
    if (!image())
        return;

    painter->bindTextureMode({
        .texture = image(),
        .pos = { rect().x(), rect().y() },
        .srcRect = srcRect(),
        .dstSize = rect().size(),
        .srcTransform = transform(),
        .srcScale = scale()
    });

    painter->drawRegion(damage);
}

void AKImage::onSceneBegin()
{
    const auto &c { changes() };

    if (c.test(Chg_Opacity) ||
        c.test(Chg_ColorFactor) ||
        c.test(Chg_CustomBlendFuncEnabled) ||
        c.test(Chg_CustomTextureColorEnabled) ||
        (customTextureColorEnabled() && c.test(Chg_Color)) ||
        (customBlendFuncEnabled() && c.test(Chg_CustomBlendFunc)))
        addDamage(AK_IRECT_INF);

    setColorHint(opacity() < 1.f || colorFactor().fA < 1.f ? ColorHint::Translucent : ColorHint::UseOpaqueRegion);
}
