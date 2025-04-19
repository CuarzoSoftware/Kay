#ifndef AKBACKGROUNDBLUREFFECT_H
#define AKBACKGROUNDBLUREFFECT_H

#include <AK/effects/AKBackgroundEffect.h>
#include <AK/AKSurface.h>
#include <AK/AKSignal.h>
#include <AK/AKRRect.h>
#include <include/core/SkPath.h>

/**
 * @brief Background blur effect
 *
 * This effect allows blurring the background of a node with optional clipping.
 *
 * It only works if an SkImage is provided to the current AKSceneTarget and
 * shares the same backing storage as the target surface.
 */
class AK::AKBackgroundBlurEffect : public AKBackgroundEffect
{
public:

    /**
     * @brief Changes.
     */
    enum Changes
    {
        CHArea,
        CHLast
    };

    /**
     * @brief Specifies how the effect is positioned and clipped relative to the target node.
     */
    enum AreaType
    {
        FullSize,
        Region,
        RoundRect,
        Path
    };

    AKBackgroundBlurEffect(AKNode *target = nullptr) noexcept;

    AKCLASS_NO_COPY(AKBackgroundBlurEffect)

    void setFullSize() noexcept
    {
        if (m_areaType == FullSize)
            return;

        m_areaType = FullSize;
        addChange(CHArea);
    }

    void setRoundRect(const AKRRect &rRect) noexcept
    {
        m_areaType = RoundRect;
        m_rRect = rRect;
        addChange(CHArea);
    }

    const AKRRect &roundRect() const noexcept { return m_rRect; };

    void setRegion(const SkRegion &region) noexcept
    {
        m_areaType = Region;
        m_region = region;
        addChange(CHArea);
    }

    const SkRegion &region() const noexcept { return m_region; };

    void setPath(const SkPath &path) noexcept
    {
        m_areaType = Path;
        m_path = path;
        addChange(CHArea);
    }

    const SkPath &path() const noexcept { return m_path; };

    AreaType areaType() const noexcept
    {
        return m_areaType;
    }

    AKSignal<> onTargetLayoutUpdated;

protected:
    using AKBackgroundEffect::effectRect;
    void onSceneCalculatedRect() override;
    void renderEvent(const AKRenderEvent &event) override;
    void onTargetNodeChanged() override { /* Nothing to free here */ }
private:
    using AKBackgroundEffect::setStackPosition;
    AKRRect m_rRect;
    SkRegion m_region;
    SkPath m_path;
    AreaType m_areaType { FullSize };
    std::shared_ptr<AKSurface> m_blur;
    std::shared_ptr<AKSurface> m_blur2;
    std::shared_ptr<AKSurface> m_roundCorners[4]; // TL, TR, BR, BL
};

#endif // AKBACKGROUNDBLUREFFECT_H
