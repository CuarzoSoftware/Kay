#ifndef AKNINEPATCH_H
#define AKNINEPATCH_H

#include <CZ/AK/Nodes/AKRenderable.h>

class CZ::AKNinePatch : public AKRenderable
{
public:
    enum Changes
    {
        CHImage = AKRenderable::CHLast,
        CHCenter,
        CHRegionVisibility,
        CHLast
    };

    enum Region
    {
        TL, TR, BR, BL, L, T, R, B, C, Region_Last
    };

    AKNinePatch(std::shared_ptr<RImage> image = nullptr, SkIRect center = {}, AKNode *parent = nullptr) noexcept;

    bool setImage(std::shared_ptr<RImage> image) noexcept;
    std::shared_ptr<RImage> image() const noexcept { return m_image; }

    bool setCenter(SkIRect center) noexcept;
    const SkIRect &center() const noexcept { return m_center; };

    bool setRegionVisibility(Region reg, bool visible) noexcept;
    bool regionVisibility(Region reg) noexcept { return m_regions[reg].visible; }

protected:
    void onSceneBegin() override;
    void renderEvent(const AKRenderEvent &e) override;
    void updateRegions() noexcept;
    SkIRect m_center {};
    std::shared_ptr<RImage> m_image;
    struct {
        bool visible { true };
        SkIRect src {};
        SkIRect dst {};
    } m_regions[Region_Last];
};

#endif // AKNINEPATCH_H
