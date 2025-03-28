#ifndef AKSOLIDCOLOR_H
#define AKSOLIDCOLOR_H

#include <AK/nodes/AKRenderable.h>
#include <AK/AKBitset.h>
#include <AK/AKBrush.h>
#include <AK/AKPen.h>

/**
 * @brief Node for displaying solid colors.
 * @ingroup AKNodes
 */
class AK::AKSolidColor : public AKRenderable
{
public:
    AKSolidColor(SkColor color, AKNode *parent = nullptr) noexcept :
        AKRenderable(SolidColor, parent)
    {
        setColorWithAlpha(color);
    }

    AKCLASS_NO_COPY(AKSolidColor)

protected:
    using AKRenderable::addDamage;
    using AKRenderable::opaqueRegion;
    void renderEvent(const AKRenderEvent &event) override;
};

#endif // AKSOLIDCOLOR_H
