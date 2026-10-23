#ifndef CZ_AKSUBSCENE_H
#define CZ_AKSUBSCENE_H

#include <CZ/AK/Nodes/AKBakeable.h>
#include <CZ/AK/AKScene.h>

/**
 * @brief A node that renders its children into its own framebuffer.
 *
 * An internal nested AKScene bakes the children into the node's surface (with its
 * own damage tracking), and the result is composited into the target as a single
 * image. This enables post-processing of whole subtrees (opacity, masks, effects)
 * and avoids re-compositing complex subtrees that rarely change.
 *
 * Children are always clipped to the node's bounds.
 *
 * @ingroup AKNodes
 */
class CZ::AKSubScene : public AKBakeable
{
public:
    /**
     * @brief Changes tracked in addition to AKBakeable::Changes.
     */
    enum class Changes
    {
        CHLast = AKBakeable::CHLast, ///< Marker for subclass chaining.
    };

    /**
     * @brief Constructor.
     */
    AKSubScene(AKNode *parent = nullptr) noexcept;

protected:
    /**
     * @brief Renders the children into the node's surface using the internal scene.
     *
     * Subclasses overriding bakeEvent() should call this first, then post-process
     * the surface if needed.
     */
    void bakeChildren(const AKBakeEvent &event) noexcept;

    /**
     * @brief Bakes the children (see bakeChildren()).
     */
    void bakeEvent(const AKBakeEvent &event) override { bakeChildren(event); }
private:
    friend class AKScene;
    void handleParentSceneNotifyBegin();
    std::shared_ptr<AKScene> m_scene { AKScene::MakeSubScene() };
    std::shared_ptr<AKTarget> m_target;
    using AKNode::enableChildrenClipping; // Always true
};

#endif // CZ_AKSUBSCENE_H
