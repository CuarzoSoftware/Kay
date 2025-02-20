#include <include/core/SkCanvas.h>
#include <include/effects/SkImageFilters.h>
#include <include/gpu/ganesh/GrDirectContext.h>
#include <AK/effects/AKBackgroundBlurEffect.h>
#include <AK/events/AKRenderEvent.h>
#include <AK/AKTarget.h>
#include <AK/AKLog.h>

using namespace AK;

void AKBackgroundBlurEffect::onSceneCalculatedRect()
{
    if (!currentTarget()->image())
        return;

    if (clipMode() == Automatic)
        effectRect = SkIRect::MakeSize(targetNode()->globalRect().size());
    else
        on.targetLayoutUpdated.notify();

    reactiveRegion.setRect(SkIRect::MakeSize(effectRect.size()));

    const auto &chgs { changes() };

    if (chgs.testAnyOf(CHSigma, CHClipMode))
        addDamage(AK_IRECT_INF);

    if (!m_brush.getImageFilter() || chgs.test(CHSigma))
        m_brush.setImageFilter(SkImageFilters::Blur(m_sigma.x(), m_sigma.y(), SkTileMode::kMirror, nullptr));
}

void AKBackgroundBlurEffect::renderEvent(const AKRenderEvent &p)
{
    if (!p.target.image() || p.damage.isEmpty())
        return;

    p.target.surface()->recordingContext()->asDirectContext()->resetContext();

    SkCanvas &c { *p.target.surface()->getCanvas() };
    c.save();

    SkPath path;
    path.setIsVolatile(true);
    p.damage.getBoundaryPath(&path);
    c.clipPath(path);

    const SkRect dstRect { SkRect::Make(p.rect) };

    if (clipMode() == Manual)
    {
        c.translate(dstRect.x(), dstRect.y());
        c.clipPath(clip);
        c.translate(-dstRect.x(), -dstRect.y());
    }

    // TODO: Handle AKTarget srcRect and custom transforms

    const SkRect srcRect { SkRect::MakeXYWH(
        (dstRect.x() - p.target.viewport().x()) * p.target.bakedComponentsScale(),
        (dstRect.y() - p.target.viewport().y()) * p.target.bakedComponentsScale(),
        dstRect.width() * p.target.bakedComponentsScale(),
        dstRect.height() * p.target.bakedComponentsScale()) };

    c.drawImageRect(p.target.image(),
                    srcRect,
                    dstRect,
                    SkFilterMode::kLinear,
                    &m_brush,
                    SkCanvas::kFast_SrcRectConstraint);
    p.target.surface()->recordingContext()->asDirectContext()->flush();
    c.restore();
    p.painter.bindProgram();
    p.painter.bindTarget(&p.target);
}
