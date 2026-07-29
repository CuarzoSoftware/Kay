#ifndef CZ_AKSOLIDCOLOR_H
#define CZ_AKSOLIDCOLOR_H

#include <CZ/AK/Nodes/AKRenderable.h>
#include <CZ/Core/CZBitset.h>
#include <CZ/Core/CZAdaptive.h>
#include <CZ/skia/core/SkPaint.h>

/**
 * @brief Node for displaying solid colors.
 *
 * The fill is a @ref CZAdaptiveColor: it is resolved for the current color scheme and re-resolved
 * automatically when the scheme changes (only when the light and dark variants actually differ).
 *
 * The inherited raw AKRenderable::setColor() (a single SkColor) is demoted to protected — use the
 * adaptive setColor()/color() below instead.
 */
class CZ::AKSolidColor : public AKRenderable
{
public:
    AKSolidColor(CZAdaptiveColor color, AKNode *parent = nullptr) noexcept :
        AKRenderable(RenderableHint::SolidColor, parent)
    {
        setColor(color);

        onColorSchemeChanged.subscribe(this, [this](CZColorScheme) {
            AKRenderable::setColor(m_adaptiveColor.resolve(colorScheme()));
        });
    }

    /** @brief Sets the adaptive fill color (resolved for the current color scheme). */
    void setColor(CZAdaptiveColor color) noexcept
    {
        m_adaptiveColor = color;
        AKRenderable::setColor(color.resolve(colorScheme()));
    }

    /** @brief The adaptive fill color. */
    CZAdaptiveColor color() const noexcept { return m_adaptiveColor; }

protected:
    using AKRenderable::addDamage;
    using AKRenderable::opaqueRegion;
    using AKRenderable::setColor;
    void renderEvent(const AKRenderEvent &event) override;

private:
    CZAdaptiveColor m_adaptiveColor;
};

#endif // CZ_AKSOLIDCOLOR_H
