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
        CHLast
    };

    AKRoundSolidColor(SkColor color, Int32 borderRadius, AKNode *parent = nullptr) noexcept;
    void setBorderRadius(Int32 borderRadius) noexcept;
    Int32 borderRadius() const noexcept { return m_borderRadius; }

protected:
    void onSceneBegin() override;
    using AKNinePatch::setCenter;
    using AKNinePatch::setImage;
    std::shared_ptr<AKAsset::RRect9Patch> m_asset;
    Int32 m_borderRadius { 8 };
};

#endif // AKROUNDSOLIDCOLOR_H
