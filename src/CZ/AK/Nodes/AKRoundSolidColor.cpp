#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/AKApp.h>

using namespace CZ;

AKRoundSolidColor::AKRoundSolidColor(SkColor color, Int32 borderRadius, AKNode *parent) noexcept : AKNinePatch({}, {}, parent)
{
    enableReplaceImageColor(true);
    setColor(color);
    setBorderRadius(borderRadius);
}

void AKRoundSolidColor::setBorderRadius(Int32 borderRadius) noexcept
{
    if (borderRadius < 0)
        borderRadius = 0;

    if (borderRadius == m_borderRadius)
        return;

    m_borderRadius = borderRadius;
    addChange(CHBorderRadius);
}

void AKRoundSolidColor::onSceneBegin()
{
    const auto &ch { changes() };

    if (ch.testAnyOf(CHBorderRadius, CHLayoutScale))
    {
        m_asset = theme()->RRect9Patch.get(borderRadius(), scale());
        setImage(m_asset->image);
        setCenter(m_asset->center);
    }

    AKNinePatch::onSceneBegin();

    if (ch.testAnyOf(CHLayoutSize, CHImage, CHBorderRadius, CHColor, CHColorFactor, CHReplaceImageColorEnabled, CHOpacity))
    {
        if (opacity() >= 1.f && colorFactor().fA >= 1.f && (!replaceImageColorEnabled() || (replaceImageColorEnabled() && SkColorGetA(color()) == 255)))
        {
            opaqueRegion.setRect(m_regions[C].dst);
            opaqueRegion.op(m_regions[T].dst, SkRegion::kUnion_Op);
            opaqueRegion.op(m_regions[L].dst, SkRegion::kUnion_Op);
            opaqueRegion.op(m_regions[R].dst, SkRegion::kUnion_Op);
            opaqueRegion.op(m_regions[B].dst, SkRegion::kUnion_Op);
        }
        else
            opaqueRegion.setEmpty();
    }
}
