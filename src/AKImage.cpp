#include <include/core/SkCanvas.h>
#include <AKImage.h>

using namespace AK;

void AKImage::onRender(SkCanvas *canvas, const SkRegion &damage, bool opaque)
{
    if (!m_image)
        return;

    SkPaint paint;
    paint.setBlendMode(opaque ? SkBlendMode::kSrc : SkBlendMode::kSrcOver);
    const SkSamplingOptions samplingOptions { SkFilterMode::kLinear };
    SkMatrix matrix;
    SkRect srcRect, dstRect;
    calculateRenderParams(matrix, srcRect, dstRect);
    canvas->save();
    SkRegion::Iterator it(damage);
    while (!it.done())
    {
        canvas->save();
        canvas->clipIRect(it.rect());
        canvas->concat(matrix);
        canvas->drawImageRect(m_image,
                              srcRect,
                              dstRect,
                              samplingOptions,
                              &paint,
                              SkCanvas::kFast_SrcRectConstraint);
        canvas->restore();
        it.next();
    }
    canvas->restore();
}

void AKImage::calculateRenderParams(SkMatrix &matrix, SkRect &srcRect, SkRect &dstRect) const noexcept
{
    const SkRect src {
        m_srcRect.left() * scale(),
        m_srcRect.top() * scale(),
        m_srcRect.right() * scale(),
        m_srcRect.bottom() * scale()};

    switch (transform())
    {
    case AKTransform::Normal:
        matrix.preTranslate(layoutGetLeft(), layoutGetTop());
        srcRect = src;
        dstRect.setXYWH(0, 0, layoutGetWidth(), layoutGetHeight());
        break;
    case AKTransform::Rotated90:
        matrix.preTranslate(layoutGetLeft() + layoutGetWidth(), layoutGetTop());
        matrix.preRotate(90.f);
        srcRect.setLTRB(
            src.top(),
            image()->height() - src.right(),
            src.bottom(),
            src.right());
        dstRect.setXYWH(0, 0, layoutGetHeight(), layoutGetWidth());
        break;
    case AKTransform::Rotated180:
        matrix.preTranslate(layoutGetLeft() + layoutGetWidth(), layoutGetTop() + layoutGetHeight());
        matrix.preRotate(180.f);
        srcRect.setLTRB(
            image()->width() - src.right(),
            image()->height() - src.bottom(),
            src.right(),
            src.bottom());
        dstRect.setXYWH(0, 0, layoutGetWidth(), layoutGetHeight());
        break;
    case AKTransform::Rotated270:
        matrix.preTranslate(layoutGetLeft(), layoutGetTop() + layoutGetHeight());
        matrix.preRotate(-90.f);
        srcRect.setLTRB(
            src.top(),
            src.left(),
            src.bottom(),
            src.right());
        dstRect.setXYWH(0, 0, layoutGetHeight(), layoutGetWidth());
        break;
    case AKTransform::Flipped:
        matrix.preTranslate(layoutGetLeft() + layoutGetWidth(), layoutGetTop());
        matrix.preScale(-1.f, 1.f);
        srcRect.setLTRB(
            image()->width() - src.right(),
            image()->height() - src.bottom(),
            src.right(),
            src.bottom());
        dstRect.setXYWH(0, 0, layoutGetWidth(), layoutGetHeight());
        break;
    case AKTransform::Flipped90:
        matrix.preTranslate(layoutGetLeft(), layoutGetTop());
        matrix.preScale(-1.f, 1.f);
        matrix.preRotate(90.f);
        srcRect.setLTRB(
            src.top(),
            src.left(),
            src.bottom(),
            src.right());
        dstRect.setXYWH(0, 0, layoutGetHeight(), layoutGetWidth());
        break;
    case AKTransform::Flipped180:
        matrix.preTranslate(layoutGetLeft(), layoutGetTop() + layoutGetHeight());
        matrix.preScale(1.f, -1.f);
        srcRect.setLTRB(
            src.left(),
            image()->height() - src.bottom(),
            src.right(),
            src.bottom());
        dstRect.setXYWH(0, 0, layoutGetWidth(), layoutGetHeight());
        break;
    case AKTransform::Flipped270:
        matrix.preTranslate(layoutGetLeft() + layoutGetWidth(), layoutGetTop() + layoutGetHeight());
        matrix.preScale(-1.f, 1.f);
        matrix.preRotate(-90.f);
        srcRect.setLTRB(
            src.top(),
            src.left(),
            src.bottom(),
            src.right());
        dstRect.setXYWH(0, 0, layoutGetHeight(), layoutGetWidth());
        break;
    }
}

