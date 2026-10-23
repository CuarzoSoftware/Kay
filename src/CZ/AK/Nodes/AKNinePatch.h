#ifndef AKNINEPATCH_H
#define AKNINEPATCH_H

#include <CZ/AK/Nodes/AKRenderable.h>

/**
 * @brief Nine-patch image node.
 * @ingroup AKNodes
 *
 * Splits an image into nine regions defined by a center rect (see setCenter()):
 * the four corners keep their size, the four edges and the center stretch to fill
 * the node's bounds. Each region can be hidden individually with
 * setRegionVisibility(). Nothing is rendered while no image is set or the center
 * rect is invalid (negative origin or exceeding the image's logical size).
 */
class CZ::AKNinePatch : public AKRenderable
{
public:
    /** @brief Property change bits consumed on the next AKScene::render(). */
    enum Changes
    {
        /// image() changed.
        CHImage = AKRenderable::CHLast,

        /// center() changed.
        CHCenter,

        /// regionVisibility() changed for some region.
        CHRegionVisibility,

        CHLast ///< Marker for subclass chaining.
    };

    /**
     * @brief The nine patch regions.
     *
     * TL, TR, BR and BL are the fixed-size corners; L, T, R and B the stretched
     * edges; C the stretched center. Region_Last is the number of regions.
     */
    enum Region
    {
        TL, TR, BR, BL, L, T, R, B, C, Region_Last
    };

    /** @brief Constructs the node with an optional image and center rect. */
    AKNinePatch(std::shared_ptr<RImage> image = nullptr, SkIRect center = {}, AKNode *parent = nullptr) noexcept;

    /**
     * @brief Sets the source image (nullptr to unset). Adds full damage.
     *
     * @return `true` if the image changed, `false` otherwise.
     */
    bool setImage(std::shared_ptr<RImage> image) noexcept;

    /** @brief The source image, or nullptr if none is set. */
    std::shared_ptr<RImage> image() const noexcept { return m_image; }

    /**
     * @brief Sets the center rect, in logical image coordinates. Adds full damage.
     *
     * Everything outside it forms the corner and edge regions.
     *
     * @return `true` if the rect changed, `false` otherwise.
     */
    bool setCenter(SkIRect center) noexcept;

    /** @brief The center rect in logical image coordinates. */
    const SkIRect &center() const noexcept { return m_center; };

    /**
     * @brief Shows or hides a single region (all are visible by default).
     *
     * @return `true` if the visibility changed, `false` otherwise.
     */
    bool setRegionVisibility(Region reg, bool visible) noexcept;

    /** @brief Whether the given region is visible. */
    bool regionVisibility(Region reg) noexcept { return m_regions[reg].visible; }

protected:
    /** @brief Recalculates the regions' src/dst rects when the image, center, visibility, size or scale changed. */
    void onSceneBegin() override;

    /** @brief Draws each visible region into the damaged area. No-op without an image. */
    void renderEvent(const AKRenderEvent &e) override;
    void updateRegions() noexcept;
    SkIRect m_center {};
    std::shared_ptr<RImage> m_image;
    struct {
        bool visible { true };
        SkIRect src {};
        SkIRect dst {};
    } m_regions[Region_Last];
};

#endif // AKNINEPATCH_H
