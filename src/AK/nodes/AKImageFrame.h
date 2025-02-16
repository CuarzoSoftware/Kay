#ifndef AKIMAGEFRAME_H
#define AKIMAGEFRAME_H

#include <AK/nodes/AKContainer.h>
#include <AK/nodes/AKRenderableImage.h>
#include <AK/AKAlignment.h>

/**
 * @brief A frame for displaying images with flexible sizing and alignment options.
 *
 * The `AKImageFrame` node allows displaying `SkImage` objects with various sizing and alignment modes,
 * such as contained, cover, or fill. It is particularly useful for displaying wallpapers, icons, or other
 * images where maintaining aspect ratio or positioning is important.
 *
 * This class is a subclass of `AKContainer` and internally uses an `AKRenderableImage` child node to render
 * the image. The position and dimensions of the `AKRenderableImage` are automatically calculated and updated
 * in the `AKImageFrame::updateLayout()` event.
 *
 * @note It is not recommended to add additional child nodes or modify the layout properties of the frame
 * (other than size and position), as this may interfere with the image display logic.
 *
 * @note Since `AKRenderableImage` is used internally, no additional buffering is required for displaying
 * images, making this class efficient for rendering.
 */
class AK::AKImageFrame : public AKContainer
{
public:
    /**
     * @brief Enumeration of changes specific to `AKImageFrame`.
     *
     * Extends the base `AKContainer::Changes` to include additional change types:
     * - `CHSizeMode`: Indicates a change to the size mode (e.g., `Contain`, `Cover`, `Fill`).
     * - `CHAlignment`: Indicates a change to the image alignment within the frame.
     * - `CHLast`: Marks the end of change types for this component.
     */
    enum Changes
    {
        CHSizeMode = AKContainer::CHLast, ///< Change type for the size mode.
        CHAlignment,                        ///< Change type for the alignment.
        CHLast                              ///< Marks the end of change types for this component.
    };

    /**
     * @brief Constructs an `AKImageFrame` without an initial `SkImage`.
     *
     * @param parent The parent `AKNode` to which this frame will be attached. Defaults to `nullptr`.
     */
    explicit AKImageFrame(AKNode* parent = nullptr) noexcept;

    /**
     * @brief Constructs an `AKImageFrame` with an initial `SkImage`.
     *
     * @param image The `SkImage` to display. Can be `nullptr` if no image is provided initially.
     * @param parent The parent `AKNode` to which this frame will be attached. Defaults to `nullptr`.
     */
    explicit AKImageFrame(sk_sp<SkImage> image, AKNode* parent = nullptr) noexcept;

    AKCLASS_NO_COPY(AKImageFrame) ///< Disables copying of `AKImageFrame`.

    /**
     * @brief Enumeration of size modes for the image within the frame.
     *
     * Determines how the image is resized and positioned within the frame.
     */
    enum class SizeMode
    {
        /**
         * @brief Contained mode (default).
         *
         * Scales the image to fit within the frame while maintaining its aspect ratio.
         * The image's position within the frame depends on the `alignment()`.
         */
        Contain,

        /**
         * @brief Cover mode.
         *
         * Scales the image to cover the entire frame while maintaining its aspect ratio.
         * This may cause parts of the image to be clipped.
         * The image's position within the frame depends on the `alignment()`.
         */
        Cover,

        /**
         * @brief Fill mode.
         *
         * Ignores the aspect ratio and resizes the image to match the frame's dimensions exactly.
         * The image's position within the frame is unaffected by the `alignment()`.
         */
        Fill
    };

    /**
     * @brief Sets the `SkImage` to be displayed in the frame.
     *
     * If the image is `nullptr`, the internal `AKRenderableImage` will be hidden,
     * but the frame retains its dimensions.
     *
     * @param image The `SkImage` to display. Can be `nullptr`.
     */
    void setImage(sk_sp<SkImage> image) noexcept
    {
        m_renderableImage.setImage(image);
        updateDimensions();
    }

    /**
     * @brief Retrieves the `SkImage` currently displayed in the frame.
     *
     * @return The `SkImage` being displayed, or `nullptr` if no image is set.
     */
    sk_sp<SkImage> image() const noexcept
    {
        return m_renderableImage.image();
    }

    /**
     * @brief Sets the transform applied to the source rectangle (`srcRect()`).
     *
     * @param transform The transform to apply. Defaults to `AK::AKTransform::Normal`.
     */
    bool setSrcTransform(AKTransform transform) noexcept
    {
        if (m_renderableImage.setSrcTransform(transform))
        {
            updateDimensions();
            return true;
        }

        return false;
    }

    /**
     * @brief Retrieves the transform applied to the source rectangle (`srcRect()`).
     *
     * @return The current transform. Defaults to `AK::AKTransform::Normal`.
     */
    AKTransform srcTransform() const noexcept
    {
        return m_renderableImage.srcTransform();
    }

    /**
     * @brief Sets the alignment of the image within the frame.
     *
     * This property has no effect if the `SizeMode` is set to `Fill`.
     *
     * @param alignment The alignment to apply. Defaults to `AKAlignment::Center`.
     */
    bool setAlignment(AKAlignment alignment) noexcept
    {
        if (m_alignment == alignment)
            return false;

        m_alignment = alignment;
        addChange(CHAlignment);
        updateDimensions();
        return true;
    }

    /**
     * @brief Retrieves the current alignment of the image within the frame.
     *
     * @return The current alignment. Defaults to `AKAlignment::Center`.
     */
    AKAlignment alignment() const noexcept
    {
        return m_alignment;
    }

    /**
     * @brief Sets the size mode for the image within the frame.
     *
     * @param mode The size mode to apply. Defaults to `SizeMode::Contain`.
     */
    bool setSizeMode(SizeMode mode) noexcept
    {
        if (m_sizeMode == mode)
            return false;

        m_sizeMode = mode;
        addChange(CHSizeMode);
        updateDimensions();
        return true;
    }

    /**
     * @brief Retrieves the current size mode for the image within the frame.
     *
     * @return The current size mode. Defaults to `SizeMode::Contain`.
     */
    SizeMode sizeMode() const noexcept
    {
        return m_sizeMode;
    }

    /**
     * @brief Sets the source rectangle mode for the image.
     *
     * Determines whether the entire image or a subrectangle is used for rendering.
     *
     * @param mode The source rectangle mode to apply.
     */
    bool setSrcRectMode(AKRenderableImage::SrcRectMode mode) noexcept
    {
        if (m_renderableImage.setSrcRectMode(mode))
        {
            updateDimensions();
            return true;
        }
        return false;
    }

    /**
     * @brief Retrieves the current source rectangle mode for the image.
     *
     * @return The current source rectangle mode.
     */
    AKRenderableImage::SrcRectMode srcRectMode() const noexcept
    {
        return m_renderableImage.srcRectMode();
    }

    /**
     * @brief Sets a custom source rectangle for the image.
     *
     * This property is only used if the `srcRectMode()` is set to `Custom`.
     *
     * @param rect The custom source rectangle to apply.
     */
    bool setCustomSrcRect(const SkRect &rect) noexcept
    {
        if (m_renderableImage.setCustomSrcRect(rect))
        {
            updateDimensions();
            return true;
        }

        return false;
    }

    /**
     * @brief Retrieves the current custom source rectangle for the image.
     *
     * @return The current custom source rectangle.
     */
    const SkRect &customSrcRect() const noexcept
    {
        return m_renderableImage.customSrcRect();
    }

    /**
     * @brief Sets the scaling factor for the custom source rectangle.
     *
     * This property is only used if the `srcRectMode()` is set to `Custom`.
     *
     * @param scale The scaling factor to apply.
     */
    bool setCustomSrcRectScale(SkScalar scale) noexcept
    {
        if (m_renderableImage.setCustomSrcRectScale(scale))
        {
            updateDimensions();
            return true;
        }

        return false;
    }

    /**
     * @brief Retrieves the current scaling factor for the custom source rectangle.
     *
     * @return The current scaling factor.
     */
    SkScalar customSrcRectScale() const noexcept
    {
        return m_renderableImage.customSrcRectScale();
    }

    /**
     * @brief Enables or disables automatic damage tracking for the `AKRenderableImage`.
     *
     * When enabled, the `AKRenderableImage` will automatically mark itself as damaged when properties
     * such as the transform, scaling factor, etc., change. However, manual damage marking is still
     * required if the image is replaced with another of the same size.
     *
     * @param enabled Whether to enable automatic damage tracking.
     */
    void enableAutoDamage(bool enabled) noexcept
    {
        m_renderableImage.enableAutoDamage(enabled);
    }

    /**
     * @brief Checks if automatic damage tracking is enabled for the `AKRenderableImage`.
     *
     * @return `true` if automatic damage tracking is enabled, `false` otherwise.
     */
    bool autoDamageEnabled() const noexcept
    {
        return m_renderableImage.autoDamageEnabled();
    }

    /**
     * @brief Retrieves the internal `AKRenderableImage` component.
     *
     * @return A const reference to the internal `AKRenderableImage`.
     */
    const AKRenderableImage &renderableImage() const noexcept
    {
        return m_renderableImage;
    }

    /**
     * @brief Retrieves the internal `AKRenderableImage` component.
     *
     * @return A mutable reference to the internal `AKRenderableImage`.
     */
    AKRenderableImage &renderableImage() noexcept
    {
        return m_renderableImage;
    }

    /**
     * @brief Retrieves the opaque region of the `AKRenderableImage`.
     *
     * @return A reference to the opaque region of the `AKRenderableImage`.
     */
    SkRegion &opaqueRegion() noexcept
    {
        return m_renderableImage.opaqueRegion;
    }

protected:
    void init() noexcept;
    void updateDimensions() noexcept;
    AKRenderableImage m_renderableImage;
    SizeMode m_sizeMode { SizeMode::Contain };
    AKAlignment m_alignment { AKAlignCenter };
};

#endif // AKIMAGEFRAME_H
