#ifndef CZ_AKSLIDER_H
#define CZ_AKSLIDER_H

#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Nodes/AKImageFrame.h>
#include <CZ/Core/CZAdaptive.h>
#include <CZ/Core/CZSignal.h>
#include <memory>
#include <vector>

/**
 * @brief Horizontal slider, like macOS Control Center's (a thick pill track filled up to a round knob).
 * @ingroup AKNodes
 *
 * The value goes from 0 to 1. Pressing anywhere on the track jumps to that value and dragging (the
 * pointer is grabbed until released) follows the cursor. An optional icon is drawn inside the
 * track's left end, over the fill (e.g. a speaker for a volume slider).
 *
 * Its size is set through its layout like any node (e.g. a width percent and a fixed height): the
 * knob is as big as the height, the track fully rounded.
 *
 * With the Linear style it's like macOS's settings sliders instead: a thin track (filled up to the
 * knob, unless the fill is transparent), optionally with evenly spaced tick marks (setTickCount()).
 */
class CZ::AKSlider : public AKContainer
{
public:
    /** @brief How it looks. */
    enum class Style
    {
        Pill,  ///< A thick track as tall as the knob, the icon inside it (Control Center)
        Linear ///< A thin track through the middle of the knob, with optional tick marks (settings)
    };

    /** @brief Constructs a slider with value 0. Its default height is 22. */
    AKSlider(AKNode *parent = nullptr) noexcept;

    /**
     * @brief Sets the value (clamped to [0, 1]), without emitting onValueChanged.
     *
     * Ignored while the user drags the slider, so external updates (e.g. the system volume
     * changing) don't fight the drag.
     *
     * @return `true` if the value changed, `false` otherwise.
     */
    bool setValue(SkScalar value) noexcept;

    /** @brief The current value, in [0, 1]. */
    SkScalar value() const noexcept { return m_value; }

    /** @brief Sets the style (Pill by default). The icon is only shown by the Pill style. */
    void setStyle(Style style) noexcept;
    Style style() const noexcept { return m_style; }

    /**
     * @brief Tick marks along the track (Linear style only), from end to end: 0 for none (default).
     *
     * Only marks: the value isn't snapped to them.
     */
    void setTickCount(UInt32 count) noexcept;
    UInt32 tickCount() const noexcept { return UInt32(m_ticks.size()); }

    /** @brief Whether the user is dragging the slider. */
    bool dragging() const noexcept { return m_dragging; }

    /** @brief The icon inside the track's left end (tinted like the knob border), nullptr for none. */
    void setIcon(std::shared_ptr<RImage> icon) noexcept;

    /** @brief The icon, from images made for the size it's drawn at (see AKImageFrame::setImageProvider()). */
    void setIconProvider(AKImageFrame::ImageProvider provider) noexcept;

    /** @brief Track color (the unfilled part). */
    void setTrackColor(CZAdaptiveColor color) noexcept;

    /** @brief Fill color (up to the knob). */
    void setFillColor(CZAdaptiveColor color) noexcept;

    /** @brief Knob color. */
    void setKnobColor(CZAdaptiveColor color) noexcept;

    /** @brief Icon tint. */
    void setIconColor(CZAdaptiveColor color) noexcept;
    /** @brief Tick marks color (see setTickCount()). */
    void setTickColor(CZAdaptiveColor color) noexcept;

    /** @brief Emitted when the user changes the value (press or drag), with the new value. */
    CZSignal<SkScalar> onValueChanged;

    /** @brief Emitted when the user releases the slider after changing it. */
    CZSignal<SkScalar> onReleased;

protected:
    void layoutEvent(const CZLayoutEvent &e) override;
    void colorSchemeEvent(const CZColorSchemeEvent &e) override;
    void pointerButtonEvent(const CZPointerButtonEvent &e) override;
    void pointerMoveEvent(const CZPointerMoveEvent &e) override;

private:
    void updateGeometry() noexcept;
    void updateColors() noexcept;
    void setValueByPointer() noexcept;

    AKRoundSolidColor m_track;
    AKRoundSolidColor m_fill;
    AKImageFrame m_icon;
    AKRoundSolidColor m_knob;

    CZAdaptiveColor m_trackColor { 0x1F000000, 0x2EFFFFFF };
    CZAdaptiveColor m_fillColor { 0xFFFFFFFF, 0xFFE8E8E8 };
    CZAdaptiveColor m_knobColor { 0xFFFFFFFF, 0xFFFFFFFF };
    CZAdaptiveColor m_knobStroke { 0x26000000, 0x40000000 };
    CZAdaptiveColor m_iconColor { 0x99000000, 0x99000000 };
    CZAdaptiveColor m_tickColor { 0x40000000, 0x4DFFFFFF };
    std::vector<std::unique_ptr<AKRoundSolidColor>> m_ticks;
    Style m_style { Style::Pill };
    bool m_hasIcon { false };
    bool m_customColors { false }; // Track or fill set by the user (kept when the style changes)

    SkScalar m_value { 0.f };
    bool m_dragging { false };
    bool m_changedWhileDragging { false };
};

#endif // CZ_AKSLIDER_H
