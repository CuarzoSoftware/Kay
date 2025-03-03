#ifndef AKRENDERABLEIMAGE_H
#define AKRENDERABLEIMAGE_H

#include <AK/nodes/AKRenderable.h>

/**
 * @brief Node for displaying SkImages.
 * @ingroup AKNodes
 */
class AK::AKRenderableImage : public AKRenderable
{
public:
    AKRenderableImage(AKNode *parent = nullptr) noexcept : AKRenderable(Texture, parent) {}
    AKRenderableImage(sk_sp<SkImage> image, AKNode *parent = nullptr) noexcept : AKRenderable(Texture, parent), m_image(image) {}

    AKCLASS_NO_COPY(AKRenderableImage)

    enum Changes
    {
        CHImage = AKRenderable::CHLast,
        CHSrcTransform,
        CHSrcRectMode,
        CHCustomSrcRect,
        CHCustomSrcRectScale,
        CHLast
    };

    enum class SrcRectMode
    {
        EntireImage, // Default
        Custom
    };


    bool setImage(sk_sp<SkImage> image) noexcept
    {
        if (image.get() == m_image.get())
            return false;

        addChange(CHImage);
        m_image = image;
        return true;
    }

    sk_sp<SkImage> image() const noexcept
    {
        return m_image;
    }

    bool setSrcTransform(AKTransform transform) noexcept
    {
        if (m_srcTransform == transform)
            return false;

        addChange(CHSrcTransform);
        m_srcTransform = transform;
        return true;
    }

    AKTransform srcTransform() const noexcept
    {
        return m_srcTransform;
    }

    bool setSrcRectMode(SrcRectMode mode) noexcept
    {
        if (m_srcRectMode == mode)
            return false;

        m_srcRectMode = mode;
        addChange(CHSrcRectMode);
        return true;
    }

    SrcRectMode srcRectMode() const noexcept
    {
        return m_srcRectMode;
    }

    bool setCustomSrcRect(const SkRect &rect) noexcept
    {
        if (m_customSrcRect == rect)
            return false;

        if (m_srcRectMode == SrcRectMode::Custom)
            addChange(CHCustomSrcRect);

        m_customSrcRect = rect;
        return true;
    }

    const SkRect &customSrcRect() const noexcept
    {
        return m_customSrcRect;
    }

    bool setCustomSrcRectScale(SkScalar scale) noexcept
    {
        if (scale <= 0.f || m_customSrcRectScale == scale)
            return false;

        if (m_srcRectMode == SrcRectMode::Custom)
            addChange(CHCustomSrcRectScale);
        m_customSrcRectScale = scale;
        return true;
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
    void renderEvent(const AKRenderEvent &event) override;
    sk_sp<SkImage> m_image;
    SkRect m_customSrcRect { 0.f, 0.f, 0.f, 0.f };
    SkScalar m_customSrcRectScale { 1.f };
    SrcRectMode m_srcRectMode { SrcRectMode::EntireImage };
    AKTransform m_srcTransform { AKTransform::Normal };
    bool m_autoDamage { true };
};

#endif // AKRENDERABLEIMAGE_H
