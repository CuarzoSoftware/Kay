#ifndef AKRENDERABLE_H
#define AKRENDERABLE_H

#include <AK/AKNode.h>

class AK::AKRenderable : public AKNode
{
public:

    AKRenderable(AKNode *parent = nullptr) noexcept : AKNode(parent) { m_caps |= Render; }

    void addDamage(const SkRegion &region) noexcept;
    void addDamage(const SkIRect &rect) noexcept;

    const SkRegion &damage() const noexcept;

    void setOpaqueRegion(const SkIRect &rect) noexcept
    {
        m_opaqueRegion.setRect(rect);
    }

    void setOpaqueRegion(const SkRegion &region) noexcept
    {
        m_opaqueRegion = region;
    }

    const SkRegion &opaqueRegion() const noexcept
    {
        return m_opaqueRegion;
    }

protected:
    friend class AKScene;
    friend class AKSubScene;
    virtual void onRender(SkCanvas *canvas, const SkRegion &damage, bool opaque) = 0;
private:
    SkRegion m_opaqueRegion;
};

#endif // AKRENDERABLE_H