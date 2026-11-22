#ifndef CZ_AKFORMROW_H
#define CZ_AKFORMROW_H

#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Nodes/AKSolidColor.h>
#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKText.h>
#include <CZ/Core/CZSignal.h>

/**
 * @brief A row of an AKFormSection: [leading][title / subtitle][trailing].
 * @ingroup AKNodes
 *
 * Like the rows of macOS's settings. leading and trailing are rows for anything (an icon, a check
 * mark, a switch, buttons...), the texts column holds the title and an optional subtitle (and can
 * hold more lines). Rows after the first one of their section draw a separator above them.
 *
 * Clickable rows (setClickable()) highlight while hovered and emit onClicked.
 */
class CZ::AKFormRow : public AKRoundSolidColor
{
public:
    /** @brief Creates the row with @p title. */
    AKFormRow(const std::string &title = "", AKNode *parent = nullptr) noexcept;

    static constexpr SkScalar MinHeight { 40.f };
    static constexpr SkScalar Padding   { 10.f }; ///< Horizontal padding (the separator is inset by it too)

    /** @brief Sets the subtitle (empty: hidden). */
    void setSubtitle(const std::string &subtitle) noexcept;

    /** @brief Highlights while hovered and emits onClicked when clicked. */
    void setClickable(bool clickable) noexcept;
    bool clickable() const noexcept { return m_clickable; }

    /** @brief Clicked (when clickable()). */
    CZSignal<const CZPointerButtonEvent&> onClicked;

    AKSolidColor separator { 0, this };                         ///< At the top (see AKFormSection)
    AKContainer leading { YGFlexDirectionRow, false, this };    ///< Before the texts
    AKContainer texts { YGFlexDirectionColumn, false, this };   ///< The title and subtitle (flex: 1)
    AKText title { "", &texts };
    AKText subtitle { "", &texts };
    AKContainer trailing { YGFlexDirectionRow, false, this };   ///< After the texts

protected:
    void pointerEnterEvent(const CZPointerEnterEvent &e) override;
    void pointerLeaveEvent(const CZPointerLeaveEvent &e) override;
    void pointerButtonEvent(const CZPointerButtonEvent &e) override;
    void colorSchemeEvent(const CZColorSchemeEvent &e) override;
    /// @cond INTERNAL
    void updateColors() noexcept;
    bool m_clickable { false }, m_hover { false }, m_pressed { false };
    /// @endcond
};

#endif // CZ_AKFORMROW_H
