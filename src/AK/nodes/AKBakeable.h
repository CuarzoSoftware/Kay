#ifndef AKBAKEABLE_H
#define AKBAKEABLE_H

#include <AK/nodes/AKRenderable.h>

class AK::AKBakeable : public AKRenderable
{
public:
    struct OnBakeParams
    {
        const SkRegion *clip;
        SkRegion *damage;
        SkRegion *opaque;
        std::shared_ptr<AKSurface> surface;
    };

protected:
    friend class AKScene;
    AKBakeable(AKNode *parent = nullptr) noexcept : AKRenderable(parent) { m_caps |= Bake; }
    virtual void onBake(OnBakeParams *params) = 0;
private:
    virtual void onRender(AKPainter *, const SkRegion &damage) override;
};

#endif // AKBAKEABLE_H
