#ifndef AKIMAGE_H
#define AKIMAGE_H

#include <AKRenderable.h>
#include <AKTransform.h>
#include <include/core/SkImage.h>
#include <include/core/SkRect.h>

class AK::AKImage : public AKRenderable
{
public:
    AKImage(AKNode *parent = nullptr) noexcept : AKRenderable(parent) {}
    AKImage(sk_sp<SkImage> image, AKNode *parent = nullptr) noexcept : AKRenderable(parent), m_image(image) {}

    void setImage(sk_sp<SkImage> image) noexcept
    {
        m_image = image;
    }

    sk_sp<SkImage> image() const noexcept
    {
        return m_image;
    }

    void setSrcRect(const SkRect &rect) noexcept
    {
        m_srcRect = rect;
    }

    const SkRect &srcRect() const noexcept
    {
        return m_srcRect;
    }

    void setTransform(AKTransform transform) noexcept
    {
        m_transform = transform;
    }

    AKTransform transform() const noexcept
    {
        return m_transform;
    }

    void setScale(SkScalar scale) noexcept
    {
        if (scale <= 0.f)
            return;

        m_scale = scale;
    }

    SkScalar scale() const noexcept
    {
        return m_scale;
    }

    void setFilter(sk_sp<SkImageFilter> filter) noexcept
    {
        m_filter = filter;
    }

private:
    virtual void onRender(SkCanvas *canvas, const SkRegion &damage, bool opaque) override;
    void calculateRenderParams(SkMatrix &matrix, SkRect &srcRect, SkRect &dstRect) const noexcept;
    sk_sp<SkImage> m_image;
    sk_sp<SkImageFilter> m_filter;
    SkRect m_srcRect;
    float m_scale { 1.f };
    AKTransform m_transform { AKTransform::Normal };
};

#endif // AKIMAGE_H
