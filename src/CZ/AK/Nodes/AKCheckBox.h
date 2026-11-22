#ifndef CZ_AKCHECKBOX_H
#define CZ_AKCHECKBOX_H

#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Nodes/AKFontIcon.h>
#include <CZ/AK/Nodes/AKText.h>
#include <CZ/Core/CZSignal.h>

/**
 * @brief A check box with an optional label, like macOS's.
 * @ingroup AKNodes
 *
 * [box][label]
 *
 * Clicking it (the box or the label) toggles it and emits onToggled. While checked, the box is
 * filled with the accent color and shows a check mark.
 */
class CZ::AKCheckBox : public AKContainer
{
public:
    /** @brief Creates it with @p label (empty: just the box). */
    AKCheckBox(const std::string &label = "", bool checked = false, AKNode *parent = nullptr) noexcept;

    static constexpr SkScalar BoxSize  { 14.f };
    static constexpr SkScalar FontSize { 13.f };

    /** @brief Checks or unchecks it without emitting onToggled. */
    void setChecked(bool checked) noexcept;
    bool checked() const noexcept { return m_checked; }

    /** @brief Dimmed and not clickable while disabled. */
    void setEnabled(bool enabled) noexcept;
    bool enabled() const noexcept { return m_enabled; }

    /** @brief Sets the label (empty: hidden). */
    void setLabel(const std::string &text) noexcept;

    /** @brief Toggled by the user (the new state). */
    CZSignal<bool> onToggled;

    AKRoundSolidColor box { 0, CZBorderRadius::Make(4), this };
    AKFontIcon check { "check", 12, &box };
    AKText label { "", this };

protected:
    void pointerButtonEvent(const CZPointerButtonEvent &e) override;
    void pointerLeaveEvent(const CZPointerLeaveEvent &e) override;
    void colorSchemeEvent(const CZColorSchemeEvent &e) override;
    /// @cond INTERNAL
    void updateColors() noexcept;
    bool m_checked { false }, m_enabled { true }, m_pressed { false };
    /// @endcond
};

#endif // CZ_AKCHECKBOX_H
