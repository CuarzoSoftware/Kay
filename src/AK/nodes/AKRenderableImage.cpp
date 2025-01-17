#include <AK/nodes/AKRenderableImage.h>

using namespace AK;

void AKRenderableImage::onSceneBegin()
{
    if (!m_autoDamage)
        return;

    const auto &chg { changes() };

    if (chg.test(Chg_Image) ||
        chg.test(Chg_SrcTransform) ||
        chg.test(Chg_SrcRectMode) ||
        (srcRectMode() == SrcRectMode::Custom && (chg.test(Chg_CustomSrcRect) || chg.test(Chg_CustomSrcRectScale))))
        addDamage(AK_IRECT_INF);
}

void AKRenderableImage::onRender(AKPainter *painter, const SkRegion &damage)
{
    if (damage.isEmpty() || !image() || image()->width() <= 0 || image()->height() <= 0)
        return;

    AKPainter::TextureParams params;

    if (srcRectMode() == SrcRectMode::Custom)
    {
        if (customSrcRect().isEmpty())
            return;

        params.srcRect = customSrcRect();
        params.srcScale = customSrcRectScale();
    }
    else
    {
        params.srcScale = 1.f;

        if (AK::is90Transform(srcTransform()))
            params.srcRect.setWH(image()->height(), image()->width());
        else
            params.srcRect.setWH(image()->width(), image()->height());
    }

    params.pos = rect().topLeft();
    params.dstSize = rect().size();
    params.texture = image();
    params.srcTransform = srcTransform();
    painter->bindTextureMode(params);
    painter->drawRegion(damage);
}
