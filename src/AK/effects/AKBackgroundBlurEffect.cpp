#include <include/core/SkCanvas.h>
#include <include/effects/SkImageFilters.h>
#include <include/gpu/ganesh/GrDirectContext.h>
#include <AK/effects/AKBackgroundBlurEffect.h>
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

    if (chgs.test(Chg_Sigma) || chgs.test(Chg_ClipMode))
        addDamage(AK_IRECT_INF);

    if (!m_brush.getImageFilter() || chgs.test(Chg_Sigma))
        m_brush.setImageFilter(SkImageFilters::Blur(m_sigma.x(), m_sigma.y(), SkTileMode::kMirror, nullptr));
}

void AKBackgroundBlurEffect::onRender(AKPainter *painter, const SkRegion &damage, const SkIRect &rect)
{
    if (!currentTarget()->image() || damage.isEmpty())
        return;

    currentTarget()->surface()->recordingContext()->asDirectContext()->resetContext();

    SkCanvas &c { *currentTarget()->surface()->getCanvas() };
    c.save();

    SkPath path;
    path.setIsVolatile(true);
    damage.getBoundaryPath(&path);
    c.clipPath(path);

    const SkRect dstRect { SkRect::Make(rect) };

    if (clipMode() == Manual)
    {
        c.translate(dstRect.x(), dstRect.y());
        c.clipPath(clip);
        c.translate(-dstRect.x(), -dstRect.y());
    }

    // TODO: Handle AKTarget srcRect and custom transforms

    const SkRect srcRect { SkRect::MakeXYWH(
        (dstRect.x() - currentTarget()->viewport().x()) * currentTarget()->bakedComponentsScale(),
        (dstRect.y() - currentTarget()->viewport().y()) * currentTarget()->bakedComponentsScale(),
        dstRect.width() * currentTarget()->bakedComponentsScale(),
        dstRect.height() * currentTarget()->bakedComponentsScale()) };

    c.drawImageRect(currentTarget()->image(),
                    srcRect,
                    dstRect,
                    SkFilterMode::kLinear,
                    &m_brush,
                    SkCanvas::kFast_SrcRectConstraint);
    currentTarget()->surface()->recordingContext()->asDirectContext()->flush();
    c.restore();
    painter->bindProgram();
    painter->bindTarget(currentTarget());
}
