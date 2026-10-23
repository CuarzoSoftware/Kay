#ifndef CZ_AKIMAGE_H
#define CZ_AKIMAGE_H

#include <CZ/AK/Nodes/AKRenderable.h>
#include <CZ/Core/CZTransform.h>
#include <CZ/Ream/Ream.h>

/**
 * @brief Node for displaying SkImages.
 * @ingroup AKNodes
 *
 * Renders an RImage stretched to the node's bounds via the Ream painter.
 * The sampled source rect is either the whole image (default) or a custom
 * rect in logical coordinates (see setSrcRectMode()). The node draws nothing
 * while no image is set.
 */
class CZ::AKImage : public AKRenderable
{
public:
    /** @brief Constructs the node without an image. */
    AKImage(AKNode *parent = nullptr) noexcept : AKRenderable(RenderableHint::Image, parent) {}

    /** @brief Constructs the node with an initial image (may be nullptr). */
    AKImage(std::shared_ptr<RImage> image, AKNode *parent = nullptr) noexcept : AKRenderable(RenderableHint::Image, parent), m_image(image) {}

    /** @brief Property change bits consumed on the next AKScene::render(). */
    enum Changes
    {
        /// image() changed.
        CHImage = AKRenderable::CHLast,

        /// srcTransform() changed.
        CHSrcTransform,

        /// srcRectMode() changed.
        CHSrcRectMode,

        /// customSrcRect() changed (flagged only while SrcRectMode::Custom is active).
        CHCustomSrcRect,

        /// customSrcRectScale() changed (flagged only while SrcRectMode::Custom is active).
        CHCustomSrcRectScale,

        CHLast ///< Marker for subclass chaining.
    };

    /** @brief Selects which part of the image is sampled. */
    enum class SrcRectMode
    {
        /// The entire image is sampled (default).
        EntireImage, // Default

        /// customSrcRect() scaled by customSrcRectScale() is sampled.
        Custom
    };

    /**
     * @brief Sets the displayed image.
     *
     * Passing nullptr unsets it (the node then draws nothing). Gaining or losing
     * the image adds full damage; swapping between two images does not, since that
     * is expected to be covered by fine-grained damage added by the owner.
     *
     * @return `true` if the image changed, `false` otherwise.
     */
    bool setImage(std::shared_ptr<RImage> image) noexcept
    {
        if (image.get() == m_image.get())
            return false;

        /* Swapping between images is covered by the fine-grained damage the owner adds (e.g. a
         * client's buffer damage), but gaining or losing the image changes whether the node draws
         * AT ALL. That transition isn't covered by such damage: it may have been consumed by a
         * frame where the node couldn't draw yet (null image), leaving the node permanently stale
         * on the buffers composited before the image arrived. */
        if ((m_image == nullptr) != (image == nullptr))
            addDamage(AK_IRECT_INF);

        addChange(CHImage);
        m_image = image;
        return true;
    }

    /** @brief The displayed image, or nullptr if none is set. */
    std::shared_ptr<RImage> image() const noexcept
    {
        return m_image;
    }

    /**
     * @brief Sets the transform the source image already has.
     *
     * The scene applies the inverse while rendering so the image is displayed upright.
     * Defaults to CZTransform::Normal.
     *
     * @return `true` if the transform changed, `false` otherwise.
     */
    bool setSrcTransform(CZTransform transform) noexcept
    {
        if (m_srcTransform == transform)
            return false;

        addChange(CHSrcTransform);
        m_srcTransform = transform;
        return true;
    }

    /** @brief The current source transform. Defaults to CZTransform::Normal. */
    CZTransform srcTransform() const noexcept
    {
        return m_srcTransform;
    }

    /**
     * @brief Sets how the source rect is determined (see SrcRectMode).
     *
     * @return `true` if the mode changed, `false` otherwise.
     */
    bool setSrcRectMode(SrcRectMode mode) noexcept
    {
        if (m_srcRectMode == mode)
            return false;

        m_srcRectMode = mode;
        addChange(CHSrcRectMode);
        return true;
    }

    /** @brief The current source rect mode. Defaults to SrcRectMode::EntireImage. */
    SrcRectMode srcRectMode() const noexcept
    {
        return m_srcRectMode;
    }

    /**
     * @brief Sets the custom source rect, in logical coordinates (see setCustomSrcRectScale()).
     *
     * Only sampled while srcRectMode() is SrcRectMode::Custom.
     *
     * @return `true` if the rect changed, `false` otherwise.
     */
    bool setCustomSrcRect(const SkRect &rect) noexcept
    {
        if (m_customSrcRect == rect)
            return false;

        if (m_srcRectMode == SrcRectMode::Custom)
            addChange(CHCustomSrcRect);

        m_customSrcRect = rect;
        return true;
    }

    /** @brief The custom source rect in logical coordinates. Defaults to an empty rect. */
    const SkRect &customSrcRect() const noexcept
    {
        return m_customSrcRect;
    }

    /**
     * @brief Sets the scale factor that maps customSrcRect() into image pixels.
     *
     * Only used while srcRectMode() is SrcRectMode::Custom. Values <= 0 are rejected.
     *
     * @return `true` if the scale changed, `false` otherwise.
     */
    bool setCustomSrcRectScale(SkScalar scale) noexcept
    {
        if (scale <= 0.f || m_customSrcRectScale == scale)
            return false;

        if (m_srcRectMode == SrcRectMode::Custom)
            addChange(CHCustomSrcRectScale);
        m_customSrcRectScale = scale;
        return true;
    }

    /** @brief The custom source rect scale factor. Defaults to 1.0. */
    SkScalar customSrcRectScale() const noexcept
    {
        return m_customSrcRectScale;
    }

    /**
     * @brief Toggles automatic full damage when source properties change (default enabled).
     *
     * While enabled, a change to srcTransform(), srcRectMode() or — in custom mode — the
     * custom source rect/scale adds full damage during onSceneBegin(). Disable it when the
     * owner provides its own fine-grained damage (e.g. a client buffer's damage).
     */
    void enableAutoDamage(bool enabled) noexcept
    {
        m_autoDamage = enabled;
    }

    /** @brief Whether automatic damage is enabled. Defaults to `true`. */
    bool autoDamageEnabled() const noexcept
    {
        return m_autoDamage;
    }
protected:
    /** @brief Adds full damage when source properties changed, unless auto damage is disabled. */
    virtual void onSceneBegin() override;

    /** @brief Draws the image into the damaged area of the target. No-op without an image. */
    void renderEvent(const AKRenderEvent &event) override;
    std::shared_ptr<RImage> m_image;
    SkRect m_customSrcRect {};
    SkScalar m_customSrcRectScale { 1.f };
    SrcRectMode m_srcRectMode { SrcRectMode::EntireImage };
    CZTransform m_srcTransform { CZTransform::Normal };
    bool m_autoDamage { true };
};

#endif // CZ_AKIMAGE_H
