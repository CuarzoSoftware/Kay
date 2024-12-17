#ifndef AKBACKGROUNDBLUREFFECT_H
#define AKBACKGROUNDBLUREFFECT_H

#include <AK/effects/AKBackgroundEffect.h>
#include <AK/AKSignal.h>
#include <AK/AKBrush.h>

class AK::AKBackgroundBlurEffect : public AKBackgroundEffect
{
public:
    using AKBackgroundEffect::rect;

    AKBackgroundBlurEffect(AKNode *target = nullptr) noexcept :
        AKBackgroundEffect(Behind)
    {
        if (target)
            target->setBackgroundEffect(this);
    };

    AKSignal<> onLayoutUpdateSignal;
    SkPath clip;
    AKBrush brush;

    void onLayoutUpdate() override;
    void onRender(AKPainter *, const SkRegion &damage) override;
    void onTargetNodeChanged() override {}
};

#endif // AKBACKGROUNDBLUREFFECT_H
