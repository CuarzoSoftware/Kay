#ifndef CZ_AKIMAGEFRAME_H
#define CZ_AKIMAGEFRAME_H

#include <CZ/AK/Nodes/AKRenderable.h>
#include <CZ/Core/CZTransform.h>
#include <CZ/Core/CZAlignment.h>

/**
 * @brief Image view with flexible sizing and alignment options.
 *
 * Displays an image framed by the node bounds.
 */
class CZ::AKImageFrame : public AKRenderable
{
public:

    enum Changes
    {
        CHSizeMode = AKRenderable::CHLast,
        CHAlignment,
        CHTransform,
        CHImage,
        CHSrcRect,
        CHScale,
        CHLast
    };

    explicit AKImageFrame(AKNode* parent = nullptr) noexcept;
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
    bool setTransform(CZTransform transform) noexcept;

    /**
     * @brief Retrieves the current alignment of the image within the frame.
     *
     * This property has no effect if the `SizeMode` is set to `Fill`.
     *
     * @return The current alignment. Defaults to 0 (center).
     */
    CZAlignment alignment() const noexcept { return m_alignment; }
    bool setAlignment(CZAlignment alignment) noexcept;

    /**
     * @brief Retrieves the current size mode for the image within the frame.
     *
     * @return The current size mode. Defaults to `SizeMode::Contain`.
     */
    SizeMode sizeMode() const noexcept { return m_sizeMode; }
    bool setSizeMode(SizeMode mode) noexcept;

    /**
     * @brief Retrieves the current image source rect.
     *
     * Defaults to nullopt (the entire image is used).
     */
    std::optional<SkRect> srcRect() const noexcept { return m_srcRect; }
    bool setSrcRect(std::optional<SkRect> rect) noexcept;

    /**
     * @brief Retrieves the current scaling factor for the custom source rectangle.
     *
     * @return The current scaling factor.
     */
    SkScalar scale() const noexcept { return m_scale;}
    bool setScale(SkScalar scale) noexcept;

protected:
    void onSceneBegin() override;
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
