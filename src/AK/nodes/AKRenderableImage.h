#ifndef AKRENDERABLEIMAGE_H
#define AKRENDERABLEIMAGE_H

#include <AK/nodes/AKRenderable.h>

class AK::AKRenderableImage : public AKRenderable
{
public:
    AKRenderableImage(AKNode *parent = nullptr) noexcept : AKRenderable(Texture, parent) {}
    AKRenderableImage(sk_sp<SkImage> image, AKNode *parent = nullptr) noexcept : AKRenderable(Texture, parent), m_image(image) {}

    enum Changes
    {
        Chg_Image = AKRenderable::Chg_Last,
        Chg_SrcTransform,
        Chg_SrcRectMode,
        Chg_CustomSrcRect,
        Chg_CustomSrcRectScale,
        Chg_Last
    };

    enum class SrcRectMode
    {
        EntireImage, // Default
        Custom
    };


    void setImage(sk_sp<SkImage> image) noexcept
    {
        if (image.get() == m_image.get())
            return;

        addChange(Chg_Image);
        m_image = image;
    }

    sk_sp<SkImage> image() const noexcept
    {
        return m_image;
    }

    void setSrcTransform(AKTransform transform) noexcept
    {
        if (m_srcTransform == transform)
            return;

        addChange(Chg_SrcTransform);
        m_srcTransform = transform;
    }

    AKTransform srcTransform() const noexcept
    {
        return m_srcTransform;
    }

    void setSrcRectMode(SrcRectMode mode) noexcept
    {
        if (m_srcRectMode == mode)
            return;

        m_srcRectMode = mode;
        addChange(Chg_SrcRectMode);
    }

    SrcRectMode srcRectMode() const noexcept
    {
        return m_srcRectMode;
    }

    void setCustomSrcRect(const SkRect &rect) noexcept
    {
        if (m_customSrcRect == rect)
            return;

        if (m_srcRectMode == SrcRectMode::Custom)
            addChange(Chg_CustomSrcRect);

        m_customSrcRect = rect;
    }

    const SkRect &customSrcRect() const noexcept
    {
        return m_customSrcRect;
    }

    void setCustomSrcRectScale(SkScalar scale) noexcept
    {
        if (scale <= 0.f || m_customSrcRectScale == scale)
            return;

        if (m_srcRectMode == SrcRectMode::Custom)
            addChange(Chg_CustomSrcRectScale);
        m_customSrcRectScale = scale;
    }

    SkScalar customSrcRectScale() const noexcept
    {
        return m_customSrcRectScale;
    }

    void enableAutoDamage(bool enabled) noexcept
    {
        m_autoDamage = enabled;
    }

    bool autoDamageEnabled() const noexcept
    {
        return m_autoDamage;
    }
protected:
    virtual void onSceneBegin() override;
    virtual void onRender(AKPainter *painter, const SkRegion &damage) override;
    sk_sp<SkImage> m_image;
    SkRect m_customSrcRect { 0.f, 0.f, 0.f, 0.f };
    SkScalar m_customSrcRectScale { 1.f };
    SrcRectMode m_srcRectMode { SrcRectMode::EntireImage };
    AKTransform m_srcTransform { AKTransform::Normal };
    bool m_autoDamage { true };
};

#endif // AKRENDERABLEIMAGE_H
