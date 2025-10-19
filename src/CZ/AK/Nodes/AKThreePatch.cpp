#include <CZ/Ream/RPass.h>
#include <CZ/AK/Nodes/AKThreePatch.h>
#include <CZ/AK/Events/AKRenderEvent.h>
#include <CZ/AK/AKLog.h>

using namespace CZ;

void AKThreePatch::renderEvent(const AKRenderEvent &e)
{
    if (!m_image || m_imageScale <= 0)
        return;

    const SkIRect &rect { e.rect };
    auto *painter { e.pass->getPainter() };
    SkIRect dsts[3];
    RDrawImageInfo info {};

    if (orientation() == CZOrientation::H)
    {
        const SkRect srcs[] { m_sideSrcRect, m_centerSrcRect, m_sideSrcRect.makeOffset(m_centerSrcRect.width(), 0.f) };
        constexpr static CZTransform transforms[] { CZTransform::Normal, CZTransform::Normal, CZTransform::Flipped };

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
            info.image = m_image;
            info.dst = dsts[i];
            info.src = srcs[i];
            info.srcScale = m_imageScale;
            info.srcTransform = transforms[i];
            painter->drawImage(info, &e.damage);
        }
    }
    else
    {
        SkRect srcs[] { m_sideSrcRect, m_centerSrcRect, m_sideSrcRect.makeOffset(0.f, -m_centerSrcRect.height()) };
        srcs[2].offset(0, m_centerSrcRect.height());
        constexpr static CZTransform transforms[] { CZTransform::Normal, CZTransform::Normal, CZTransform::Flipped180 };

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
            info.image = m_image;
            info.dst = dsts[i];
            info.src = srcs[i];
            info.srcScale = m_imageScale;
            info.srcTransform = transforms[i];
            painter->drawImage(info, &e.damage);
        }
    }
}
