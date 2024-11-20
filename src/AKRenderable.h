#ifndef AKRENDERABLE_H
#define AKRENDERABLE_H

#include <AKNode.h>

class AK::AKRenderable : public AKNode
{
public:
    AKRenderable(AKNode *parent = nullptr) noexcept : AKNode(parent) { m_caps |= Render; }

    void setDirtyRegion(SkRegion *region) noexcept
    {
        m_dirtyRegion = region ? std::make_unique<SkRegion>(*region) : nullptr;
    }

    SkRegion *dirtyRegion() const noexcept
    {
        return m_dirtyRegion.get();
    }

    void setOpaqueRegion(SkRegion *region) noexcept
    {
        m_opaqueRegion = region ? std::make_unique<SkRegion>(*region) : nullptr;
    }

    SkRegion *opaqueRegion() const noexcept
    {
        return m_opaqueRegion.get();
    }

protected:
    virtual void onRender(SkCanvas *canvas) = 0;

private:
    std::unique_ptr<SkRegion> m_dirtyRegion, m_opaqueRegion;
};

#endif // AKRENDERABLE_H
