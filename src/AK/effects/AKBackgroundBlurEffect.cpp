#include "include/core/SkCanvas.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include <include/effects/SkImageFilters.h>
#include <AK/effects/AKBackgroundBlurEffect.h>
#include <AK/AKTarget.h>

using namespace AK;

void AKBackgroundBlurEffect::onLayoutUpdate()
{
    if (!currentTarget()->image())
        return;

    if (clipMode() == Automatic)
        effectRect = SkIRect::MakeSize(targetNode()->rect().size());
    else
        on.targetLayoutUpdated.notify();

    reactiveRegion.setRect(SkIRect::MakeSize(effectRect.size()));

    const auto &chgs { changes() };

    if (chgs.test(Chg_Sigma) || chgs.test(Chg_ClipMode))
        addDamage(AK_IRECT_INF);

    if (!m_brush.getImageFilter() || chgs.test(Chg_Sigma))
        m_brush.setImageFilter(SkImageFilters::Blur(m_sigma.x(), m_sigma.y(), SkTileMode::kMirror, nullptr));
}

void AKBackgroundBlurEffect::onRender(AKPainter *, const SkRegion &damage)
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

    const SkRect dstRect { SkRect::Make(rect()) };

    if (clipMode() == Manual)
    {
        c.translate(dstRect.x(), dstRect.y());
        c.clipPath(clip);
        c.translate(-dstRect.x(), -dstRect.y());
    }

    // TODO: Handle AKTarget srcRect and custom transforms

    const SkRect srcRect { SkRect::MakeXYWH(
        (dstRect.x() - currentTarget()->viewport().x()) * currentTarget()->xyScale().x(),
        (dstRect.y() - currentTarget()->viewport().y()) * currentTarget()->xyScale().y(),
        dstRect.width() * currentTarget()->xyScale().x(),
        dstRect.height() * currentTarget()->xyScale().y()) };

    c.drawImageRect(currentTarget()->image(),
                    srcRect,
                    dstRect,
                    SkFilterMode::kLinear,
                    &m_brush,
                    SkCanvas::kFast_SrcRectConstraint);
    currentTarget()->surface()->flush();
    c.restore();
}
