#ifndef CZ_AKTHREEIMAGEPATCH_H
#define CZ_AKTHREEIMAGEPATCH_H

#include <CZ/AK/Nodes/AKRenderable.h>
#include <CZ/Core/CZOrientation.h>

/**
 * @brief Vertical or horizontal three image patch
 *
 * [Fixed Left/Top] [Stretched Center] [Mirrored Left/Top Side]
 *
 * @note The height of all three parts is stretched to match the component's height.
 */
class CZ::AKThreePatch : public AKRenderable
{
public:
    enum Changes
    {
        CHSideSrcRect = AKRenderable::CHLast,
        CHCenterSrcRect,
        CHOrientation,
        CHImageScale,
        CHImage,
        CHKeepSidesAspectRatio,
        CHLast
    };

    /**
     * @brief Constructs an AKThreePatch node.
     */
    AKThreePatch(CZOrientation orientation, AKNode *parent = nullptr) noexcept :
        AKRenderable(RenderableHint::Image, parent),
        m_orientation(orientation) {};


    void setOrientation(CZOrientation orientation) noexcept
    {
        if (m_orientation == orientation)
            return;

        m_orientation = orientation;
        addChange(CHOrientation);
        addDamage(AK_IRECT_INF);
    }

    CZOrientation orientation() const noexcept { return m_orientation; }

    /**
     * @brief Sets the subrect of the image used for the left/top side.
     *
     * The right/bottom side uses the same subrectangle but mirrored.
     *
     * @param rect The subrect in logical coordinates.
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
     * @brief Returns the subrect of the imaga used for the left/top side.
     *
     * @return The subrect in logical coordinates.
     */
    const SkRect &sideSrcRect() const noexcept
    {
        return m_sideSrcRect;
    }

    /**
     * @brief Sets the subrect of the image used for the stretchable center.
     *
     * @param rect The subrect in logical coordinates.
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
     * @brief Returns the subrect of the image used for the stretchable center.
     *
     * @return The subrect in logical coordinates.
     */
    const SkRect &centerSrcRect() const noexcept { return m_centerSrcRect; }

    /**
     * @brief Sets the scaling factor of the image.
     *
     * This scaling factor is used to properly convert sideSrcRect() and centerSrcRect()
     * into pixel coordiantes.
     *
     * @param scale The scaling factor to apply.
     */
    void setImageScale(SkScalar scale) noexcept
    {
        if (scale < 0.1f)
            scale = 0.1f;

        if (scale == m_imageScale)
            return;

        m_imageScale = scale;
        addChange(CHImageScale);
        addDamage(AK_IRECT_INF);
    }

    /**
     * @brief Returns the current scaling factor of the image.
     *
     * @return The scaling factor. The default value is 1.f.
     */
    SkScalar imageScale() const noexcept { return m_imageScale; }

    /**
     * @brief Sets the patch image source.
     *
     * The image should contain two parts: left or top and center.
     *
     * @param image `nullptr` to unset.
     */
    void setImage(std::shared_ptr<RImage> image) noexcept
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
     * @return The image being used or `nullptr`.
     */
    std::shared_ptr<RImage> image() const noexcept { return m_image; }

    bool keepSidesAspectRatio() const noexcept { return m_keepSidesAspectRatio; }

    /**
     * @brief Forces to keep sides aspect ratio.
     *
     * Enabled by default.
     */
    bool setKeepSidesAspectRatio(bool keepAspectRatio) noexcept
    {
        if (keepAspectRatio == m_keepSidesAspectRatio)
            return false;

        m_keepSidesAspectRatio = keepAspectRatio;
        addChange(CHKeepSidesAspectRatio);
        addDamage(AK_IRECT_INF);
        return true;
    }

protected:
    void renderEvent(const AKRenderEvent &event) override;
    SkRect m_sideSrcRect { 0.f, 0.f, 0.f, 0.f };
    SkRect m_centerSrcRect { 0.f, 0.f, 0.f, 0.f };
    SkScalar m_imageScale { 1.f };
    CZOrientation m_orientation;
    std::shared_ptr<RImage> m_image;
    bool m_keepSidesAspectRatio { true };
};

#endif // CZ_AKTHREEIMAGEPATCH_H
