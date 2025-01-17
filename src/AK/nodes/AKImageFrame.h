#ifndef AKIMAGEFRAME_H
#define AKIMAGEFRAME_H

#include <AK/nodes/AKContainer.h>
#include <AK/nodes/AKRenderableImage.h>
#include <AK/AKAlignment.h>

/**
 * @brief A frame for images
 *
 * The AK::AKImageFrame node allows displaying SkImages aligned, contained, stretched, etc., within a frame depending on the specified
 * sizeMode() and alignment().
 *
 * It's useful for displaying wallpapers, icons, etc., without having to worry about manually specifying the image dimensions to keep
 * the aspect ratio.
 *
 * It is a subclass of AK::AKContainer and has a child AKRenderableImage whose position and dimensions are calculated and set
 * within the AK::AKImageFrame::updateLayout() event.
 *
 * Given this structure, it is not recommended to add additional child nodes, or change the layout properties of the frame other than
 * size and position, as it could affect the way the image is displayed.
 *
 * @note Since an AKRenderableImage is used, there is no additional buffering required for displaying the images, making it efficient.
 */
class AK::AKImageFrame : public AKContainer
{
public:
    /**
     * @brief Constructs an AKImageFrame without an SkImage.
     *
     * @param parent The node's parent, can be nullptr.
     */
    explicit AKImageFrame(AKNode* parent = nullptr) noexcept;

    /**
     * @brief Constructs an AKImageFrame with an SkImage.
     *
     * @param image The SkImage to display, can be nullptr.
     * @param parent The node's parent, can be nullptr.
     */
    explicit AKImageFrame(sk_sp<SkImage> image, AKNode* parent = nullptr) noexcept;

    AKCLASS_NO_COPY(AKImageFrame)

    enum Changes
    {
        Chg_SizeMode = AKContainer::Chg_Last,
        Chg_Alignment,
        Chg_Last
    };

    /**
     * @brief Size mode
     *
     * The size mode determines how the image is resized and positioned within the frame.
     *
     * @see setSizeMode()
     */
    enum class SizeMode
    {
        /**
         * @brief Contained mode (default)
         *
         * Scales the image to fit within the frame while maintaining its aspect ratio.
         *
         * The image's position within the frame depends on the alignment().
         */
        Contain,

        /**
         * @brief Cover mode
         *
         * Scales the image to cover the entire frame while maintaining its aspect ratio.
         * This may cause parts of the image to be clipped.
         *
         * The image's position within the frame depends on the alignment().
         */
        Cover,

        /**
         * @brief Fill mode
         *
         * The aspect ratio is ignored, and the image is resized to match the frame's dimensions.
         *
         * The image's position within the frame is unaffected by the alignment().
         */
        Fill
    };

    /**
     * @brief Sets the SkImage for the sub AK::AKRenderableImage node.
     *
     * If the image is nullptr, the AK::AKRenderableImage will be hidden,
     * but the frame retains its dimensions.
     *
     * @param image The SkImage to display, can be nullptr.
     */
    void setImage(sk_sp<SkImage> image) noexcept
    {
        m_renderableImage.setImage(image);
    }

    /**
     * @brief Retrieves the SkImage assigned to the sub AK::AKRenderableImage node.
     *
     * @see setImage()
     */
    sk_sp<SkImage> image() const noexcept
    {
        return m_renderableImage.image();
    }

    /**
     * @brief Sets the transform applied to the srcRect().
     *
     * The default value is AK::AKTransform::Normal.
     */
    void setSrcTransform(AKTransform transform) noexcept
    {
        m_renderableImage.setSrcTransform(transform);
    }

    /**
     * @brief Gets the transform applied to the srcRect().
     *
     * @see setSrcTransform()
     *
     * The default value is AK::AKTransform::Normal.
     */
    AKTransform srcTransform() const noexcept
    {
        return m_renderableImage.srcTransform();
    }

    // TODO: add doc
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

    void setSrcRectMode(AKRenderableImage::SrcRectMode mode) noexcept
    {
        m_renderableImage.setSrcRectMode(mode);
    }

    AKRenderableImage::SrcRectMode srcRectMode() const noexcept
    {
        return m_renderableImage.srcRectMode();
    }

    void setCustomSrcRect(const SkRect &rect) noexcept
    {
       m_renderableImage.setCustomSrcRect(rect);
    }

    const SkRect &customSrcRect() const noexcept
    {
        return m_renderableImage.customSrcRect();
    }

    void setCustomSrcRectScale(SkScalar scale) noexcept
    {
        m_renderableImage.setCustomSrcRectScale(scale);
    }

    SkScalar customSrcRectScale() const noexcept
    {
        return m_renderableImage.customSrcRectScale();
    }

    void enableAutoDamage(bool enabled) noexcept
    {
        m_renderableImage.enableAutoDamage(enabled);
    }

    bool autoDamageEnabled() const noexcept
    {
        return m_renderableImage.autoDamageEnabled();
    }

    const AKRenderableImage &renderableImage() const noexcept
    {
        return m_renderableImage;
    }

    AKRenderableImage &renderableImage() noexcept
    {
        return m_renderableImage;
    }

    SkRegion &opaqueRegion() noexcept
    {
        return m_renderableImage.opaqueRegion;
    }

protected:
    virtual void updateLayout() override;
    AKRenderableImage m_renderableImage;
    SizeMode m_sizeMode { SizeMode::Contain };
    AKAlignment m_alignment { AKAlignCenter };
};

#endif // AKIMAGEFRAME_H
