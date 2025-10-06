#ifndef CZ_AKBACKGROUNDBLUREFFECT_H
#define CZ_AKBACKGROUNDBLUREFFECT_H

#include <CZ/AK/Effects/AKBackgroundEffect.h>
#include <CZ/Ream/RSurface.h>
#include <CZ/Core/CZSignal.h>
#include <CZ/Core/CZRRect.h>
#include <CZ/skia/core/SkPath.h>

/**
 * @brief Background blur effect
 *
 * Applies a blur to the background of a node, with optional area and shape clipping.
 *
 * The effect can be constrained by two independent parameters:
 *
 * - **Area**: Determines the region affected by the blur. It can be set to `FullSize`
 *   to match the dimensions of the target node, or `Region` to specify a custom area.
 *
 * - **Clip**: An optional shape used for additional clipping. Supported types include
 *   `RoundRect` and `Path`.
 *
 * The final blurred region is the intersection of the specified area and the clip shape.
 */
class CZ::AKBackgroundBlurEffect : public AKBackgroundEffect
{
public:

    /**
     * @brief Change flags used for tracking internal state updates.
     */
    enum Changes
    {
        CHArea,   ///< Indicates that the blur area has changed.
        CHClip,   ///< Indicates that the clipping shape has changed.
        CHLast    ///< Sentinel value (not used directly).
    };

    /**
     * @brief Defines how the blur area is determined relative to the target node.
     *
     * @see setFullSize() and setRegion()
     */
    enum AreaType
    {
        FullSize,  ///< The blur covers the full size of the target node.
        Region     ///< The blur is restricted to a custom-defined region.
    };

    /**
     * @brief Specifies the type of additional clipping applied to the blur area.
     *
     * @see setRoundRectClip(), setPathClip() and clearClip()
     */
    enum ClipType
    {
        NoClip,     ///< No additional clipping.
        RoundRect,  ///< A rounded rectangle shape is used as a clip.
        Path        ///< A custom path is used as a clip.
    };

    /**
     * @brief Constructs a new background blur effect.
     *
     * @param target The node to which this effect is applied. Can be nullptr.
     */
    AKBackgroundBlurEffect(AKNode *target = nullptr) noexcept;

    /**
     * @brief Returns the current area type (FullSize or Region).
     *
     * Defaults to AreaType::FullSize
     */
    AreaType areaType() const noexcept { return m_areaType; }

    /**
     * @brief Returns the current clip type (NoClip, RoundRect, or Path).
     *
     * Defaults to ClipType::NoClip
     */
    ClipType clipType() const noexcept { return m_clipType; }

    /**
     * @brief Sets the blur area to match the full size of the target node.
     */
    void setFullSize() noexcept;

    /**
     * @brief Sets a custom region to limit the blur area.
     *
     * The region is specified relative to the target node.
     */
    void setRegion(const SkRegion &region) noexcept;

    /**
     * @brief Returns the currently set region used for the blur area.
     */
    const SkRegion &region() const noexcept { return m_userRegion; };

    /**
     * @brief Removes any additional clipping (round rect or path).
     *
     * Note: This does not affect the blur area (FullSize or Region).
     */
    void clearClip() noexcept;

    /**
     * @brief Applies a rounded rectangle clip to further restrict the blur area.
     *
     * This replaces any previously set clip (RoundRect or Path).
     * The clip is defined relative to the target node.
     */
    void setRoundRectClip(const CZRRect &rRect) noexcept;

    /**
     * @brief Returns the currently set rounded rectangle clip.
     */
    const CZRRect &roundRectClip() const noexcept { return m_rRectClip; };

    /**
     * @brief Applies a path clip to further restrict the blur area.
     *
     * This replaces any previously set clip (RoundRect or Path).
     * The path is defined relative to the target node.
     */
    void setPathClip(const SkPath &path) noexcept;

    /**
     * @brief Returns the currently set path clip.
     */
    const SkPath &pathClip() const noexcept { return m_pathClip; };

    /**
     * @brief Signal emitted when the target node's layout changes.
     *
     * This signal should be used to update dependent data (such as the region,
     * path, etc.) when the blur area is not set to @c FullSize.
     */
    CZSignal<> onTargetLayoutUpdated;

protected:
    using AKBackgroundEffect::effectRect;
    void targetNodeRectCalculated() override;
    void renderEvent(const AKRenderEvent &event) override;
    void onTargetNodeChanged() override { /* Nothing to free here */ }
private:
    using AKBackgroundEffect::setStackPosition;
    SkRegion m_userRegion, m_finalRegion;
    CZRRect m_rRectClip;
    SkPath m_pathClip;
    AreaType m_areaType { FullSize };
    ClipType m_clipType { NoClip };
    std::shared_ptr<RSurface> m_blur;
    std::shared_ptr<RSurface> m_blur2;
};

#endif // CZ_AKBACKGROUNDBLUREFFECT_H
