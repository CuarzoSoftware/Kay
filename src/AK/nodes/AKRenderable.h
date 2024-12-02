#ifndef AKRENDERABLE_H
#define AKRENDERABLE_H

#include <AK/nodes/AKNode.h>

class AK::AKRenderable : public AKNode
{
public:

    AKRenderable(AKNode *parent = nullptr) noexcept : AKNode(parent) { m_caps |= Render; }

    void addDamage(const SkRegion &region) noexcept;
    void addDamage(const SkIRect &rect) noexcept;

    const SkRegion &damage() const noexcept;
    SkRegion opaqueRegion;

protected:
    friend class AKScene;
    friend class AKSubScene;
    virtual void onRender(SkCanvas *canvas, const SkRegion &damage, bool opaque) = 0;
};

#endif // AKRENDERABLE_H
