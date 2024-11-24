#include <include/core/SkCanvas.h>
#include <include/core/SkMatrix.h>
#include <include/core/SkPaint.h>
#include <AKBakeable.h>

using namespace AK;

void AKBakeable::onRender(SkCanvas *canvas, const SkRegion &damage, bool opaque)
{
    if (!t->bake.image)
        return;

    SkPaint paint;
    paint.setBlendMode(opaque ? SkBlendMode::kSrc : SkBlendMode::kSrcOver);
    const SkSamplingOptions samplingOptions { SkFilterMode::kLinear };
    const SkRect dstRect (
        globalRect().fLeft,
        globalRect().fTop,
        globalRect().fRight,
        globalRect().fBottom);

    SkRegion::Iterator it(damage);
    while (!it.done())
    {
        canvas->save();
        canvas->clipIRect(it.rect());
        canvas->drawImageRect(t->bake.image,
                              t->bake.srcRect,
                              dstRect,
                              samplingOptions,
                              &paint,
                              SkCanvas::kFast_SrcRectConstraint);
        canvas->restore();
        it.next();
    }
}
