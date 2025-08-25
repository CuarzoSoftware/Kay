#ifndef CZ_AKSUBSCENE_H
#define CZ_AKSUBSCENE_H

#include <CZ/AK/Nodes/AKBakeable.h>
#include <CZ/AK/AKScene.h>

/**
 * @brief A node that renders its children into its own framebuffer.
 * @ingroup AKNodes
 */
class CZ::AKSubScene : public AKBakeable
{
public:
    enum class Changes
    {
        CHLast = AKBakeable::CHLast,
    };

    AKSubScene(AKNode *parent = nullptr) noexcept;

protected:
    void bakeChildren(const AKBakeEvent &event) noexcept;
    void bakeEvent(const AKBakeEvent &event) override { bakeChildren(event); }
private:
    friend class AKScene;
    void handleParentSceneNotifyBegin();
    std::shared_ptr<AKScene> m_scene { AKScene::MakeSubScene() };
    std::shared_ptr<AKTarget> m_target;
    using AKNode::enableChildrenClipping; // Always true
};

#endif // CZ_AKSUBSCENE_H
