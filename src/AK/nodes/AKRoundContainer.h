#ifndef AKROUNDCONTAINER_H
#define AKROUNDCONTAINER_H

#include <AK/nodes/AKSubScene.h>
#include <AK/effects/AKRoundCornersEffect.h>
#include <include/core/SkPath.h>

class AK::AKRoundContainer : public AKSubScene
{
public:
    AKRoundContainer(const AKBorderRadius &radius, AKNode *parent = nullptr) noexcept :
        AKSubScene(parent),
        m_borderRadiusEffect(radius) {}

    const AKRoundCornersEffect &borderRadius() const noexcept
    {
        return m_borderRadiusEffect;
    }

    AKRoundCornersEffect &borderRadius() noexcept
    {
        return m_borderRadiusEffect;
    }
protected:
    void onBake(OnBakeParams *params) override;
private:
    AKRoundCornersEffect m_borderRadiusEffect;
};

#endif // AKROUNDCONTAINER_H
