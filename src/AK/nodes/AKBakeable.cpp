#include <include/core/SkCanvas.h>
#include <include/core/SkMatrix.h>
#include <include/core/SkPaint.h>
#include <AK/nodes/AKBakeable.h>
#include <AK/AKSurface.h>

using namespace AK;

void AKBakeable::onRender(SkCanvas *canvas, const SkRegion &damage, bool opaque)
{
    if (!t || !t->bake->image())
        return;

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setBlendMode(opaque ? SkBlendMode::kSrc : SkBlendMode::kSrcOver);
    const SkRect dstRect { SkRect::MakeWH(t->bake->size().width(), t->bake->size().height()) };

    SkRegion::Iterator it(damage);
    while (!it.done())
    {
        canvas->save();
        canvas->clipIRect(it.rect());
        canvas->drawImageRect(t->bake->image(),
                              SkRect::Make(t->bake->imageSrcRect()),
                              dstRect,
                              SkFilterMode::kLinear,
                              &paint,
                              SkCanvas::kStrict_SrcRectConstraint);
        canvas->restore();
        it.next();
    }
}
