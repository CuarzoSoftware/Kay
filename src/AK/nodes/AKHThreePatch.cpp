#include <AK/nodes/AKHThreePatch.h>

using namespace AK;

void AKHThreePatch::onRender(AKPainter *painter, const SkRegion &damage)
{
    if (!m_image || m_scale <= 0)
        return;

    constexpr static AKTransform transforms[] { AKTransform::Normal, AKTransform::Normal, AKTransform::Flipped };
    const SkRect srcs[] { m_sideSrcRect, m_centerSrcRect, m_sideSrcRect };
    SkIRect dsts[3];
    const SkScalar centerWidth { std::max(1.f, SkScalar(rect().width()) - 2.f * m_sideSrcRect.width()) };
    dsts[0].setXYWH(rect().x(), rect().y(), m_sideSrcRect.width(), rect().height());
    dsts[1].setXYWH(dsts[0].fRight, rect().y(), centerWidth, dsts[0].height());
    dsts[2].setXYWH(dsts[1].fRight, rect().y(), m_sideSrcRect.width(), rect().height());
    SkRegion regions[3] { damage, damage, damage };

    for (Int32 i = 0; i < 3; i++)
    {
        regions[i].op(dsts[i], SkRegion::Op::kIntersect_Op);

        if (regions[i].isEmpty())
            continue;

        painter->bindTextureMode({
            .texture = m_image,
            .pos = dsts[i].topLeft(),
            .srcRect = srcs[i],
            .dstSize = dsts[i].size(),
            .srcTransform = transforms[i],
            .srcScale = m_scale
        });

        painter->drawRegion(regions[i]);
    }
}
