#ifndef CZ_AKIMAGEFRAME_H
#define CZ_AKIMAGEFRAME_H

#include <CZ/AK/Nodes/AKRenderable.h>
#include <CZ/Core/CZTransform.h>
#include <CZ/Core/CZAlignment.h>

/**
 * @brief Image view with flexible sizing and alignment options.
 * @ingroup AKNodes
 *
 * Displays an image framed by the node bounds.
 */
class CZ::AKImageFrame : public AKRenderable
{
public:

    /** @brief Property change bits consumed on the next AKScene::render(). */
    enum Changes
    {
        /// sizeMode() changed.
        CHSizeMode = AKRenderable::CHLast,

        /// alignment() changed.
        CHAlignment,

        /// transform() changed.
        CHTransform,

        /// image() changed.
        CHImage,

        /// srcRect() changed.
        CHSrcRect,

        /// scale() changed.
        CHScale,

        CHLast ///< Marker for subclass chaining.
    };

    /** @brief Constructs the frame without an image. */
    explicit AKImageFrame(AKNode* parent = nullptr) noexcept;

    /** @brief Constructs the frame with an initial image (may be nullptr). */
    explicit AKImageFrame(std::shared_ptr<RImage> image, AKNode* parent = nullptr) noexcept;

    /**
     * @brief Determines how the image is resized and positioned within the frame.
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
     * @brief Retrieves the currently displayed in the frame.
     *
     * @return `nullptr` if no image is set.
     */
    std::shared_ptr<RImage> image() const noexcept { return m_image; }

    /**
     * @brief Sets the displayed image (nullptr to unset; the node then draws nothing).
     *
     * @return `true` if the image changed, `false` otherwise.
     */
    bool setImage(std::shared_ptr<RImage> image) noexcept;

    /**
     * @brief Retrieves the source rect transform.
     *
     * @note This represents the transform that the source image already has.
     *       For example, if the image is rotated 90° counter-clockwise,
     *       set this value to CZTransform::Rotated90. The scene will then
     *       apply a 90° clockwise rotation to display the image as normal.
     *
     * @return The current transform. Defaults to `CZTransform::Normal`.
     */
    CZTransform transform() const noexcept { return m_transform; }

    /** @brief Sets the source rect transform (see transform()). */
    bool setTransform(CZTransform transform) noexcept;

    /**
     * @brief Retrieves the current alignment of the image within the frame.
     *
     * This property has no effect if the `SizeMode` is set to `Fill`.
     *
     * @return The current alignment. Defaults to 0 (center).
     */
    CZAlignment alignment() const noexcept { return m_alignment; }

    /**
     * @brief Sets the alignment of the image within the frame (see alignment()).
     *
     * @return `true` if the alignment changed, `false` otherwise.
     */
    bool setAlignment(CZAlignment alignment) noexcept;

    /**
     * @brief Retrieves the current size mode for the image within the frame.
     *
     * @return The current size mode. Defaults to `SizeMode::Contain`.
     */
    SizeMode sizeMode() const noexcept { return m_sizeMode; }

    /**
     * @brief Sets how the image is resized within the frame (see SizeMode).
     *
     * @return `true` if the mode changed, `false` otherwise.
     */
    bool setSizeMode(SizeMode mode) noexcept;

    /**
     * @brief Retrieves the current image source rect.
     *
     * Defaults to nullopt (the entire image is used).
     */
    std::optional<SkRect> srcRect() const noexcept { return m_srcRect; }

    /**
     * @brief Sets a custom source rect, in logical coordinates (see setScale()).
     *
     * Pass std::nullopt to use the entire image (the default).
     */
    bool setSrcRect(std::optional<SkRect> rect) noexcept;

    /**
     * @brief Retrieves the current scaling factor for the custom source rectangle.
     *
     * @return The current scaling factor.
     */
    SkScalar scale() const noexcept { return m_scale;}

    /**
     * @brief Sets the scale factor that maps srcRect() into image pixels (only used when a srcRect() is set).
     *
     * Clamped to a minimum of 0.1. Defaults to 1.0.
     *
     * @return `true` if the scale changed, `false` otherwise.
     */
    bool setScale(SkScalar scale) noexcept;

protected:
    /** @brief Recalculates the source/destination rects and the invisible region when relevant properties changed. */
    void onSceneBegin() override;

    /** @brief Draws the image into the calculated destination rect. No-op without an image. */
    void renderEvent(const AKRenderEvent &e) override;
    std::shared_ptr<RImage> m_image;
    std::optional<SkRect> m_srcRect;
    SkScalar m_scale { 1.f };
    CZTransform m_transform { CZTransform::Normal };
    SizeMode m_sizeMode { SizeMode::Contain };
    CZAlignment m_alignment { CZAlignCenter };

    // When no srcRect is provided
    SkRect m_finalSrcRect {};

    // Relative to the node position
    SkIRect m_dst {};
};

#endif // CZ_AKIMAGEFRAME_H
