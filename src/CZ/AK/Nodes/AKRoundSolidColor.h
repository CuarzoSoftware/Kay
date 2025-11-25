#ifndef AKROUNDSOLIDCOLOR_H
#define AKROUNDSOLIDCOLOR_H

#include <CZ/AK/Nodes/AKNinePatch.h>
#include <CZ/AK/AKTheme.h>

class CZ::AKRoundSolidColor : public AKNinePatch
{
public:
    enum Changes
    {
        CHBorderRadius = AKNinePatch::CHLast,
        CHBackgroundColor,
        CHStrokeColor,
        CHStrokeWidth,
        CHLast
    };

    AKRoundSolidColor(SkColor color, Int32 borderRadius, AKNode *parent = nullptr) noexcept;

    void setBackgroundColor(SkColor color) noexcept;
    SkColor backgroundColor() const noexcept { return color(); }

    void setStrokeColor(SkColor color) noexcept;
    SkColor strokeColor() const noexcept { return m_strokeColor; }

    void setStrokeWidth(Int32 width) noexcept;
    Int32 strokeWidth() const noexcept { return m_strokeWidth; }

    void setBorderRadius(Int32 borderRadius) noexcept;
    Int32 borderRadius() const noexcept { return m_borderRadius; }

protected:
    void onSceneBegin() override;
    using AKNinePatch::setCenter;
    using AKNinePatch::setImage;
    using AKRenderable::setColor;
    using AKRenderable::color;
    std::shared_ptr<AKAsset::RRect9Patch> m_asset;
    Int32 m_borderRadius { 8 };
    Int32 m_strokeWidth { 0 };
    SkColor m_strokeColor { SK_ColorBLACK };
};

#endif // AKROUNDSOLIDCOLOR_H
