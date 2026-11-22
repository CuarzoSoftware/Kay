#ifndef CZ_AKLEVELINDICATOR_H
#define CZ_AKLEVELINDICATOR_H

#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/Core/CZAdaptive.h>
#include <memory>
#include <vector>

/**
 * @brief A row of segments lit up to a level, like macOS's input level meter.
 * @ingroup AKNodes
 *
 * [■■■■■□□□□□]
 *
 * The level goes from 0 to 1: the first `round(level × count)` segments are lit (see setValue()).
 */
class CZ::AKLevelIndicator : public AKContainer
{
public:
    /** @brief Creates it with @p count segments, none lit. */
    AKLevelIndicator(UInt32 count = 15, AKNode *parent = nullptr) noexcept;

    static constexpr SkScalar SegmentWidth  { 8.f };
    static constexpr SkScalar SegmentHeight { 18.f };
    static constexpr SkScalar Gap           { 12.f };

    /** @brief The level, clamped to [0, 1]. */
    void setValue(SkScalar value) noexcept;
    SkScalar value() const noexcept { return m_value; }

    /** @brief The number of segments. */
    void setCount(UInt32 count) noexcept;
    UInt32 count() const noexcept { return UInt32(m_segments.size()); }

    /** @brief Lit segments' color (the accent color by default). */
    void setOnColor(CZAdaptiveColor color) noexcept;

    /** @brief Unlit segments' color. */
    void setOffColor(CZAdaptiveColor color) noexcept;

protected:
    void colorSchemeEvent(const CZColorSchemeEvent &e) override;
    /// @cond INTERNAL
    void updateColors() noexcept;
    std::vector<std::unique_ptr<AKRoundSolidColor>> m_segments;
    CZAdaptiveColor m_onColor, m_offColor { 0x1A000000, 0x1FFFFFFF };
    SkScalar m_value { 0.f };
    UInt32 m_lit { 0 };
    /// @endcond
};

#endif // CZ_AKLEVELINDICATOR_H
