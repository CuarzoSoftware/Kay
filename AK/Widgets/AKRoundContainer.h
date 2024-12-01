#ifndef AKROUNDCONTAINER_H
#define AKROUNDCONTAINER_H

#include <AK/Widgets/AKSubScene.h>
#include <AK/AKBorderRadius.h>
#include <include/core/SkPath.h>

class AK::AKRoundContainer : public AKSubScene
{
public:
    AKRoundContainer(const AKBorderRadius &radius, AKNode *parent = nullptr) noexcept :
        AKSubScene(parent),
        m_borderRadius(radius) {}

    const AKBorderRadius borderRadius() const noexcept
    {
        return m_borderRadius;
    }

protected:
    void onBake(SkCanvas *canvas, const SkRegion &clip, bool surfaceChanged) override;

private:
    AKBorderRadius m_borderRadius;
    SkPath m_maskTopLeft;
    SkPath m_maskTopRight;
    SkPath m_maskBottomLeft;
    SkPath m_maskBottomRight;
};

#endif // AKROUNDCONTAINER_H
