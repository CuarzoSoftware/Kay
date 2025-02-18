#ifndef AKSUBSCENE_H
#define AKSUBSCENE_H

#include <AK/nodes/AKBakeable.h>
#include <AK/AKScene.h>

/**
 * @brief A node that renders its children into its own framebuffer.
 * @ingroup AKNodes
 */
class AK::AKSubScene : public AKBakeable
{
public:
    enum class Changes
    {
        CHLast = AKBakeable::CHLast,
    };

    AKSubScene(AKNode *parent = nullptr) noexcept;

protected:
    void bakeChildren(const BakeEvent &event) noexcept;
    void onBake(const BakeEvent &event) override;
private:
    friend class AKScene;
    void handleParentSceneNotifyBegin();
    AKScene m_scene;
    AKWeak<AKTarget> m_target;
    using AKNode::enableChildrenClipping; // Always true
};

#endif // AKSUBSCENE_H
