#include <include/core/SkCanvas.h>
#include <include/core/SkMatrix.h>
#include <include/core/SkPaint.h>
#include <AK/AKBakeable.h>

using namespace AK;

void AKBakeable::onRender(SkCanvas *canvas, const SkRegion &damage, bool opaque)
{
    if (!t || !t->bake.image)
        return;

    SkPaint paint;
    paint.setBlendMode(opaque ? SkBlendMode::kSrc : SkBlendMode::kSrcOver);
    const SkRect dstRect { SkRect::MakeWH(layout().calculatedWidth(), layout().calculatedHeight()) };

    SkRegion::Iterator it(damage);
    while (!it.done())
    {
        canvas->save();
        canvas->clipIRect(it.rect());
        canvas->drawImageRect(t->bake.image,
                              t->bake.srcRect,
                              dstRect,
                              SkFilterMode::kLinear,
                              &paint,
                              SkCanvas::kStrict_SrcRectConstraint);
        canvas->restore();
        it.next();
    }
}
