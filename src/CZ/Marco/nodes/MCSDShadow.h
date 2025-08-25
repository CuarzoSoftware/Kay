#ifndef MCSDSHADOW_H
#define MCSDSHADOW_H

#include <CZ/Marco/Marco.h>
#include <CZ/Marco/MTheme.h>
#include <CZ/AK/Nodes/AKRenderable.h>
#include <CZ/CZWeak.h>

class CZ::MCSDShadow : public AKRenderable
{
public:
    MCSDShadow(MToplevel *toplevel) noexcept;
protected:
    void layoutEvent(const CZLayoutEvent &e) override;
    void renderEvent(const AKRenderEvent &p) override;
    CZWeak<MToplevel> m_toplevel;
    std::shared_ptr<RImage> m_image;
    CZBitset<MTheme::ShadowClamp> m_clampSides;
};

#endif // MCSDSHADOW_H
