#ifndef AKBAKEABLE_H
#define AKBAKEABLE_H

#include <AK/nodes/AKRenderable.h>
#include <AK/AKSurface.h>

/**
 * @brief A bakeable node.
 * @ingroup AKNodes
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
        CHLast = AKRenderable::CHLast,
    };

    /**
     * @brief AKSurface associated with an AKTarget.
     *
     * @note During an onBake() or onRender() event, there is always a surface for the AKNode::currentTarget().
     *
     * @return The AKSurface of the node associated with a target, or `nullptr` if there isn't one.
     */
    std::shared_ptr<AKSurface> surface() const noexcept;

    bool onBakeGeneratedDamage() const noexcept { return m_onBakeGeneratedDamage; };

protected:
    friend class AKScene;
    /**
     * @brief Constructor
     */
    AKBakeable(AKNode *parent = nullptr) noexcept : AKRenderable(Texture, parent) { m_caps |= Bake; }

    AKCLASS_NO_COPY(AKBakeable)

    /**
     * @brief Bake event
     *
     * Triggered after onSceneBegin(), onSceneCalculatedRect() and before onRender().
     */
    virtual void bakeEvent(const AKBakeEvent &event) = 0;
    void renderEvent(const AKRenderEvent &event) override;
    bool event(const AKEvent &event) override;

private:
    std::shared_ptr<AKSurface> m_surface;
    bool m_onBakeGeneratedDamage;
};

#endif // AKBAKEABLE_H
