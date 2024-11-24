#include <AKDiv.h>

#include <include/core/SkCanvas.h>
#include <include/core/SkRRect.h>
#include <include/effects/SkImageFilters.h>

using namespace AK;

void AKDiv::onBake(SkCanvas *canvas)
{
    canvas->clear(SK_ColorTRANSPARENT);
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorWHITE);

    const SkRect rect = SkRect::MakeXYWH(
        0, 0,
        globalRect().width(),
        globalRect().height());

    canvas->drawRoundRect(
        rect, 10.f, 10.f, paint);

    paint.setStroke(true);
    paint.setColor(SkColorSetARGB(128, 0, 0, 0));
    paint.setAntiAlias(true);
    paint.setStrokeWidth(0.25f);

    const SkRect borderRect = SkRect::MakeXYWH(
        paint.getStrokeWidth(),
        paint.getStrokeWidth(),
        globalRect().width() - paint.getStrokeWidth() * 2.f,
        globalRect().height() - paint.getStrokeWidth() * 2.f);

    canvas->drawRoundRect(
        borderRect, 10.f, 10.f, paint);

    SkRegion region;
    region.op(
        SkIRect::MakeXYWH(0, 0, globalRect().width(), globalRect().height()),
        SkRegion::Op::kUnion_Op);
    addDamage(region);

    region.setEmpty();

    region.op(
        SkIRect::MakeXYWH(1, 1, globalRect().width() - 2, globalRect().height() - 2),
        SkRegion::Op::kUnion_Op);

    // TL
    region.op(SkIRect::MakeXYWH(0, 0, 10.f, 10.f), SkRegion::Op::kDifference_Op);

    // TR
    region.op(SkIRect::MakeXYWH(globalRect().width() - 10.f, 0, 10.f, 10.f), SkRegion::Op::kDifference_Op);

    // BL
    region.op(SkIRect::MakeXYWH(0, globalRect().height() - 10.f, 10.f, 10.f), SkRegion::Op::kDifference_Op);

    // BR
    region.op(SkIRect::MakeXYWH(globalRect().width() - 10.f, globalRect().height() - 10.f, 10.f, 10.f), SkRegion::Op::kDifference_Op);
    setOpaqueRegion(&region);
}
