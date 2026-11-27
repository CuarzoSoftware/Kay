#ifndef CZ_AKINFOROW_H
#define CZ_AKINFOROW_H

#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKCoreTextEditor.h>
#include <CZ/AK/Nodes/AKText.h>
#include <string>

/**
 * @brief A label and its value side by side, like the rows of Finder's Get Info window
 *        ("Kind: Folder", "Created: Monday, 3 November 2026 at 09:14").
 * @ingroup AKNodes
 *
 * [label (right-aligned, in a column as wide as setLabelWidth())][value (selectable, wrapped)]
 *
 * Rows with the same label width line up: the labels end, and the values start, at the same
 * place. The value can be selected and copied (an AKCoreTextEditor) and wraps at setValueWidth().
 * Other nodes can be added after the value's text, in trailing (e.g. a button).
 */
class CZ::AKInfoRow : public AKContainer
{
public:
    /** @brief Creates the row with @p label and @p value. */
    AKInfoRow(const std::string &label = "", const std::string &value = "", AKNode *parent = nullptr) noexcept;

    static constexpr SkScalar FontSize          { 12.f };
    static constexpr SkScalar DefaultLabelWidth { 80.f };
    static constexpr SkScalar Gap               { 5.f };  ///< Between the label and the value

    void setLabel(const std::string &text) noexcept { label.setText(text); }
    void setValue(const std::string &text) noexcept { value.setText(text); }

    /** @brief Width of the label's column (DefaultLabelWidth by default). */
    void setLabelWidth(SkScalar width) noexcept;
    SkScalar labelWidth() const noexcept { return m_labelWidth; }

    /** @brief Width the value wraps at, or 0 (the default) to never wrap. */
    void setValueWidth(SkScalar width) noexcept { value.setWrapWidth(width); }
    SkScalar valueWidth() const noexcept { return value.wrapWidth(); }

    AKContainer labelColumn { YGFlexDirectionRow, false, this };
    AKText label { "", &labelColumn };
    AKCoreTextEditor value { "", this };

protected:
    /// @cond INTERNAL
    SkScalar m_labelWidth { DefaultLabelWidth };
    /// @endcond
};

#endif // CZ_AKINFOROW_H
