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
    AKCLASS_NO_COPY(AKSubScene)

protected:
    void bakeChildren(const AKBakeEvent &event) noexcept;
    void bakeEvent(const AKBakeEvent &event) override;
private:
    friend class AKScene;
    void handleParentSceneNotifyBegin();
    AKScene m_scene { true };
    AKWeak<AKTarget> m_target;
    using AKNode::enableChildrenClipping; // Always true
};

#endif // AKSUBSCENE_H
