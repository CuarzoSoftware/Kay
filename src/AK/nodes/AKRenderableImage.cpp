#include <AK/nodes/AKRenderableImage.h>

using namespace AK;

void AKRenderableImage::onSceneBegin()
{
    if (!m_autoDamage)
        return;

    const auto &chg { changes() };

    if (chg.testAnyOf(CHSrcTransform, CHSrcRectMode) ||
        (srcRectMode() == SrcRectMode::Custom && (chg.testAnyOf(CHCustomSrcRect, CHCustomSrcRectScale))))
        addDamage(AK_IRECT_INF);
}

void AKRenderableImage::onRender(const OnRenderParams &p)
{
    if (p.damage.isEmpty() || !image() || image()->width() <= 0 || image()->height() <= 0)
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

    params.pos = p.rect.topLeft();
    params.dstSize = p.rect.size();
    params.texture = image();
    params.srcTransform = srcTransform();
    p.painter.bindTextureMode(params);
    p.painter.drawRegion(p.damage);
}
