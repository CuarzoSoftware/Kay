#ifndef CZ_AKROUNDCONTAINER_H
#define CZ_AKROUNDCONTAINER_H

#include <CZ/AK/Nodes/AKSubScene.h>
#include <CZ/AK/Effects/AKRoundCornersEffect.h>
#include <CZ/skia/core/SkPath.h>

class CZ::AKRoundContainer : public AKSubScene
{
public:
    AKRoundContainer(const AKBorderRadius &radius, AKNode *parent = nullptr) noexcept :
        AKSubScene(parent),
        m_borderRadiusEffect(radius) {}

    CZ_DISABLE_COPY(AKRoundContainer)

    const AKRoundCornersEffect &borderRadius() const noexcept
    {
        return m_borderRadiusEffect;
    }

    AKRoundCornersEffect &borderRadius() noexcept
    {
        return m_borderRadiusEffect;
    }
protected:
    void bakeEvent(const AKBakeEvent &event) override;
private:
    AKRoundCornersEffect m_borderRadiusEffect;
};

#endif // CZ_AKROUNDCONTAINER_H
