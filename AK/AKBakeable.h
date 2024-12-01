#ifndef AKBAKEABLE_H
#define AKBAKEABLE_H

#include <AK/AKRenderable.h>

class AK::AKBakeable : public AKRenderable
{
protected:
    friend class AKScene;
    AKBakeable(AKNode *parent = nullptr) noexcept : AKRenderable(parent) { m_caps |= Bake; }
    virtual void onBake(SkCanvas *canvas, const SkRegion &clip, bool surfaceChanged) = 0;
private:
    virtual void onRender(SkCanvas *canvas, const SkRegion &damage, bool opaque) override;
};

#endif // AKBAKEABLE_H
