#ifndef CZ_AKBLURIMAGE_H
#define CZ_AKBLURIMAGE_H

#include <CZ/AK/Nodes/AKBakeable.h>
#include <CZ/Ream/Ream.h>

/**
 * @brief An image drawn blurred (Gaussian), e.g. a background that blurs in and out.
 * @ingroup AKNodes
 *
 * Unlike AKBackgroundBlurEffect (which blurs whatever is behind a node), this blurs its own image.
 * The image is fitted to the node (sizeMode()) over backgroundColor(), and blurred with
 * blurRadius(): changing it (e.g. animating it) bakes it again, nothing is baked while it doesn't
 * change. The edges are clamped, so they don't fade into the background color.
 *
 * Large radii are cheap on the GPU: Skia blurs a downscaled copy.
 */
class CZ::AKBlurImage : public AKBakeable
{
public:
    /** @brief How the image is fitted to the node. */
    enum class SizeMode
    {
        Cover,   ///< Fills the node keeping its aspect ratio, cropped (centered).
        Contain, ///< Fits inside keeping its aspect ratio (centered), backgroundColor() around it.
        Fill     ///< Stretched to the node.
    };

    enum Changes
    {
        CHImage = AKBakeable::CHLast, ///< image() changed.
        CHBlurRadius,                 ///< blurRadius() changed.
        CHSizeMode,                   ///< sizeMode() changed.
        CHBackgroundColor,            ///< backgroundColor() changed.
        CHLast                        ///< Marker for subclass chaining.
    };

    AKBlurImage(std::shared_ptr<RImage> image = nullptr, AKNode *parent = nullptr) noexcept;

    bool setImage(std::shared_ptr<RImage> image) noexcept;
    std::shared_ptr<RImage> image() const noexcept { return m_image; }

    /** @brief Gaussian standard deviation in logical units (like CSS `blur()`), 0 (default) = sharp. */
    bool setBlurRadius(SkScalar radius) noexcept;
    SkScalar blurRadius() const noexcept { return m_blurRadius; }

    /** @brief Defaults to Cover. */
    bool setSizeMode(SizeMode mode) noexcept;
    SizeMode sizeMode() const noexcept { return m_sizeMode; }

    /** @brief Drawn behind the image (and alone without one). Defaults to opaque black. */
    bool setBackgroundColor(SkColor color) noexcept;
    SkColor backgroundColor() const noexcept { return m_backgroundColor; }

protected:
    void bakeEvent(const AKBakeEvent &event) override;

private:
    std::shared_ptr<RImage> m_image;
    SkScalar m_blurRadius { 0.f };
    SizeMode m_sizeMode { SizeMode::Cover };
    SkColor m_backgroundColor { SK_ColorBLACK };
};

#endif // CZ_AKBLURIMAGE_H
