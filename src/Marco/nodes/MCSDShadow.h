#ifndef MCSDSHADOW_H
#define MCSDSHADOW_H

#include <Marco/Marco.h>
#include <Marco/MTheme.h>
#include <AK/nodes/AKRenderable.h>
#include <AK/AKWeak.h>

class AK::MCSDShadow : public AKRenderable
{
public:
    MCSDShadow(MToplevel *toplevel) noexcept;
protected:
    void layoutEvent(const AKLayoutEvent &e) override;
    void renderEvent(const AKRenderEvent &p) override;
    AKWeak<MToplevel> m_toplevel;
    sk_sp<SkImage> m_image;
    AKBitset<MTheme::ShadowClamp> m_clampSides;
};

#endif // MCSDSHADOW_H
