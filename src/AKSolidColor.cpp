#include <AKSolidColor.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkRegion.h>

void AK::AKSolidColor::onRender(SkCanvas *canvas, const SkRegion &damage, bool opaque)
{
    SkBlendMode blendMode { opaque ? SkBlendMode::kSrc : SkBlendMode::kSrcOver };
    SkRegion::Iterator it(damage);

    while (!it.done())
    {
        canvas->save();
        canvas->clipIRect(it.rect());
        canvas->drawColor(m_color, blendMode);
        canvas->restore();
        it.next();
    }
}
