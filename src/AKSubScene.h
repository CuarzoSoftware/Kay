#ifndef AKSUBSCENE_H
#define AKSUBSCENE_H

#include <AKBakeable.h>
#include <AKScene.h>

class AK::AKSubScene : public AKBakeable
{
public:
    AKSubScene(AKNode *parent = nullptr) noexcept;
    virtual void onBake(SkCanvas *canvas, const SkRect &clip, bool surfaceChanged) override;
private:
    AKScene m_scene;
    std::unordered_map<AKTarget*, AKTarget*> m_targets;
};

#endif // AKSUBSCENE_H
