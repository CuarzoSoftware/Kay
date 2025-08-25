#ifndef CZ_AKBAKEABLE_H
#define CZ_AKBAKEABLE_H

#include <CZ/AK/Nodes/AKRenderable.h>
#include <CZ/Ream/RSurface.h>

/**
 * @brief A bakeable node.
 * @ingroup AKNodes
 *
 * Bakeable nodes contain their own RSurface(s) where rendering can be performed during an onBake() event using its SkCanvas.
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
 * An RSurface is automatically created for each AKTarget the node is displayed in, ensuring a pixel-perfect
 * rendering based on the AKTarget scaling factor.
 *
 * To improve performance, the surfaces only grow in size when the node is resized. RSurface::shrink() must be
 * manually called if memory usage needs to be reduced.
 */
class CZ::AKBakeable : public AKRenderable
{
public:

    enum Changes
    {
        CHLast = AKRenderable::CHLast,
    };

    /**
     * @brief RSurface associated with an AKTarget.
     *
     * @note During an onBake() or onRender() event, there is always a surface for the AKNode::currentTarget().
     *
     * @return The RSurface of the node associated with a target, or `nullptr` if there isn't one.
     */
    std::shared_ptr<RSurface> surface() const noexcept;

    bool onBakeGeneratedDamage() const noexcept { return m_onBakeGeneratedDamage; };

protected:
    friend class AKScene;
    /**
     * @brief Constructor
     */
    AKBakeable(AKNode *parent = nullptr) noexcept : AKRenderable(RenderableHint::Image, parent) {
        m_caps |= BakeableBit;
    }

    /**
     * @brief Bake event
     *
     * Triggered after onSceneBegin(), onSceneCalculatedRect() and before onRender().
     */
    virtual void bakeEvent(const AKBakeEvent &event) = 0;
    void renderEvent(const AKRenderEvent &event) override;
    bool event(const CZEvent &event) noexcept override;

private:
    std::shared_ptr<RSurface> m_surface;
    Int32 m_surfaceScale { 1 };
    bool m_onBakeGeneratedDamage;
};

#endif // CZ_AKBAKEABLE_H
