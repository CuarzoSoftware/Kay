#ifndef AKImage_H
#define AKImage_H

#include <AK/nodes/AKRenderable.h>
#include <AK/AKTransform.h>

#include <include/core/SkImage.h>
#include <include/core/SkRect.h>
#include <include/core/SkPaint.h>

class AK::AKImage : public AKRenderable
{
public:
    AKImage(AKNode *parent = nullptr) noexcept : AKRenderable(Texture, parent) {}
    AKImage(sk_sp<SkImage> image, AKNode *parent = nullptr) noexcept : AKRenderable(Texture, parent), m_image(image) {}

    enum Changes
    {
        Chg_Image = AKRenderable::Chg_Last,
        Chg_SrcRect,
        Chg_Transform,
        Chg_Scale,

        Chg_Last
    };

    void setImage(sk_sp<SkImage> image) noexcept
    {
        addChange(Chg_Image);
        m_image = image;
    }

    sk_sp<SkImage> image() const noexcept
    {
        return m_image;
    }

    void setSrcRect(const SkRect &rect) noexcept
    {
        if (m_srcRect == rect)
            return;

        addChange(Chg_SrcRect);
        m_srcRect = rect;
    }

    const SkRect &srcRect() const noexcept
    {
        return m_srcRect;
    }

    void setTransform(AKTransform transform) noexcept
    {
        if (m_transform == transform)
            return;

        addChange(Chg_Transform);
        m_transform = transform;
    }

    AKTransform transform() const noexcept
    {
        return m_transform;
    }

    void setScale(SkScalar scale) noexcept
    {
        if (scale <= 0.f || m_scale == scale)
            return;

        addChange(Chg_Scale);
        m_scale = scale;
    }

    SkScalar scale() const noexcept
    {
        return m_scale;
    }

protected:
    virtual void onRender(AKPainter *painter, const SkRegion &damage) override;
    sk_sp<SkImage> m_image;
    SkRect m_srcRect;
    SkScalar m_scale { 1.f };
    AKTransform m_transform { AKTransform::Normal };
};

#endif // AKImage_H
