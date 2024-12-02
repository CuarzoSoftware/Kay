#include <include/core/SkCanvas.h>
#include <AK/nodes/AKImage.h>
#include <AK/AKPen.h>

using namespace AK;

void AKImage::onRender(SkCanvas *canvas, const SkRegion &damage, bool opaque)
{
    if (!image())
        return;

    p_paint.setBlendMode(opaque ? SkBlendMode::kSrc : SkBlendMode::kSrcOver);
    p_paint.setAntiAlias(true);
    updateImageMatrix();
    SkRegion::Iterator it(damage);
    while (!it.done())
    {
        canvas->save();
        canvas->clipIRect(it.rect());
        canvas->concat(p_imageMatrix);
        canvas->drawImageRect(u_image,
                              p_imageSrcRect,
                              p_imageDstRect,
                              SkFilterMode::kLinear,
                              &p_paint,
                              SkCanvas::kStrict_SrcRectConstraint);
        canvas->restore();
        it.next();
    }
}

void AKImage::updateImageMatrix() noexcept
{
    const SkRect rect { SkRect::MakeWH(layout().calculatedWidth(), layout().calculatedHeight()) };

    p_imageMatrix.setIdentity();

    const SkRect src {
        u_imageSrcRect.left() * imageScale(),
        u_imageSrcRect.top() * imageScale(),
        u_imageSrcRect.right() * imageScale(),
        u_imageSrcRect.bottom() * imageScale() };

    switch (imageTransform())
    {
    case AKTransform::Normal:
        p_imageMatrix.preTranslate(rect.x(), rect.y());
        p_imageSrcRect = src;
        p_imageDstRect.setXYWH(0, 0, rect.width(), rect.height());
        break;
    case AKTransform::Rotated90:
        p_imageMatrix.preTranslate(rect.x() + rect.width(), rect.y());
        p_imageMatrix.preRotate(90.f);
        p_imageSrcRect.setLTRB(
            src.top(),
            image()->height() - src.right(),
            src.bottom(),
            src.right());
        p_imageDstRect.setXYWH(0, 0, rect.height(), rect.width());
        break;
    case AKTransform::Rotated180:
        p_imageMatrix.preTranslate(rect.x() + rect.width(), rect.y() + rect.height());
        p_imageMatrix.preRotate(180.f);
        p_imageSrcRect.setLTRB(
            image()->width() - src.right(),
            image()->height() - src.bottom(),
            src.right(),
            src.bottom());
        p_imageDstRect.setXYWH(0, 0, rect.width(), rect.height());
        break;
    case AKTransform::Rotated270:
        p_imageMatrix.preTranslate(rect.x(), rect.y() + rect.height());
        p_imageMatrix.preRotate(-90.f);
        p_imageSrcRect.setLTRB(
            src.top(),
            src.left(),
            src.bottom(),
            src.right());
        p_imageDstRect.setXYWH(0, 0, rect.height(), rect.width());
        break;
    case AKTransform::Flipped:
        p_imageMatrix.preTranslate(rect.x() + rect.width(), rect.y());
        p_imageMatrix.preScale(-1.f, 1.f);
        p_imageSrcRect.setLTRB(
            image()->width() - src.right(),
            image()->height() - src.bottom(),
            src.right(),
            src.bottom());
        p_imageDstRect.setXYWH(0, 0, rect.width(), rect.height());
        break;
    case AKTransform::Flipped90:
        p_imageMatrix.preTranslate(rect.x(), rect.y());
        p_imageMatrix.preScale(-1.f, 1.f);
        p_imageMatrix.preRotate(90.f);
        p_imageSrcRect.setLTRB(
            src.top(),
            src.left(),
            src.bottom(),
            src.right());
        p_imageDstRect.setXYWH(0, 0, rect.height(), rect.width());
        break;
    case AKTransform::Flipped180:
        p_imageMatrix.preTranslate(rect.x(), rect.y() + rect.height());
        p_imageMatrix.preScale(1.f, -1.f);
        p_imageSrcRect.setLTRB(
            src.left(),
            image()->height() - src.bottom(),
            src.right(),
            src.bottom());
        p_imageDstRect.setXYWH(0, 0, rect.width(), rect.height());
        break;
    case AKTransform::Flipped270:
        p_imageMatrix.preTranslate(rect.x() + rect.width(), rect.y() + rect.height());
        p_imageMatrix.preScale(-1.f, 1.f);
        p_imageMatrix.preRotate(-90.f);
        p_imageSrcRect.setLTRB(
            src.top(),
            src.left(),
            src.bottom(),
            src.right());
        p_imageDstRect.setXYWH(0, 0, rect.height(), rect.width());
        break;
    }
}

