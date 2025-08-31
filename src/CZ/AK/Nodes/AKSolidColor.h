#ifndef CZ_AKSOLIDCOLOR_H
#define CZ_AKSOLIDCOLOR_H

#include <CZ/AK/Nodes/AKRenderable.h>
#include <CZ/Core/CZBitset.h>
#include <CZ/skia/core/SkPaint.h>

/**
 * @brief Node for displaying solid colors.
 */
class CZ::AKSolidColor : public AKRenderable
{
public:
    AKSolidColor(SkColor color, AKNode *parent = nullptr) noexcept :
        AKRenderable(RenderableHint::SolidColor, parent)
    {
        setColor(color);
    }

protected:
    using AKRenderable::addDamage;
    using AKRenderable::opaqueRegion;
    void renderEvent(const AKRenderEvent &event) override;
};

#endif // CZ_AKSOLIDCOLOR_H
