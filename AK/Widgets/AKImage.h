#ifndef AKImage_H
#define AKImage_H

#include <AK/AKRenderable.h>
#include <AK/AKTransform.h>

#include <include/core/SkImage.h>
#include <include/core/SkRect.h>
#include <include/core/SkPaint.h>

class AK::AKImage : public AKRenderable
{
public:
    AKImage(AKNode *parent = nullptr) noexcept : AKRenderable(parent) {}
    AKImage(sk_sp<SkImage> image, AKNode *parent = nullptr) noexcept : AKRenderable(parent), u_image(image) {}

    void setImage(sk_sp<SkImage> image) noexcept
    {
        u_image = image;
    }

    sk_sp<SkImage> image() const noexcept
    {
        return u_image;
    }

    void setImageSrcRect(const SkRect &rect) noexcept
    {
        u_imageSrcRect = rect;
    }

    const SkRect &imageSrcRect() const noexcept
    {
        return u_imageSrcRect;
    }

    void setImageTransform(AKTransform transform) noexcept
    {
        u_imageTransform = transform;
    }

    AKTransform imageTransform() const noexcept
    {
        return u_imageTransform;
    }

    void setImageScale(SkScalar scale) noexcept
    {
        if (scale <= 0.f)
            return;

        u_imageScale = scale;
    }

    SkScalar imageScale() const noexcept
    {
        return u_imageScale;
    }

protected:
    virtual void onRender(SkCanvas *canvas, const SkRegion &damage, bool opaque) override;
    void updateImageMatrix() noexcept;

    sk_sp<SkImage> u_image;
    SkRect u_imageSrcRect;
    SkScalar u_imageScale { 1.f };
    AKTransform u_imageTransform { AKTransform::Normal };

    SkPaint p_paint;
    SkMatrix p_imageMatrix;
    SkRect p_imageSrcRect;
    SkRect p_imageDstRect;
};

#endif // AKImage_H
