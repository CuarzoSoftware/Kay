#ifndef AKImage_H
#define AKImage_H

#include <AK/nodes/AKRenderable.h>
#include <AK/AKTransform.h>
#include <AK/AKAlignment.h>

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
        Chg_SizeMode,
        Chg_Transform,
        Chg_Alignment,
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

    enum class SizeMode
    {
        Contain, // Default
        Cover,
        Fit
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

    void setAlignment(AKAlignment alignment) noexcept
    {
        if (m_alignment == alignment)
            return;

        m_alignment = alignment;
        addChange(Chg_Alignment);
    }

    AKAlignment alignment() const noexcept
    {
        return m_alignment;
    }

    void setSizeMode(SizeMode mode) noexcept
    {
        if (m_sizeMode == mode)
            return;

        m_sizeMode = mode;
        addChange(Chg_SizeMode);
    }

    SizeMode sizeMode() const noexcept
    {
        return m_sizeMode;
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
    SizeMode m_sizeMode { SizeMode::Contain };
    AKTransform m_transform { AKTransform::Normal };
    AKAlignment m_alignment { AKAlignCenter };
    bool m_autoDamage { true };
};

#endif // AKImage_H
