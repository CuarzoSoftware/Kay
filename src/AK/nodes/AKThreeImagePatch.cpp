#include <AK/nodes/AKThreeImagePatch.h>
#include <AK/events/AKRenderEvent.h>
#include <AK/AKLog.h>
#include <AK/AKTime.h>

using namespace AK;

void AKThreeImagePatch::renderEvent(const AKRenderEvent &params)
{
    if (!m_image || m_imageScale <= 0)
        return;

    const SkIRect &rect { params.rect };
    const SkRegion &damage { params.damage };
    AKPainter &painter { params.painter };
    SkIRect dsts[3];
    SkRegion region;

    if (orientation() == AKHorizontal)
    {
        const SkRect srcs[] { m_sideSrcRect, m_centerSrcRect, m_sideSrcRect.makeOffset(m_centerSrcRect.width(), 0.f) };
        constexpr static AKTransform transforms[] { AKTransform::Normal, AKTransform::Normal, AKTransform::Flipped };

        if (keepSidesAspectRatio())
        {
            const SkScalar scaledWidth { m_sideSrcRect.height() == 0.f ? 0.f : (rect.height() * m_sideSrcRect.width())/m_sideSrcRect.height() };
            dsts[0].setXYWH(rect.x(), rect.y(), scaledWidth, rect.height());
            dsts[1].setXYWH(dsts[0].fRight, rect.y(), rect.width() - 2.f * scaledWidth, dsts[0].height());
            dsts[2].setXYWH(dsts[1].fRight, rect.y(), scaledWidth, rect.height());
        }
        else
        {
            const SkScalar centerWidth { std::max(1.f, SkScalar(rect.width()) - 2.f * m_sideSrcRect.width()) };
            dsts[0].setXYWH(rect.x(), rect.y(), m_sideSrcRect.width(), rect.height());
            dsts[1].setXYWH(dsts[0].fRight, rect.y(), centerWidth, dsts[0].height());
            dsts[2].setXYWH(dsts[1].fRight, rect.y(), m_sideSrcRect.width(), rect.height());
        }

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
        SkRect srcs[] { m_sideSrcRect, m_centerSrcRect, m_sideSrcRect.makeOffset(0.f, -m_centerSrcRect.height()) };
        srcs[2].offset(0, m_centerSrcRect.height());
        constexpr static AKTransform transforms[] { AKTransform::Normal, AKTransform::Normal, AKTransform::Flipped180 };

        if (keepSidesAspectRatio())
        {
            const SkScalar scaledHeight { m_sideSrcRect.width() == 0.f ? 0.f : (rect.width() * m_sideSrcRect.height())/m_sideSrcRect.width() };
            dsts[0].setXYWH(rect.x(), rect.y(), rect.width(), scaledHeight);
            dsts[1].setXYWH(rect.x(), dsts[0].fBottom, rect.width(), rect.height() - 2.f * scaledHeight);
            dsts[2].setXYWH(rect.x(), dsts[1].fBottom, rect.width(), scaledHeight);
        }
        else
        {
            const SkScalar centerHeight { std::max(1.f, SkScalar(rect.height()) - 2.f * m_sideSrcRect.height()) };
            dsts[0].setXYWH(rect.x(), rect.y(), rect.width(), m_sideSrcRect.height());
            dsts[1].setXYWH(rect.x(), dsts[0].fBottom, rect.width(), centerHeight);
            dsts[2].setXYWH(rect.x(), dsts[1].fBottom, rect.width(), m_sideSrcRect.height());
        }

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
