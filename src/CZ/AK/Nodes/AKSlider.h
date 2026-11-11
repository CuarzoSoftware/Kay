#ifndef CZ_AKSLIDER_H
#define CZ_AKSLIDER_H

#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Nodes/AKImageFrame.h>
#include <CZ/Core/CZAdaptive.h>
#include <CZ/Core/CZSignal.h>

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
 */
class CZ::AKSlider : public AKContainer
{
public:
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

    /** @brief Whether the user is dragging the slider. */
    bool dragging() const noexcept { return m_dragging; }

    /** @brief The icon inside the track's left end (tinted like the knob border), nullptr for none. */
    void setIcon(std::shared_ptr<RImage> icon) noexcept;

    /** @brief Track color (the unfilled part). */
    void setTrackColor(CZAdaptiveColor color) noexcept;

    /** @brief Fill color (up to the knob). */
    void setFillColor(CZAdaptiveColor color) noexcept;

    /** @brief Knob color. */
    void setKnobColor(CZAdaptiveColor color) noexcept;

    /** @brief Icon tint. */
    void setIconColor(CZAdaptiveColor color) noexcept;

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

    SkScalar m_value { 0.f };
    bool m_dragging { false };
    bool m_changedWhileDragging { false };
};

#endif // CZ_AKSLIDER_H
