#include <include/core/SkCanvas.h>
#include <include/effects/SkImageFilters.h>
#include <include/effects/SkColorMatrixFilter.h>
#include <AK/events/AKRenderEvent.h>
#include <AK/effects/AKBackgroundImageShadowEffect.h>
#include <AK/nodes/AKBakeable.h>
#include <AK/AKSceneTarget.h>
#include <AK/AKSurface.h>
#include <AK/AKBrush.h>

using namespace AK;

void AKBackgroundImageShadowEffect::onSceneCalculatedRect()
{
    AKBakeable *bakeableTarget = dynamic_cast<AKBakeable*>(targetNode());

    if (!bakeableTarget || !bakeableTarget->surface())
        return;

    bool needsNewSurface { m_surface == nullptr };
    bool needsFullDamage { needsNewSurface };
    const auto &chg { changes() };

    if (m_prevScale != bakeableTarget->scale())
    {
        needsNewSurface = needsFullDamage = true;
        m_prevScale = bakeableTarget->scale();
    }

    const SkISize prevSize { effectRect.size() };
    effectRect = SkIRect::MakeWH(bakeableTarget->globalRect().width(), bakeableTarget->globalRect().height());
    effectRect.outset(m_radius, m_radius);
    effectRect.offset(offset().x(), offset().y());

    if (prevSize != effectRect.size() || chg.test(CHRadius) || bakeableTarget->onBakeGeneratedDamage())
        needsFullDamage = needsNewSurface = true;

    if (needsNewSurface)
    {
        const SkISize surfaceSize { effectRect.size() };

        if (m_surface)
            m_surface->resize(surfaceSize, bakeableTarget->scale(), true);
        else
        {
            m_surface = AKSurface::Make(
                surfaceSize,
                bakeableTarget->scale(),
                true);
        }

        SkCanvas &canvas { *m_surface->surface()->getCanvas() };
        AKBrush brush;
        canvas.save();
        canvas.scale(
            bakeableTarget->scale(),
            bakeableTarget->scale());
        canvas.clear(SK_ColorTRANSPARENT);
        brush.setBlendMode(SkBlendMode::kSrc);
        brush.setImageFilter(SkImageFilters::DropShadowOnly(0, 0, m_radius/3.f, m_radius/3.f, SK_ColorBLACK, nullptr));
        canvas.drawImageRect(
            bakeableTarget->surface()->image(),
            SkRect::MakeXYWH(0, 0,
                             bakeableTarget->globalRect().width() * bakeableTarget->scale(),
                             bakeableTarget->globalRect().height() * bakeableTarget->scale()),
            SkRect::MakeXYWH(m_radius, m_radius, bakeableTarget->globalRect().width(), bakeableTarget->globalRect().height()),
            SkFilterMode::kLinear,
            &brush,
            SkCanvas::kFast_SrcRectConstraint);
        canvas.restore();
    }

    if (needsFullDamage)
        addDamage(AK_IRECT_INF);
}


void AKBackgroundImageShadowEffect::renderEvent(const AKRenderEvent &p)
{
    AKBakeable *bakeableTarget = dynamic_cast<AKBakeable*>(targetNode());

    if (!bakeableTarget || !bakeableTarget->surface())
        return;

    const SkRect src { SkRect::MakeWH(p.rect.size().width(), p.rect.size().height()) };

    p.painter.bindTextureMode({
        .texture = m_surface->image(),
        .pos = { p.rect.x(), p.rect.y() },
        .srcRect = src,
        .dstSize = p.rect.size(),
        .srcTransform = AKTransform::Normal,
        .srcScale = SkScalar(m_surface->scale())
    });

    p.painter.drawRegion(p.damage);
}

void AKBackgroundImageShadowEffect::onTargetNodeChanged()
{

}
