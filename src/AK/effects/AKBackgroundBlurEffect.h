#ifndef AKBACKGROUNDBLUREFFECT_H
#define AKBACKGROUNDBLUREFFECT_H

#include <AK/effects/AKBackgroundEffect.h>
#include <AK/AKSurface.h>
#include <AK/AKSignal.h>
#include <AK/AKRRect.h>
#include <skia/core/SkPath.h>

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
        CHClip,
        CHLast
    };

    /**
     * @brief Specifies how the effect is positioned and clipped relative to the target node.
     */
    enum AreaType
    {
        FullSize,
        Region,
    };

    enum ClipType
    {
        NoClip,
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

    void setRegion(const SkRegion &region) noexcept
    {
        m_areaType = Region;
        m_userRegion = region;
        addChange(CHArea);
    }

    void clearClip() noexcept
    {
        if (m_clipType == NoClip)
            return;

        m_clipType = NoClip;
        addChange(CHClip);
    }

    const SkRegion &region() const noexcept { return m_userRegion; };

    void setRoundRectClip(const AKRRect &rRect) noexcept
    {
        if (m_clipType == RoundRect && rRect == m_rRectClip)
            return;

        m_clipType = RoundRect;
        m_rRectClip = rRect;
        addChange(CHClip);
    }

    const AKRRect &roundRectClip() const noexcept { return m_rRectClip; };


    void setPathClip(const SkPath &path) noexcept
    {
        m_clipType = Path;
        m_pathClip = path;
        addChange(CHClip);
    }

    const SkPath &pathClip() const noexcept { return m_pathClip; };

    AreaType areaType() const noexcept
    {
        return m_areaType;
    }

    ClipType clipType() const noexcept
    {
        return m_clipType;
    }

    AKSignal<> onTargetLayoutUpdated;

    UInt32 shader { 1 };

protected:
    using AKBackgroundEffect::effectRect;
    void onSceneCalculatedRect() override;
    void renderEvent(const AKRenderEvent &event) override;
    void onTargetNodeChanged() override { /* Nothing to free here */ }
private:
    using AKBackgroundEffect::setStackPosition;
    SkRegion m_userRegion, m_finalRegion;
    AKRRect m_rRectClip;
    SkPath m_pathClip;
    AreaType m_areaType { FullSize };
    ClipType m_clipType { NoClip };
    std::shared_ptr<AKSurface> m_blur;
    std::shared_ptr<AKSurface> m_blur2;
    std::shared_ptr<AKSurface> m_roundCorners[4]; // TL, TR, BR, BL
};

#endif // AKBACKGROUNDBLUREFFECT_H
