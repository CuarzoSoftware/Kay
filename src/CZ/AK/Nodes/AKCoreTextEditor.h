#ifndef CZ_AKCORETEXTEDITOR_H
#define CZ_AKCORETEXTEDITOR_H

#include <CZ/AK/Nodes/AKBakeable.h>
#include <CZ/AK/Nodes/AKSolidColor.h>
#include <CZ/Core/CZAdaptive.h>
#include <CZ/Core/CZTimer.h>
#include <CZ/Core/CZSignal.h>
#include <CZ/skia/modules/skparagraph/include/Paragraph.h>
#include <CZ/skia/modules/skparagraph/include/ParagraphStyle.h>
#include <CZ/skia/modules/skparagraph/include/TextStyle.h>
#include <memory>
#include <string>
#include <vector>

/**
 * @brief Text that can be selected and, optionally, edited.
 * @ingroup AKNodes
 *
 * The core of text input: no background, border or padding (wrap it in other nodes for that), just
 * the text, its selection highlight and a blinking caret. Use it to show text the user can select
 * and copy (selectable(), the default), or edit (setEditable()).
 *
 * #### Layout
 * The node is sized to its text. With a wrap width (setWrapWidth()) it's that wide and the text
 * wraps (lines aligned with setTextAlign()); otherwise it's as wide as the longest line. Lines are
 * only broken at `\n` while editing a single-line editor (see setMultiline()).
 * setMaxLines() ellipsizes long text while it's not being edited.
 *
 * #### Interaction
 * - Pointer: click to place the caret, drag to select (Shift+click extends), double click selects
 *   a word (dragging extends by words), triple click selects everything.
 * - Keyboard: arrows (Ctrl: by words, Shift: extending the selection), Home/End (Ctrl: whole text),
 *   Ctrl+A, Ctrl+C. Editable: typing, Backspace/Delete (Ctrl: by words), Ctrl+X, Ctrl+V,
 *   Ctrl+Z / Ctrl+Shift+Z (or Ctrl+Y). Enter emits onSubmit (multiline: inserts a line break,
 *   Ctrl+Enter submits) and Escape emits onCancel.
 * - It takes keyboard focus when clicked, and drops it when a press lands outside it (emitting
 *   onEditingFinished).
 *
 * Copy & paste use AKApp::clipboard() (unavailable without one).
 *
 * Positions (selection, caret) are byte offsets into text() (UTF-8), always at code point boundaries.
 */
class CZ::AKCoreTextEditor : public AKBakeable
{
public:
    /** @brief Property change bits consumed on the next AKScene::render(). */
    enum Changes
    {
        CHText = AKBakeable::CHLast, ///< The text or its layout changed.
        CHSelection,                 ///< The selection changed.
        CHLast                       ///< Marker for subclass chaining.
    };

    /** @brief Creates a selectable, non-editable text. */
    AKCoreTextEditor(const std::string &text = "", AKNode *parent = nullptr) noexcept;
    ~AKCoreTextEditor() noexcept;

    /* ------------------------------------- Text ------------------------------------- */

    /**
     * @brief Replaces the text (UTF-8).
     *
     * Places the caret at the end and clears the undo history. Emits onTextChanged.
     *
     * @return `true` if the text changed.
     */
    bool setText(const std::string &text) noexcept;
    const std::string &text() const noexcept { return m_text; }

    /* ------------------------------------- Style ------------------------------------- */

    /** @brief The text style (the theme's default text style initially). Its color is ignored, see setTextColor(). */
    void setTextStyle(const skia::textlayout::TextStyle &style) noexcept;
    const skia::textlayout::TextStyle &textStyle() const noexcept { return m_style; }

    void setFontSize(SkScalar size) noexcept;
    void setFontStyle(SkFontStyle style) noexcept;

    /** @brief Color of the text and the caret (AKTheme::Text by default). */
    void setTextColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor textColor() const noexcept { return m_textColor; }

    /** @brief Selection highlight while focused (unfocused selections are gray). */
    void setSelectionColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor selectionColor() const noexcept { return m_selectionColor; }

    /** @brief Line alignment (left by default). */
    void setTextAlign(skia::textlayout::TextAlign align) noexcept;
    skia::textlayout::TextAlign textAlign() const noexcept { return m_align; }

    /**
     * @brief Width the text wraps at (the node's width), or 0 (the default) to never wrap.
     */
    void setWrapWidth(SkScalar width) noexcept;
    SkScalar wrapWidth() const noexcept { return m_wrapWidth; }

    /**
     * @brief Maximum number of lines shown while not being edited (the last one ellipsized), or 0
     *        (the default) for no limit.
     */
    void setMaxLines(size_t lines) noexcept;
    size_t maxLines() const noexcept { return m_maxLines; }

    /* ----------------------------------- Behavior ----------------------------------- */

    /** @brief Whether the user can select (and copy) the text. `true` by default. */
    void setSelectable(bool selectable) noexcept;
    bool selectable() const noexcept { return m_selectable || m_editable; }

    /** @brief Whether the user can edit the text (implies selectable()). `false` by default. */
    void setEditable(bool editable) noexcept;
    bool editable() const noexcept { return m_editable; }

    /** @brief Whether Enter inserts line breaks (Ctrl+Enter then submits). `false` by default. */
    void setMultiline(bool multiline) noexcept { m_multiline = multiline; }
    bool multiline() const noexcept { return m_multiline; }

    /** @brief Being edited: editable() and with keyboard focus. */
    bool editing() const noexcept;

    /* ----------------------------------- Selection ----------------------------------- */

    /**
     * @brief Selects from @p anchor to @p caret (in any order; equal: no selection, just the caret).
     *
     * Clamped to the text and snapped to code point boundaries. Emits onSelectionChanged.
     */
    void setSelection(size_t anchor, size_t caret) noexcept;
    void setCaret(size_t pos) noexcept { setSelection(pos, pos); }
    void selectAll() noexcept { setSelection(0, m_text.size()); }

    size_t anchor() const noexcept { return m_anchor; }
    size_t caret() const noexcept { return m_caret; }
    bool hasSelection() const noexcept { return m_anchor != m_caret; }
    size_t selectionStart() const noexcept { return std::min(m_anchor, m_caret); }
    size_t selectionEnd() const noexcept { return std::max(m_anchor, m_caret); }
    std::string selectedText() const noexcept;

    /* ------------------------------------ Editing ------------------------------------ */

    /**
     * @brief Replaces the selection with @p text (inserts it at the caret), as if typed.
     *
     * Works even if not editable(). Line breaks become spaces unless multiline(). Undoable.
     */
    void insert(const std::string &text) noexcept;

    /** @brief Copies the selection to the clipboard. @p triggeringEvent: e.g. the key press. */
    bool copy(const CZInputEvent &triggeringEvent) noexcept;

    /** @brief Copies the selection and removes it (editable() only). */
    bool cut(const CZInputEvent &triggeringEvent) noexcept;

    /** @brief Inserts the clipboard's text (editable() only, asynchronous). */
    void paste() noexcept;

    bool undo() noexcept;
    bool redo() noexcept;

    /* ----------------------------------- Geometry ----------------------------------- */

    /** @brief The text position closest to @p pos (node coordinates). */
    size_t positionAt(SkPoint pos) const noexcept;

    /** @brief Where the caret is drawn for @p pos (node coordinates). */
    SkRect caretRect(size_t pos) const noexcept;

    /** @brief Bounds of the laid-out text (node coordinates, e.g. to fit a background to it). */
    SkRect textBounds() const noexcept;

    /* ------------------------------------ Signals ------------------------------------ */

    /** @brief The text changed (by the user or setText()). */
    CZSignal<> onTextChanged;

    /** @brief The selection or the caret moved. */
    CZSignal<> onSelectionChanged;

    /** @brief Enter was pressed (Ctrl+Enter if multiline()). */
    CZSignal<> onSubmit;

    /** @brief Escape was pressed. */
    CZSignal<> onCancel;

    /** @brief Keyboard focus was lost (e.g. a click elsewhere). */
    CZSignal<> onEditingFinished;

protected:
    void bakeEvent(const AKBakeEvent &event) override;
    void keyboardEnterEvent(const CZKeyboardEnterEvent &event) override;
    void keyboardKeyEvent(const CZKeyboardKeyEvent &event) override;
    void keyboardLeaveEvent(const CZKeyboardLeaveEvent &event) override;
    void pointerButtonEvent(const CZPointerButtonEvent &event) override;
    void pointerMoveEvent(const CZPointerMoveEvent &event) override;
    bool eventFilter(const CZEvent &event, CZObject &target) noexcept override;

private:
    struct UndoState
    {
        std::string text;
        size_t anchor, caret;
    };

    void relayout() noexcept;
    void updateMaps() noexcept;
    void updateCaret() noexcept;
    void restartBlink() noexcept;
    void replace(size_t from, size_t to, const std::string &with, bool typing) noexcept;
    void moveCaret(size_t pos, bool extend) noexcept;
    void eraseTo(size_t pos) noexcept;

    size_t toU16(size_t u8) const noexcept;
    size_t toU8(size_t u16) const noexcept;
    size_t snap(size_t pos) const noexcept;
    size_t prevPos(size_t pos) const noexcept;
    size_t nextPos(size_t pos) const noexcept;
    size_t prevWordPos(size_t pos) const noexcept;
    size_t nextWordPos(size_t pos) const noexcept;
    void wordAt(size_t pos, size_t &start, size_t &end) const noexcept;
    size_t lineEdge(size_t pos, bool end) const noexcept;
    size_t verticalPos(size_t pos, bool down) noexcept;
    SkScalar alignedX() const noexcept;
    SkPoint localPointer() const noexcept;

    std::string m_text;
    std::vector<size_t> m_u8ToU16, m_u16ToU8;
    std::unique_ptr<skia::textlayout::Paragraph> m_paragraph;
    skia::textlayout::TextStyle m_style;
    CZAdaptiveColor m_textColor;
    CZAdaptiveColor m_selectionColor;
    skia::textlayout::TextAlign m_align { skia::textlayout::TextAlign::kLeft };
    SkScalar m_wrapWidth { 0.f };
    SkISize m_size {};
    size_t m_maxLines { 0 };

    size_t m_anchor { 0 }, m_caret { 0 };
    SkScalar m_preferredX { -1.f }; // Kept while moving up/down
    bool m_selectable { true }, m_editable { false }, m_multiline { false };

    // Pointer selection: 1 = characters, 2 = words, 3 = everything
    bool m_dragging { false };
    int m_clickCount { 0 };
    UInt64 m_lastClickMs { 0 };
    SkPoint m_lastClickPos {};
    size_t m_dragStart { 0 }, m_dragEnd { 0 };

    std::vector<UndoState> m_undo, m_redo;
    bool m_lastEditTyping { false };

    AKSolidColor m_caretNode;
    CZTimer m_blink;
};

#endif // CZ_AKCORETEXTEDITOR_H
