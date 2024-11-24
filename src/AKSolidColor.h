#ifndef AKSOLIDCOLOR_H
#define AKSOLIDCOLOR_H

#include <AKRenderable.h>
#include <include/core/SkColor.h>

class AK::AKSolidColor : public AKRenderable
{
public:
    AKSolidColor(SkColor color, AKNode *parent = nullptr) noexcept : AKRenderable(parent),
        m_color(color)
    {}

    SkColor color() const noexcept
    {
        return m_color;
    }

private:
    void onRender(SkCanvas *canvas, const SkRegion &damage, bool opaque) override;
    SkColor m_color;
};

#endif // AKSOLIDCOLOR_H
