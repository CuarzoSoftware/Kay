#include "include/core/SkCanvas.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include <include/effects/SkImageFilters.h>
#include <AK/effects/AKBackgroundBlurEffect.h>
#include <AK/AKTarget.h>

using namespace AK;

static const float BLUR = 16.f;

void AKBackgroundBlurEffect::onLayoutUpdate()
{
    if (!currentTarget()->image)
        return;

    //rect = SkIRect::MakeSize(targetNode()->rect().size());
    onLayoutUpdateSignal.notify();
    reactiveRegion.setRect(SkIRect::MakeSize(rect.size()));
}

void AKBackgroundBlurEffect::onRender(AKPainter *, const SkRegion &damage)
{
    if (!currentTarget()->image)
        return;

    currentTarget()->surface->recordingContext()->asDirectContext()->resetContext();

    SkCanvas &c { *currentTarget()->surface->getCanvas() };
    c.save();

    SkPath path;
    damage.getBoundaryPath(&path);

    c.clipIRect(AKNode::rect());

    SkRect dstRect = SkRect::Make(AKNode::rect());

    c.translate(dstRect.x(), dstRect.y());
    c.clipPath(clip);
    c.translate(-dstRect.x(), -dstRect.y());

    if (!brush.getImageFilter())
        brush.setImageFilter(SkImageFilters::Blur(BLUR, BLUR, SkTileMode::kMirror, nullptr));

    SkRect srcRect = SkRect::MakeXYWH(
        (dstRect.x() -currentTarget()->viewport.x()) * currentTarget()->xyScale().x(),
        (dstRect.y()- currentTarget()->viewport.y()) * currentTarget()->xyScale().y(),
         dstRect.width() * currentTarget()->xyScale().x(),
         dstRect.height() * currentTarget()->xyScale().y());

    c.drawImageRect(currentTarget()->image,
                    srcRect,
                    dstRect,
                    SkFilterMode::kLinear,
                    &brush,
                    SkCanvas::SrcRectConstraint::kFast_SrcRectConstraint);
    currentTarget()->surface->flush();
    c.restore();
}
