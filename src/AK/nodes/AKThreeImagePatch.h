#ifndef AKTHREEIMAGEPATCH_H
#define AKTHREEIMAGEPATCH_H

#include <AK/nodes/AKRenderable.h>

/**
 * @brief Displays a horizontal three-patch image.
 * @ingroup AKNodes
 *
 * This component renders a three-patch image composed of:
 * - A **fixed-width left side**, displaying the subrectangle of the image defined by `sideSrcRect()`.
 * - A **stretchable center**, displaying the subrectangle of the image defined by `centerSrcRect()`.
 * - A **fixed-width right side**, displaying a horizontally mirrored version of the subrectangle defined by `sideSrcRect()`.
 *
 * The layout follows this pattern:
 * **[Fixed Left] [Stretched Center] [Mirrored Left Side]**
 *
 * @note The height of all three parts is stretched to match the component's height.
 *
 * This node is commonly used in components like `AKButton` or `AKTextField` to optimize memory usage and improve rendering performance.
 */
class AK::AKThreeImagePatch : public AKRenderable
{
public:
    enum Changes
    {
        CHSideSrcRect = AKRenderable::CHLast,
        CHCenterSrcRect,
        CHOrientation,
        CHImageScale,
        CHImage,
        CHLast
    };

    enum Orientation
    {
        Horizontal,
        Vertical
    };

    /**
     * @brief Constructs an AKHThreePatch node.
     *
     * @param parent The parent AKNode. Defaults to `nullptr` if no parent is provided.
     */
    AKThreeImagePatch(Orientation orientation, AKNode *parent = nullptr) noexcept :
        AKRenderable(RenderableHint::Texture, parent),
        m_orientation(orientation) {};

    void setOrientation(Orientation orientation) noexcept
    {
        if (m_orientation == orientation)
            return;

        m_orientation = orientation;
        addChange(CHOrientation);
        addDamage(AK_IRECT_INF);
    }

    Orientation orientation() const noexcept
    {
        return m_orientation;
    }

    /**
     * @brief Sets the subrectangle of the image() used for the left side.
     *
     * The right side uses the same subrectangle but horizontally mirrored.
     *
     * @param rect The subrectangle of image() in logical coordinates. Use setScale() to specify the image() scaling factor.
     */
    void setSideSrcRect(const SkRect &rect) noexcept
    {
        if (rect == m_sideSrcRect)
            return;

        m_sideSrcRect = rect;
        addChange(CHSideSrcRect);
        addDamage(AK_IRECT_INF);
    }

    /**
     * @brief Returns the subrectangle of the image() used for the left side.
     *
     * @return The subrectangle in logical coordinates.
     */
    const SkRect &sideSrcRect() const noexcept
    {
        return m_sideSrcRect;
    }

    /**
     * @brief Sets the subrectangle of the image() used for the stretchable center.
     *
     * @param rect The subrectangle of the image in logical coordinates. Use setScale() to specify the image() scaling factor.
     */
    void setCenterSrcRect(const SkRect &rect) noexcept
    {
        if (rect == m_centerSrcRect)
            return;

        m_centerSrcRect = rect;
        addChange(CHCenterSrcRect);
        addDamage(AK_IRECT_INF);
    }

    /**
     * @brief Returns the subrectangle of the image() used for the stretchable center.
     *
     * @return The subrectangle in logical coordinates.
     */
    const SkRect &centerSrcRect() const noexcept
    {
        return m_centerSrcRect;
    }

    /**
     * @brief Sets the scaling factor for the image().
     *
     * This scaling factor is used to properly convert sideSrcRect() and centerSrcRect()
     * into buffer coordiantes.
     *
     * @param scale The scaling factor to apply.
     */
    void setImageScale(SkScalar scale) noexcept
    {
        if (scale == m_imageScale)
            return;

        m_imageScale = scale;
        addChange(CHImageScale);
        addDamage(AK_IRECT_INF);
    }

    /**
     * @brief Returns the current scaling factor applied to the image().
     *
     * @return The scaling factor. The default value is 1.f.
     */
    SkScalar imageScale() const noexcept
    {
        return m_imageScale;
    }

    /**
     * @brief Sets the patch image source.
     *
     * The image should contain two parts: left and center. The left and right sides are defined by sideSrcRect(),
     * while the center by centerSrcRect().
     *
     * @param image The SkImage to be used for rendering or `nullptr` to unset.
     */
    void setImage(sk_sp<SkImage> image) noexcept
    {
        if (m_image == image)
            return;

        m_image = image;
        addChange(CHImage);
        addDamage(AK_IRECT_INF);
    }

    /**
     * @brief Returns the current patch image.
     *
     * @return The SkImage being used or `nullptr`.
     */
    sk_sp<SkImage> image() const noexcept
    {
        return m_image;
    }

protected:
    void onRender(const OnRenderParams &params) override;
    SkRect m_sideSrcRect { 0.f, 0.f, 0.f, 0.f };
    SkRect m_centerSrcRect { 0.f, 0.f, 0.f, 0.f };
    SkScalar m_imageScale { 1.f };
    Orientation m_orientation;
    sk_sp<SkImage> m_image;
};

#endif // AKTHREEIMAGEPATCH_H
