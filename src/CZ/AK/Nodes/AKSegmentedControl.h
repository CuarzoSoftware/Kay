#ifndef CZ_AKSEGMENTEDCONTROL_H
#define CZ_AKSEGMENTEDCONTROL_H

#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Nodes/AKText.h>
#include <CZ/AK/Nodes/AKFontIcon.h>
#include <CZ/Core/CZSignal.h>
#include <memory>
#include <string>
#include <vector>

/**
 * @brief A row of mutually exclusive segments, like macOS's segmented control.
 * @ingroup AKNodes
 *
 * [ Output | Input ]
 *
 * One segment is selected at a time: it's raised (a lighter fill with a hairline). Clicking another
 * one selects it and emits onSelected. The segments share the control's width equally, so give it
 * a width (e.g. a percent) to make it wider than its labels.
 *
 * Segments can show icons of the theme's icon font instead of text (setIconItems()), like the view
 * switcher of Finder's toolbar: [ ▦ | ☰ ].
 */
class CZ::AKSegmentedControl : public AKRoundSolidColor
{
public:
    /** @brief Creates it with @p items (the first one selected). */
    AKSegmentedControl(const std::vector<std::string> &items = {}, AKNode *parent = nullptr) noexcept;

    static constexpr SkScalar Height   { 24.f };
    static constexpr SkScalar FontSize { 13.f };
    static constexpr size_t   None     { size_t(-1) };

    /** @brief Replaces the segments, selecting @p selected (None: none). */
    void setItems(const std::vector<std::string> &items, size_t selected = 0) noexcept;
    size_t count() const noexcept { return m_segments.size(); }

    /** @brief Replaces the segments with icons of the theme's icon font (e.g. "grid_view"), selecting @p selected. */
    void setIconItems(const std::vector<std::string> &iconNames, size_t selected = 0) noexcept;

    static constexpr UInt32 IconSize { 16 }; ///< Of icon segments

    /** @brief Selects the segment at @p index without emitting onSelected (None or out of range: none). */
    void setSelectedIndex(size_t index) noexcept;
    size_t selectedIndex() const noexcept { return m_selected; }

    /** @brief Dimmed and not clickable while disabled. */
    void setEnabled(bool enabled) noexcept;
    bool enabled() const noexcept { return m_enabled; }

    /** @brief The user selected the segment at this index. */
    CZSignal<size_t> onSelected;

protected:
    void pointerButtonEvent(const CZPointerButtonEvent &e) override;
    void pointerMoveEvent(const CZPointerMoveEvent &e) override;
    void pointerLeaveEvent(const CZPointerLeaveEvent &e) override;
    void colorSchemeEvent(const CZColorSchemeEvent &e) override;
    /// @cond INTERNAL
    struct Segment
    {
        Segment(const std::string &text, bool icon, AKNode *parent) noexcept;
        AKRoundSolidColor bg;
        AKText label;
        std::unique_ptr<AKFontIcon> icon; // Icon segments
    };
    size_t segmentAt(SkScalar x) const noexcept;
    void updateColors() noexcept;
    std::vector<std::unique_ptr<Segment>> m_segments;
    size_t m_selected { None }, m_pressed { None }, m_hover { None };
    bool m_enabled { true };
    /// @endcond
};

#endif // CZ_AKSEGMENTEDCONTROL_H
