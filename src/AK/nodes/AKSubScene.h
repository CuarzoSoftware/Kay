#ifndef AKSUBSCENE_H
#define AKSUBSCENE_H

#include <AK/nodes/AKBakeable.h>
#include <AK/AKScene.h>

class AK::AKSubScene : public AKBakeable
{
public:
    AKSubScene(AKNode *parent = nullptr) noexcept;

protected:
    void bakeChildren(OnBakeParams *params) noexcept;
    void onBake(OnBakeParams *params) override;
private:
    AKScene m_scene;
    std::unordered_map<AKTarget*, AKTarget*> m_sceneTargets;
    using AKNode::setClipsChildren;
};

#endif // AKSUBSCENE_H
