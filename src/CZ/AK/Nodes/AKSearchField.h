#ifndef CZ_AKSEARCHFIELD_H
#define CZ_AKSEARCHFIELD_H

#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Nodes/AKCoreTextEditor.h>
#include <CZ/AK/Nodes/AKFontIcon.h>
#include <CZ/AK/Nodes/AKText.h>
#include <CZ/Core/CZAdaptive.h>
#include <CZ/Core/CZSignal.h>

/**
 * @brief macOS-style search field: a rounded box with a magnifying glass, a placeholder and a
 *        clear button.
 * @ingroup AKNodes
 *
 * A single-line AKCoreTextEditor (editor) between a "search" icon and a clear button. The
 * placeholder shows while the text is empty, the clear button while it isn't. Clicking anywhere on
 * the field focuses the editor, Escape clears the text (and then unfocuses it).
 *
 * It's as wide as its layout says (it stretches in a column by default) and Height tall.
 */
class CZ::AKSearchField : public AKRoundSolidColor
{
public:
    /** @brief Creates the field with @p placeholder ("Search" by default). */
    AKSearchField(const std::string &placeholder = "Search", AKNode *parent = nullptr) noexcept;

    static constexpr SkScalar Height   { 28.f }; ///< Default height
    static constexpr SkScalar FontSize { 13.f };

    /** @brief The text typed (UTF-8). */
    const std::string &text() const noexcept { return editor.text(); }

    /** @brief Replaces the text (emits onTextChanged if it changes). */
    void setText(const std::string &text) noexcept;

    /** @brief Clears the text (emits onTextChanged if it wasn't empty). */
    void clear() noexcept { setText(""); }

    /** @brief The text shown while empty. */
    void setPlaceholder(const std::string &text) noexcept { placeholder.setText(text); }

    /** @brief Fill of the box (adaptive). A translucent gray by default, for vibrancy backgrounds. */
    void setFillColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor fillColor() const noexcept { return m_fillColor; }

    /** @brief Emitted after the text changes (typed, pasted, cleared...). */
    CZSignal<const std::string&> onTextChanged;

    /** @brief Emitted when Enter is pressed. */
    CZSignal<const std::string&> onSubmit;

    AKFontIcon searchIcon { "search", 16, this };   ///< The magnifying glass, at the left
    AKCoreTextEditor editor { "", this };           ///< Where the text is edited (flex: 1)
    AKText placeholder { "", this };                ///< Over the editor while the text is empty
    AKFontIcon clearButton { "cancel", 14, this };  ///< At the right, while there's text

protected:
    void pointerButtonEvent(const CZPointerButtonEvent &e) override;
    void pointerEnterEvent(const CZPointerEnterEvent &e) override;
    void pointerMoveEvent(const CZPointerMoveEvent &e) override;
    void colorSchemeEvent(const CZColorSchemeEvent &e) override;
    /// @cond INTERNAL
    void updateColors() noexcept;
    void updateVisibility() noexcept;
    void updateCursor() noexcept; // Text, or the default one over the clear button
    bool overClearButton() const noexcept;
    CZAdaptiveColor m_fillColor { 0x26000000, 0x2EFFFFFF };
    /// @endcond
};

#endif // CZ_AKSEARCHFIELD_H
