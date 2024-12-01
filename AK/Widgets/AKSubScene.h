#ifndef AKSUBSCENE_H
#define AKSUBSCENE_H

#include <AK/AKBakeable.h>
#include <AK/AKScene.h>

class AK::AKSubScene : public AKBakeable
{
public:
    AKSubScene(AKNode *parent = nullptr) noexcept;

protected:
    void bakeChildren(SkCanvas *canvas, const SkRegion &clip, bool surfaceChanged,
                      const SkRegion *inDamageRegion = nullptr) noexcept;
    void onBake(SkCanvas *canvas, const SkRegion &clip, bool surfaceChanged) override;
private:
    AKScene m_scene;
    std::unordered_map<AKTarget*, AKTarget*> m_targets;
    using AKNode::setClipsChildren;
};

#endif // AKSUBSCENE_H
