#ifndef AKRENDERABLE_H
#define AKRENDERABLE_H

#include <AKNode.h>

class AK::AKRenderable : public AKNode
{
public:
    AKRenderable(AKNode *parent = nullptr) noexcept : AKNode(parent) { m_caps |= Render; }

    void addDamage(const SkRegion &region) noexcept;

    void setOpaqueRegion(SkRegion *region) noexcept
    {
        m_opaqueRegion = region ? std::make_unique<SkRegion>(*region) : nullptr;
    }

    SkRegion *opaqueRegion() const noexcept
    {
        return m_opaqueRegion.get();
    }

protected:
    friend class AKScene;
    virtual void onRender(SkCanvas *canvas, const SkRegion &damage, bool opaque) = 0;

private:
    std::unique_ptr<SkRegion> m_opaqueRegion;
};

#endif // AKRENDERABLE_H
