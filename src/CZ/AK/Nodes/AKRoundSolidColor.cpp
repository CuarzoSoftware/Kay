#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/AKApp.h>

using namespace CZ;

AKRoundSolidColor::AKRoundSolidColor(SkColor color, Int32 borderRadius, AKNode *parent) noexcept : AKNinePatch({}, {}, parent)
{
    enableReplaceImageColor(true);
    setColor(color);
    setBorderRadius(borderRadius);
}

void AKRoundSolidColor::setBackgroundColor(SkColor color) noexcept
{
    if (color == backgroundColor())
        return;

    setColor(color);
    addChange(CHBackgroundColor);
}

void AKRoundSolidColor::setStrokeColor(SkColor color) noexcept
{
    if (color == m_strokeColor)
        return;

    m_strokeColor = color;
    addChange(CHStrokeColor);
}

void AKRoundSolidColor::setStrokeWidth(Int32 width) noexcept
{
    if (width < 0) width = 0;
    if (width == m_strokeWidth)
        return;
    m_strokeWidth = width;
    addChange(CHStrokeWidth);
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

    if (ch.testAnyOf(CHStrokeColor, CHStrokeWidth))
        enableReplaceImageColor(m_strokeWidth == 0 || SkColorGetA(m_strokeColor) == 0);

    if (ch.testAnyOf(CHBorderRadius, CHLayoutScale, CHStrokeColor, CHStrokeWidth, CHBackgroundColor))
    {
        m_asset = theme()->RRect9Patch.get(borderRadius(), scale(), backgroundColor(), strokeWidth(), strokeColor());
        setImage(m_asset->image);
        setCenter(m_asset->center);
    }

    AKNinePatch::onSceneBegin();

    if (ch.testAnyOf(CHLayoutSize, CHImage, CHBorderRadius, CHColor, CHColorFactor, CHReplaceImageColorEnabled, CHOpacity))
    {
        if (opacity() >= 1.f && colorFactor().fA >= 1.f && SkColorGetA(color()) == 255)
        {
            opaqueRegion.setRect(m_regions[C].dst);
            opaqueRegion.op(m_regions[T].dst, SkRegion::kUnion_Op);
            opaqueRegion.op(m_regions[L].dst, SkRegion::kUnion_Op);
            opaqueRegion.op(m_regions[R].dst, SkRegion::kUnion_Op);
            opaqueRegion.op(m_regions[B].dst, SkRegion::kUnion_Op);

            if (SkColorGetA(m_strokeColor) != 255 && m_strokeWidth > 0)
            {
                const auto rect { SkIRect::MakeSize(worldRect().size()).makeInset(m_strokeWidth, m_strokeWidth) };
                opaqueRegion.op(rect, SkRegion::Op::kIntersect_Op);
            }
        }
        else
            opaqueRegion.setEmpty();
    }
}
