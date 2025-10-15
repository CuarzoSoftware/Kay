#include <CZ/skia/core/SkCanvas.h>
#include <CZ/skia/effects/SkImageFilters.h>
#include <CZ/skia/effects/SkColorMatrixFilter.h>
#include <CZ/AK/Events/AKRenderEvent.h>
#include <CZ/AK/Effects/AKBackgroundImageShadowEffect.h>
#include <CZ/AK/Nodes/AKBakeable.h>
#include <CZ/AK/AKTarget.h>
#include <CZ/Ream/RSurface.h>
#include <CZ/Ream/RPass.h>
#include <CZ/Ream/RImage.h>
#include <CZ/skia/core/SkPaint.h>

using namespace CZ;

AKBackgroundImageShadowEffect::AKBackgroundImageShadowEffect(SkScalar radius, const SkIPoint &offset, SkColor color, AKBakeable *targetNode) noexcept :
    AKBackgroundEffect(Background),
    m_offset(offset),
    m_radius(radius)
{
    enableReplaceImageColor(true);
    setColor(color);

    if (targetNode)
        targetNode->addBackgroundEffect(this);
}

void AKBackgroundImageShadowEffect::targetNodeRectCalculated()
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
    effectRect = SkIRect::MakeWH(bakeableTarget->worldRect().width(), bakeableTarget->worldRect().height());
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
            m_surface = RSurface::Make(
                surfaceSize,
                bakeableTarget->scale(),
                true);
        }

        auto pass { m_surface->beginPass() };
        SkCanvas &canvas { *pass->getCanvas() };
        SkPaint brush;
        canvas.save();
        canvas.clear(SK_ColorTRANSPARENT);
        brush.setBlendMode(SkBlendMode::kSrc);
        brush.setImageFilter(SkImageFilters::DropShadowOnly(0, 0, m_radius/3.f, m_radius/3.f, SK_ColorBLACK, nullptr));
        canvas.drawImageRect(
            bakeableTarget->surface()->image()->skImage(),
            SkRect::MakeXYWH(0, 0,
                             bakeableTarget->worldRect().width() * bakeableTarget->scale(),
                             bakeableTarget->worldRect().height() * bakeableTarget->scale()),
            SkRect::MakeXYWH(m_radius, m_radius, bakeableTarget->worldRect().width(), bakeableTarget->worldRect().height()),
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

    auto *painter { p.pass->getPainter() };
    RDrawImageInfo info {};
    info.image = m_surface->image();
    info.dst = p.rect;
    info.src = SkRect::MakeWH(p.rect.size().width(), p.rect.size().height());
    info.srcScale = bakeableTarget->scale();

    painter->drawImage(info, &p.damage);
}

void AKBackgroundImageShadowEffect::onTargetNodeChanged() {}
