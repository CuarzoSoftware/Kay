#ifndef AKBAKEABLE_H
#define AKBAKEABLE_H

#include <AKRenderable.h>

class AK::AKBakeable : public AKRenderable
{
protected:
    friend class AKScene;
    AKBakeable(AKNode *parent = nullptr) noexcept : AKRenderable(parent) { m_caps |= Bake; }
    virtual void onBake(SkCanvas *canvas, const SkRect &clip, bool surfaceChanged) = 0;

    void rebake() noexcept
    {
        m_pendingRebake = true;
    }

private:
    virtual void onRender(SkCanvas *canvas, const SkRegion &damage, bool opaque) override;
    bool m_pendingRebake { true };
};

#endif // AKBAKEABLE_H
