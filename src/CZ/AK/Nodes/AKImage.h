#ifndef CZ_AKIMAGE_H
#define CZ_AKIMAGE_H

#include <CZ/AK/Nodes/AKRenderable.h>
#include <CZ/Core/CZTransform.h>
#include <CZ/Ream/Ream.h>

/**
 * @brief Node for displaying SkImages.
 * @ingroup AKNodes
 */
class CZ::AKImage : public AKRenderable
{
public:
    AKImage(AKNode *parent = nullptr) noexcept : AKRenderable(RenderableHint::Image, parent) {}
    AKImage(std::shared_ptr<RImage> image, AKNode *parent = nullptr) noexcept : AKRenderable(RenderableHint::Image, parent), m_image(image) {}

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

    bool setImage(std::shared_ptr<RImage> image) noexcept
    {
        if (image.get() == m_image.get())
            return false;

        addChange(CHImage);
        m_image = image;
        return true;
    }

    std::shared_ptr<RImage> image() const noexcept
    {
        return m_image;
    }

    bool setSrcTransform(CZTransform transform) noexcept
    {
        if (m_srcTransform == transform)
            return false;

        addChange(CHSrcTransform);
        m_srcTransform = transform;
        return true;
    }

    CZTransform srcTransform() const noexcept
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
    std::shared_ptr<RImage> m_image;
    SkRect m_customSrcRect {};
    SkScalar m_customSrcRectScale { 1.f };
    SrcRectMode m_srcRectMode { SrcRectMode::EntireImage };
    CZTransform m_srcTransform { CZTransform::Normal };
    bool m_autoDamage { true };
};

#endif // CZ_AKIMAGE_H
