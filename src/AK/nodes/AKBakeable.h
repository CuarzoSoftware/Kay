#ifndef AKBAKEABLE_H
#define AKBAKEABLE_H

#include <AK/nodes/AKRenderable.h>

/**
 * @brief A bakeable node.
 *
 * Bakeable nodes contain their own AKSurface(s) where rendering can be performed during an onBake() event using its SkCanvas.
 * The surfaces are then blitted into the main target using the AKPainter shaders during the default implementation of onRender().
 *
 * The main idea of AKBakeables is to render complex components once and then let AKScene handle the composition if the
 * node, for example, only changes position. Having a custom surface also allows for more sophisticated effects such as applying masks,
 * opacity to a tree of nodes (like AKSubScene does), and so on.
 *
 * onBake() is always called during AKScene::render(), except if the node is not visible or completely occluded. It depends
 * on its own logic to prevent re-rendering itself each time. OnBakeParams::damage indicates if there are regions that need
 * to be repainted, which only happens when the node appears on a target for the first time or the node is resized.
 * For larger nodes, OnBakeParams::clip can be useful as it indicates which regions of the node are currently exposed (not occluded by other
 * nodes or clipped by the viewport).
 *
 * An AKSurface is automatically created for each AKTarget the node is displayed in, ensuring a pixel-perfect
 * rendering based on the AKTarget scaling factor.
 *
 * To improve performance, the surfaces only grow in size when the node is resized. AKSurface::shrink() must be
 * manually called if memory usage needs to be reduced.
 */
class AK::AKBakeable : public AKRenderable
{
public:

    enum Changes
    {
        Chg_Last = AKRenderable::Chg_Last,
    };

    /**
     * @brief Parameters of an onBake() event.
     *
     * This structure contains the parameters needed during an onBake() event.
     */
    struct OnBakeParams
    {
        /**
         * @brief Region in node-local coordinates that is not currently occluded (never nullptr).
         */
        const SkRegion *clip;

        /**
         * @brief Region in node-local coordinates used by both the AKScene to explicitly indicate regions that need
         * to be repainted, and to indicate new damage generated during the onBake() event (never nullptr).
         * The resulting damage should be the union of the incoming damage and the newly generated damage.
         */
        SkRegion *damage;

        /**
         * @brief The resulting opaque region during onBake() in node-local coordinates, persistent across calls (never nullptr).
         * Should be updated only when required.
         */
        SkRegion *opaque;

        /**
         * @brief The surface to render to.
         */
        std::shared_ptr<AKSurface> surface;
    };

    /**
     * @brief AKSurface associated with an AKTarget.
     *
     * @note During an onBake() or onRender() event, there is always a surface for the AKNode::currentTarget().
     *
     * @return The AKSurface of the node associated with a target, or `nullptr` if there isn't one.
     */
    std::shared_ptr<AKSurface> getSurface(AKTarget *target) const noexcept;

protected:
    friend class AKScene;
    /**
     * @brief Constructor
     */
    AKBakeable(AKNode *parent = nullptr) noexcept : AKRenderable(Texture, parent) { m_caps |= Bake; }

    /**
     * @brief Bake event
     *
     * Triggered after onSceneBegin(), onSceneCalculatedRect() and before onRender().
     */
    virtual void onBake(OnBakeParams *params) = 0;
    virtual void onRender(AKPainter *, const SkRegion &damage) override;
};

#endif // AKBAKEABLE_H
