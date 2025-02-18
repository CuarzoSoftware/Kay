#include <AK/nodes/AKThreeImagePatch.h>
#include <AK/AKLog.h>
#include <AK/AKTime.h>

using namespace AK;

void AKThreeImagePatch::onRender(const OnRenderParams &params)
{
    if (!m_image || m_imageScale <= 0)
        return;

    const SkIRect &rect { params.rect };
    const SkRegion &damage { params.damage };
    AKPainter &painter { params.painter };
    SkIRect dsts[3];
    SkRegion region;

    if (orientation() == Horizontal)
    {
        const SkRect srcs[] { m_sideSrcRect, m_centerSrcRect, m_sideSrcRect };
        constexpr static AKTransform transforms[] { AKTransform::Normal, AKTransform::Normal, AKTransform::Flipped };
        const SkScalar centerWidth { std::max(1.f, SkScalar(rect.width()) - 2.f * m_sideSrcRect.width()) };
        dsts[0].setXYWH(rect.x(), rect.y(), m_sideSrcRect.width(), rect.height());
        dsts[1].setXYWH(dsts[0].fRight, rect.y(), centerWidth, dsts[0].height());
        dsts[2].setXYWH(dsts[1].fRight, rect.y(), m_sideSrcRect.width(), rect.height());

        for (Int32 i = 0; i < 3; i++)
        {
            region = damage;
            region.op(dsts[i], SkRegion::Op::kIntersect_Op);

            if (region.isEmpty())
                continue;

            painter.bindTextureMode({
                .texture = m_image,
                .pos = dsts[i].topLeft(),
                .srcRect = srcs[i],
                .dstSize = dsts[i].size(),
                .srcTransform = transforms[i],
                .srcScale = m_imageScale
            });

            painter.drawRegion(region);
        }
    }
    else
    {
        SkRect srcs[] { m_sideSrcRect, m_centerSrcRect, m_sideSrcRect };
        srcs[2].offset(0, m_centerSrcRect.height());
        constexpr static AKTransform transforms[] { AKTransform::Normal, AKTransform::Normal, AKTransform::Flipped180 };
        const SkScalar centerHeight { std::max(1.f, SkScalar(rect.height()) - 2.f * m_sideSrcRect.height()) };
        dsts[0].setXYWH(rect.x(), rect.y(), rect.width(), m_sideSrcRect.height());
        dsts[1].setXYWH(rect.x(), dsts[0].fBottom, rect.width(), centerHeight);
        dsts[2].setXYWH(rect.x(), dsts[1].fBottom, rect.width(), m_sideSrcRect.height());

        for (Int32 i = 0; i < 3; i++)
        {
            region = damage;
            region.op(dsts[i], SkRegion::Op::kIntersect_Op);

            if (region.isEmpty())
                continue;

            painter.bindTextureMode({
                .texture = m_image,
                .pos = dsts[i].topLeft(),
                .srcRect = srcs[i],
                .dstSize = dsts[i].size(),
                .srcTransform = transforms[i],
                .srcScale = m_imageScale
            });

            painter.drawRegion(region);
        }
    }
}
