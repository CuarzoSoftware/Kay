#ifndef AKSUBSCENE_H
#define AKSUBSCENE_H

#include <AK/nodes/AKBakeable.h>
#include <AK/AKScene.h>

class AK::AKSubScene : public AKBakeable
{
public:
    enum class Changes
    {
        CHLast = AKBakeable::CHLast,
    };

    AKSubScene(AKNode *parent = nullptr) noexcept;

protected:
    void bakeChildren(OnBakeParams *params) noexcept;
    void onBake(OnBakeParams *params) override;
private:
    friend class AKScene;
    void handleParentSceneNotifyBegin();
    AKScene m_scene;
    AKWeak<AKTarget> m_target;
    using AKNode::enableChildrenClipping; // Always true
};

#endif // AKSUBSCENE_H
