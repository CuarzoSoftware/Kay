#ifndef AKSOLIDCOLOR_H
#define AKSOLIDCOLOR_H

#include <AK/AKRenderable.h>
#include <AK/AKBitset.h>
#include <AK/AKBrush.h>
#include <AK/AKPen.h>

class AK::AKRenderableRect : public AKRenderable
{
public:
    AKRenderableRect(AKNode *parent = nullptr) noexcept : AKRenderable(parent) {}

    AKRenderableRect(SkColor brushColor, AKNode *parent = nullptr) noexcept : AKRenderable(parent)
    {
        m_pendingBrush.setColor(brushColor);
    }

    void setBrush(const AKBrush &brush) noexcept
    {
        m_pendingBrush = brush;
    }

    const AKBrush &brush() const noexcept
    {
        return m_pendingBrush;
    }

    void setPen(const AKPen &pen) noexcept
    {
        m_pendingPen = pen;
    }

    const AKPen &pen() const noexcept
    {
        return m_pendingPen;
    }

protected:
    void onSceneBegin() override;
    void onRender(SkCanvas *canvas, const SkRegion &damage, bool opaque) override;
    AKPen m_currentPen { false };
    AKBrush m_currentBrush { true };
    AKPen m_pendingPen { false };
    AKBrush m_pendingBrush { true };
};

#endif // AKSOLIDCOLOR_H
