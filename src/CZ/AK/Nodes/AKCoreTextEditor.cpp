#include <CZ/AK/Nodes/AKCoreTextEditor.h>
#include <CZ/AK/Events/AKBakeEvent.h>
#include <CZ/AK/AKClipboard.h>
#include <CZ/AK/AKScene.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKApp.h>
#include <CZ/Core/CZCore.h>
#include <CZ/Core/CZKeymap.h>
#include <CZ/Core/CZTime.h>
#include <CZ/Core/CZWeak.h>
#include <CZ/Core/Events/CZKeyboardKeyEvent.h>
#include <CZ/Core/Events/CZKeyboardEnterEvent.h>
#include <CZ/Core/Events/CZKeyboardLeaveEvent.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZPointerMoveEvent.h>
#include <CZ/Ream/RPass.h>
#include <CZ/Ream/RSurface.h>
#include <CZ/skia/core/SkCanvas.h>
#include <CZ/skia/modules/skparagraph/include/ParagraphBuilder.h>
#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <algorithm>
#include <cmath>

using namespace CZ;
using namespace skia::textlayout;

static constexpr UInt32 BlinkMs { 530 };
static constexpr UInt32 MultiClickMs { 400 };
static constexpr SkScalar MultiClickDistance { 4.f };
static constexpr size_t UndoLimit { 100 };
static const CZAdaptiveColor UnfocusedSelection { 0x40808080, 0x50808080 };

static bool IsContinuationByte(char c) noexcept { return (static_cast<unsigned char>(c) & 0xC0) == 0x80; }

static size_t UTF8Length(char c) noexcept
{
    const auto b { static_cast<unsigned char>(c) };
    if (b < 0x80) return 1;
    if ((b & 0xE0) == 0xC0) return 2;
    if ((b & 0xF0) == 0xE0) return 3;
    if ((b & 0xF8) == 0xF0) return 4;
    return 1; // Invalid: one byte (Skia shows it as U+FFFD)
}

// Word characters (for Ctrl+arrows / Ctrl+Backspace): letters, digits and anything non-ASCII.
static bool IsWordByte(char c) noexcept
{
    const auto b { static_cast<unsigned char>(c) };
    return b >= 0x80 || std::isalnum(b) || b == '_';
}

static bool KeyHeld(UInt32 a, UInt32 b) noexcept
{
    auto keymap { CZCore::Get()->keymap() };
    return keymap && (keymap->pressedKeys().contains(a) || keymap->pressedKeys().contains(b));
}

AKCoreTextEditor::AKCoreTextEditor(const std::string &text, AKNode *parent) noexcept :
    AKBakeable(parent),
    m_style(theme()->DefaultTextStyle),
    m_textColor(AKTheme::Text),
    m_selectionColor({ 0x4D0088FF, 0x660091FF }),
    m_caretNode(AKTheme::Text, this)
{
    setCursor(CZCursorShape::Text);
    setKeyboardFocusable(true);

    SkRegion empty;
    m_caretNode.setInputRegion(&empty);
    m_caretNode.layout().setPositionType(YGPositionTypeAbsolute);
    m_caretNode.setVisible(false);

    m_blink.setCallback([this](CZTimer *timer) {
        m_caretNode.setOpacity(m_caretNode.opacity() > 0.5f ? 0.f : 1.f);
        timer->start(BlinkMs);
    });

    // The text color and the selection highlight follow the color scheme
    onColorSchemeChanged.subscribe(this, [this](CZColorScheme) {
        m_caretNode.setColor(m_textColor);
        relayout();
    });

    // Focus is dropped when a press lands outside (see eventFilter())
    if (auto app { AKApp::Get() })
        app->installEventFilter(this);

    m_text = text;
    m_anchor = m_caret = m_text.size();
    updateMaps();
    relayout();
}

AKCoreTextEditor::~AKCoreTextEditor() noexcept
{
    if (auto app { AKApp::Get() })
        app->removeEventFilter(this);
}

/* ------------------------------------------- Text ------------------------------------------- */

bool AKCoreTextEditor::setText(const std::string &text) noexcept
{
    if (m_text == text)
        return false;

    m_text = text;
    m_anchor = m_caret = m_text.size();
    m_undo.clear();
    m_redo.clear();
    m_lastEditTyping = false;
    updateMaps();
    relayout();
    onTextChanged.notify();
    onSelectionChanged.notify();
    return true;
}

/* ------------------------------------------- Style ------------------------------------------- */

void AKCoreTextEditor::setTextStyle(const TextStyle &style) noexcept
{
    m_style = style;
    relayout();
}

void AKCoreTextEditor::setFontSize(SkScalar size) noexcept
{
    if (m_style.getFontSize() == size)
        return;

    m_style.setFontSize(size);
    relayout();
}

void AKCoreTextEditor::setFontStyle(SkFontStyle style) noexcept
{
    if (m_style.getFontStyle() == style)
        return;

    m_style.setFontStyle(style);
    relayout();
}

void AKCoreTextEditor::setTextColor(CZAdaptiveColor color) noexcept
{
    if (m_textColor == color)
        return;

    m_textColor = color;
    m_caretNode.setColor(color);
    relayout();
}

void AKCoreTextEditor::setSelectionColor(CZAdaptiveColor color) noexcept
{
    if (m_selectionColor == color)
        return;

    m_selectionColor = color;
    addChange(CHSelection);
    addDamage(AK_IRECT_INF);
}

void AKCoreTextEditor::setTextAlign(TextAlign align) noexcept
{
    if (m_align == align)
        return;

    m_align = align;
    relayout();
}

void AKCoreTextEditor::setWrapWidth(SkScalar width) noexcept
{
    width = std::max(0.f, width);

    if (m_wrapWidth == width)
        return;

    m_wrapWidth = width;
    relayout();
}

void AKCoreTextEditor::setMaxLines(size_t lines) noexcept
{
    if (m_maxLines == lines)
        return;

    m_maxLines = lines;
    relayout();
}

/* ----------------------------------------- Behavior ----------------------------------------- */

void AKCoreTextEditor::setSelectable(bool selectable) noexcept
{
    m_selectable = selectable;
    setCursor(this->selectable() ? std::optional(CZCursorShape::Text) : std::nullopt);

    if (!this->selectable())
        setCaret(m_caret);
}

void AKCoreTextEditor::setEditable(bool editable) noexcept
{
    if (m_editable == editable)
        return;

    m_editable = editable;
    setSelectable(m_selectable);
    relayout(); // maxLines only applies while not editing
}

bool AKCoreTextEditor::editing() const noexcept
{
    return m_editable && hasKeyboardFocus();
}

/* ----------------------------------------- Selection ----------------------------------------- */

void AKCoreTextEditor::setSelection(size_t anchor, size_t caret) noexcept
{
    anchor = snap(anchor);
    caret = snap(caret);

    if (anchor == m_anchor && caret == m_caret)
        return;

    m_anchor = anchor;
    m_caret = caret;
    addChange(CHSelection);
    addDamage(AK_IRECT_INF);
    updateCaret();
    restartBlink();
    onSelectionChanged.notify();
}

std::string AKCoreTextEditor::selectedText() const noexcept
{
    return m_text.substr(selectionStart(), selectionEnd() - selectionStart());
}

/* ------------------------------------------ Editing ------------------------------------------ */

void AKCoreTextEditor::insert(const std::string &text) noexcept
{
    std::string clean { text };

    if (!m_multiline)
    {
        std::replace(clean.begin(), clean.end(), '\n', ' ');
        std::erase(clean, '\r');
    }

    // Typing one character at a time is undone as a whole
    replace(selectionStart(), selectionEnd(), clean, clean.size() <= 4 && clean != " ");
}

bool AKCoreTextEditor::copy(const CZInputEvent &triggeringEvent) noexcept
{
    auto *clipboard { AKApp::Get()->clipboard() };

    if (!clipboard || !hasSelection())
        return false;

    clipboard->setText(selectedText(), triggeringEvent);
    return true;
}

bool AKCoreTextEditor::cut(const CZInputEvent &triggeringEvent) noexcept
{
    if (!m_editable || !copy(triggeringEvent))
        return false;

    replace(selectionStart(), selectionEnd(), "", false);
    return true;
}

void AKCoreTextEditor::paste() noexcept
{
    auto *clipboard { AKApp::Get()->clipboard() };

    if (!m_editable || !clipboard)
        return;

    clipboard->requestText([self = CZWeak<AKCoreTextEditor>(this)](std::optional<std::string> text) {
        if (self && self->m_editable && text && !text->empty())
        {
            self->m_lastEditTyping = false;
            self->insert(*text);
            self->m_lastEditTyping = false;
        }
    });
}

bool AKCoreTextEditor::undo() noexcept
{
    if (m_undo.empty())
        return false;

    m_redo.push_back({ m_text, m_anchor, m_caret });
    const UndoState state { std::move(m_undo.back()) };
    m_undo.pop_back();
    m_lastEditTyping = false;
    m_text = state.text;
    m_anchor = state.anchor;
    m_caret = state.caret;
    updateMaps();
    relayout();
    onTextChanged.notify();
    onSelectionChanged.notify();
    return true;
}

bool AKCoreTextEditor::redo() noexcept
{
    if (m_redo.empty())
        return false;

    m_undo.push_back({ m_text, m_anchor, m_caret });
    const UndoState state { std::move(m_redo.back()) };
    m_redo.pop_back();
    m_lastEditTyping = false;
    m_text = state.text;
    m_anchor = state.anchor;
    m_caret = state.caret;
    updateMaps();
    relayout();
    onTextChanged.notify();
    onSelectionChanged.notify();
    return true;
}

void AKCoreTextEditor::replace(size_t from, size_t to, const std::string &with, bool typing) noexcept
{
    from = snap(from);
    to = snap(std::max(from, to));

    if (from == to && with.empty())
        return;

    if (!(typing && m_lastEditTyping))
    {
        m_undo.push_back({ m_text, m_anchor, m_caret });

        if (m_undo.size() > UndoLimit)
            m_undo.erase(m_undo.begin());
    }

    m_redo.clear();
    m_lastEditTyping = typing;
    m_text.replace(from, to - from, with);
    m_anchor = m_caret = from + with.size();
    m_preferredX = -1.f;
    updateMaps();
    relayout();
    restartBlink();
    onTextChanged.notify();
    onSelectionChanged.notify();
}

void AKCoreTextEditor::moveCaret(size_t pos, bool extend) noexcept
{
    m_lastEditTyping = false;
    setSelection(extend ? m_anchor : pos, pos);
}

void AKCoreTextEditor::eraseTo(size_t pos) noexcept
{
    if (hasSelection())
        replace(selectionStart(), selectionEnd(), "", false);
    else
        replace(std::min(pos, m_caret), std::max(pos, m_caret), "", false);
}

/* ----------------------------------------- Geometry ----------------------------------------- */

size_t AKCoreTextEditor::positionAt(SkPoint pos) const noexcept
{
    if (!m_paragraph || m_text.empty())
        return 0;

    const auto glyph { m_paragraph->getGlyphPositionAtCoordinate(pos.x(), pos.y()) };
    return toU8(size_t(std::max(0, glyph.position)));
}

SkScalar AKCoreTextEditor::alignedX() const noexcept
{
    switch (m_align)
    {
    case TextAlign::kCenter:
        return m_size.width() / 2.f;
    case TextAlign::kRight:
    case TextAlign::kEnd:
        return std::max(0.f, m_size.width() - 1.f);
    default:
        return 0.f;
    }
}

SkRect AKCoreTextEditor::caretRect(size_t pos) const noexcept
{
    pos = snap(pos);
    const SkScalar lineHeight { m_paragraph && m_text.empty() ? m_paragraph->getHeight() : m_style.getFontSize() * 1.2f };

    if (!m_paragraph)
        return SkRect::MakeXYWH(alignedX(), 0.f, 1.f, lineHeight);

    const auto charBox = [this](size_t a, size_t b) -> std::optional<SkRect> {
        const auto boxes { m_paragraph->getRectsForRange(toU16(a), toU16(b), RectHeightStyle::kMax, RectWidthStyle::kTight) };
        if (boxes.empty()) return std::nullopt;
        return boxes.front().rect;
    };

    // Before the character at pos, or after the one before it
    if (pos < m_text.size() && m_text[pos] != '\n')
        if (const auto r { charBox(pos, nextPos(pos)) })
            return SkRect::MakeLTRB(r->left(), r->top(), r->left() + 1.f, r->bottom());

    if (pos > 0 && m_text[pos - 1] != '\n')
        if (const auto r { charBox(prevPos(pos), pos) })
            return SkRect::MakeLTRB(r->right(), r->top(), r->right() + 1.f, r->bottom());

    // An empty line (empty text, or after a line break)
    std::vector<LineMetrics> lines;
    m_paragraph->getLineMetrics(lines);
    const size_t u16 { toU16(pos) };
    const LineMetrics *line { nullptr };

    for (const auto &l : lines)
        if (u16 >= l.fStartIndex && u16 <= l.fEndIncludingNewline)
            line = &l;

    if (!line && !lines.empty())
        line = &lines.back();

    if (!line)
        return SkRect::MakeXYWH(alignedX(), 0.f, 1.f, lineHeight);

    return SkRect::MakeXYWH(alignedX(), line->fBaseline - line->fAscent, 1.f, line->fHeight);
}

SkRect AKCoreTextEditor::textBounds() const noexcept
{
    if (!m_paragraph || m_text.empty())
        return caretRect(0);

    SkRect bounds { SkRect::MakeEmpty() };
    std::vector<LineMetrics> lines;
    m_paragraph->getLineMetrics(lines);

    for (const auto &l : lines)
        bounds.join(SkRect::MakeXYWH(l.fLeft, l.fBaseline - l.fAscent, std::max(1.0, l.fWidth), l.fHeight));

    return bounds;
}

/* --------------------------------------- Positions --------------------------------------- */

void AKCoreTextEditor::updateMaps() noexcept
{
    // Skia's paragraph positions are UTF-16 indices
    m_u8ToU16.assign(m_text.size() + 1, 0);
    m_u16ToU8.clear();
    m_u16ToU8.reserve(m_text.size() + 1);

    size_t u16 { 0 };

    for (size_t i = 0; i < m_text.size();)
    {
        const size_t len { std::min(UTF8Length(m_text[i]), m_text.size() - i) };

        for (size_t k = 0; k < len; k++)
            m_u8ToU16[i + k] = u16;

        const size_t units { len == 4 ? 2u : 1u };

        for (size_t k = 0; k < units; k++)
            m_u16ToU8.push_back(i);

        u16 += units;
        i += len;
    }

    m_u8ToU16[m_text.size()] = u16;
    m_u16ToU8.push_back(m_text.size());
    m_anchor = snap(m_anchor);
    m_caret = snap(m_caret);
}

size_t AKCoreTextEditor::toU16(size_t u8) const noexcept
{
    return m_u8ToU16[std::min(u8, m_u8ToU16.size() - 1)];
}

size_t AKCoreTextEditor::toU8(size_t u16) const noexcept
{
    return m_u16ToU8[std::min(u16, m_u16ToU8.size() - 1)];
}

size_t AKCoreTextEditor::snap(size_t pos) const noexcept
{
    pos = std::min(pos, m_text.size());

    while (pos > 0 && pos < m_text.size() && IsContinuationByte(m_text[pos]))
        pos--;

    return pos;
}

size_t AKCoreTextEditor::prevPos(size_t pos) const noexcept
{
    if (pos == 0)
        return 0;

    pos--;

    while (pos > 0 && IsContinuationByte(m_text[pos]))
        pos--;

    return pos;
}

size_t AKCoreTextEditor::nextPos(size_t pos) const noexcept
{
    if (pos >= m_text.size())
        return m_text.size();

    return std::min(m_text.size(), pos + UTF8Length(m_text[pos]));
}

size_t AKCoreTextEditor::prevWordPos(size_t pos) const noexcept
{
    // Skip separators, then the word
    while (pos > 0 && !IsWordByte(m_text[pos - 1]))
        pos = prevPos(pos);

    while (pos > 0 && IsWordByte(m_text[pos - 1]))
        pos = prevPos(pos);

    return pos;
}

size_t AKCoreTextEditor::nextWordPos(size_t pos) const noexcept
{
    while (pos < m_text.size() && !IsWordByte(m_text[pos]))
        pos = nextPos(pos);

    while (pos < m_text.size() && IsWordByte(m_text[pos]))
        pos = nextPos(pos);

    return pos;
}

void AKCoreTextEditor::wordAt(size_t pos, size_t &start, size_t &end) const noexcept
{
    start = end = pos;

    if (!m_paragraph || m_text.empty())
        return;

    // Skia's word boundaries (ICU), mapped back to UTF-8
    // At the end: the last word
    const size_t at { pos >= m_text.size() ? prevPos(m_text.size()) : pos };
    const auto range { m_paragraph->getWordBoundary(unsigned(toU16(at))) };
    start = toU8(range.start);
    end = toU8(range.end);

    if (start > end)
        std::swap(start, end);
}

size_t AKCoreTextEditor::lineEdge(size_t pos, bool end) const noexcept
{
    const SkRect r { caretRect(pos) };
    return positionAt(SkPoint::Make(end ? 1e6f : -1e6f, r.centerY()));
}

size_t AKCoreTextEditor::verticalPos(size_t pos, bool down) noexcept
{
    const SkRect r { caretRect(pos) };

    if (m_preferredX < 0.f)
        m_preferredX = r.left();

    const SkScalar y { down ? r.bottom() + 1.f : r.top() - 1.f };

    if (y < 0.f)
        return 0;

    if (y >= SkScalar(m_size.height()))
        return m_text.size();

    return positionAt(SkPoint::Make(m_preferredX, y));
}

SkPoint AKCoreTextEditor::localPointer() const noexcept
{
    const SkPoint pos { AKApp::Get()->pointer().pos() };
    return SkPoint::Make(pos.x() - SkScalar(worldRect().x()), pos.y() - SkScalar(worldRect().y()));
}

/* ------------------------------------------ Layout ------------------------------------------ */

void AKCoreTextEditor::relayout() noexcept
{
    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(m_align);
    paragraphStyle.setTextDirection(TextDirection::kLtr);

    TextStyle style { m_style };
    const SkColor color { m_textColor.resolve(colorScheme()) };
    SkPaint paint;
    paint.setColor(color);
    paint.setAntiAlias(true);
    style.setColor(color);
    style.setForegroundPaint(paint);
    paragraphStyle.setTextStyle(style);

    // Long text is ellipsized only while not being edited
    if (m_maxLines > 0 && !editing())
    {
        paragraphStyle.setMaxLines(m_maxLines);
        paragraphStyle.setEllipsis(u"…");
    }

    auto builder { ParagraphBuilder::make(paragraphStyle, AKApp::Get()->fontCollection()) };
    builder->pushStyle(style);
    builder->addText(m_text.data(), m_text.size());
    builder->pop();
    m_paragraph = builder->Build();

    SkScalar width { m_wrapWidth };

    if (width <= 0.f)
    {
        // As wide as the longest line, plus room for the caret at its end
        m_paragraph->layout(1e6f);
        width = std::ceil(m_paragraph->getMaxIntrinsicWidth()) + 1.f;
    }

    m_paragraph->layout(width);
    m_size = SkISize::Make(Int32(std::ceil(width)), Int32(std::ceil(m_paragraph->getHeight())));
    layout().setWidth(m_size.width());
    layout().setHeight(m_size.height());
    addChange(CHText);
    addDamage(AK_IRECT_INF);
    updateCaret();
}

void AKCoreTextEditor::updateCaret() noexcept
{
    const bool shown { editing() && !hasSelection() };
    m_caretNode.setVisible(shown);

    if (!shown)
        return;

    const SkRect r { caretRect(m_caret) };
    m_caretNode.layout().setPosition(YGEdgeLeft, std::min(r.left(), SkScalar(std::max(0, m_size.width() - 1))));
    m_caretNode.layout().setPosition(YGEdgeTop, r.top());
    m_caretNode.layout().setWidth(1.f);
    m_caretNode.layout().setHeight(r.height());
}

void AKCoreTextEditor::restartBlink() noexcept
{
    m_caretNode.setOpacity(1.f);

    if (editing())
        m_blink.start(BlinkMs);
    else
        m_blink.stop();
}

void AKCoreTextEditor::bakeEvent(const AKBakeEvent &e)
{
    if (e.damage.isEmpty() && !e.changes.testAnyOf(CHText, CHSelection))
        return;

    auto pass { e.surface->beginPass(RPassCap_SkCanvas) };
    auto *c { pass->getCanvas() };

    c->save();
    c->clipIRect(SkIRect::MakeSize(worldRect().size()));
    c->clear(SK_ColorTRANSPARENT);

    // Selection highlight behind the text
    if (selectable() && hasSelection() && m_paragraph)
    {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor((hasKeyboardFocus() ? m_selectionColor : UnfocusedSelection).resolve(colorScheme()));

        for (const auto &box : m_paragraph->getRectsForRange(toU16(selectionStart()), toU16(selectionEnd()), RectHeightStyle::kMax, RectWidthStyle::kTight))
            c->drawRect(box.rect, paint);
    }

    if (m_paragraph)
        m_paragraph->paint(c, 0.f, 0.f);

    c->restore();
}

/* ------------------------------------------- Input ------------------------------------------- */

bool AKCoreTextEditor::eventFilter(const CZEvent &event, CZObject &target) noexcept
{
    auto app { AKApp::Get() };

    // A press outside (or on another surface) drops the focus
    if (hasKeyboardFocus() && &target == app.get() && event.type() == CZEvent::Type::PointerButton &&
        static_cast<const CZPointerButtonEvent&>(event).pressed && scene() &&
        (app->pointer().focus() != scene() || !scene()->pointerFocus() ||
         (scene()->pointerFocus() != this && !scene()->pointerFocus()->isSubchildOf(this))))
        setKeyboardFocus(false);

    return false;
}

void AKCoreTextEditor::keyboardEnterEvent(const CZKeyboardEnterEvent &event)
{
    AKBakeable::keyboardEnterEvent(event);

    // The full text while editing (see setMaxLines()), and the selection in its focused color
    relayout();
    addChange(CHSelection);
    restartBlink();
}

void AKCoreTextEditor::keyboardLeaveEvent(const CZKeyboardLeaveEvent &event)
{
    AKBakeable::keyboardLeaveEvent(event);
    m_blink.stop();
    m_dragging = false;
    m_lastEditTyping = false;
    relayout();
    addChange(CHSelection);
    onEditingFinished.notify();
}

void AKCoreTextEditor::keyboardKeyEvent(const CZKeyboardKeyEvent &e)
{
    AKBakeable::keyboardKeyEvent(e);

    if (!e.isPressed || !selectable())
        return;

    e.accept();

    const bool ctrl { KeyHeld(KEY_LEFTCTRL, KEY_RIGHTCTRL) };
    const bool shift { KeyHeld(KEY_LEFTSHIFT, KEY_RIGHTSHIFT) };
    const bool vertical { e.symbol == XKB_KEY_Up || e.symbol == XKB_KEY_Down };

    if (!vertical)
        m_preferredX = -1.f;

    // Shortcuts (by key code: independent of the keyboard layout)
    if (ctrl)
    {
        switch (e.code)
        {
        case KEY_A: selectAll(); return;
        case KEY_C: copy(e); return;
        case KEY_X: cut(e); return;
        case KEY_V: paste(); return;
        case KEY_Z:
            if (m_editable) { if (shift) redo(); else undo(); }
            return;
        case KEY_Y:
            if (m_editable) redo();
            return;
        default: break;
        }
    }

    switch (e.symbol)
    {
    case XKB_KEY_Left:
    case XKB_KEY_KP_Left:
        if (hasSelection() && !shift)
            moveCaret(ctrl ? prevWordPos(selectionStart()) : selectionStart(), false);
        else
            moveCaret(ctrl ? prevWordPos(m_caret) : prevPos(m_caret), shift);
        return;
    case XKB_KEY_Right:
    case XKB_KEY_KP_Right:
        if (hasSelection() && !shift)
            moveCaret(ctrl ? nextWordPos(selectionEnd()) : selectionEnd(), false);
        else
            moveCaret(ctrl ? nextWordPos(m_caret) : nextPos(m_caret), shift);
        return;
    case XKB_KEY_Up:
    case XKB_KEY_KP_Up:
        moveCaret(verticalPos(m_caret, false), shift);
        return;
    case XKB_KEY_Down:
    case XKB_KEY_KP_Down:
        moveCaret(verticalPos(m_caret, true), shift);
        return;
    case XKB_KEY_Home:
    case XKB_KEY_KP_Home:
        moveCaret(ctrl ? 0 : lineEdge(m_caret, false), shift);
        return;
    case XKB_KEY_End:
    case XKB_KEY_KP_End:
        moveCaret(ctrl ? m_text.size() : lineEdge(m_caret, true), shift);
        return;
    case XKB_KEY_Escape:
        onCancel.notify();
        return;
    case XKB_KEY_Return:
    case XKB_KEY_KP_Enter:
        if (m_editable && m_multiline && !ctrl)
        {
            m_lastEditTyping = false;
            insert("\n");
        }
        else
            onSubmit.notify();
        return;
    default:
        break;
    }

    if (!m_editable)
        return;

    switch (e.symbol)
    {
    case XKB_KEY_BackSpace:
        eraseTo(ctrl ? prevWordPos(m_caret) : prevPos(m_caret));
        return;
    case XKB_KEY_Delete:
    case XKB_KEY_KP_Delete:
        eraseTo(ctrl ? nextWordPos(m_caret) : nextPos(m_caret));
        return;
    case XKB_KEY_Tab:
    case XKB_KEY_ISO_Left_Tab:
        return; // Not typed
    default:
        break;
    }

    // Typing (control characters aren't text)
    if (!ctrl && !e.utf8.empty() && static_cast<unsigned char>(e.utf8[0]) >= 0x20 && e.utf8[0] != 0x7F)
        insert(e.utf8);
}

void AKCoreTextEditor::pointerButtonEvent(const CZPointerButtonEvent &e)
{
    AKBakeable::pointerButtonEvent(e);

    if (!selectable() || e.button != BTN_LEFT)
        return;

    e.accept();

    if (!e.pressed)
    {
        m_dragging = false;

        if (scene() && scene()->pointerGrab() == this)
            scene()->setPointerGrab(nullptr);

        return;
    }

    if (!hasKeyboardFocus())
        setKeyboardFocus(true);

    const SkPoint local { localPointer() };
    const UInt64 now { CZTime::Ms() };

    if (now - m_lastClickMs <= MultiClickMs && SkPoint::Distance(local, m_lastClickPos) <= MultiClickDistance)
        m_clickCount = m_clickCount % 3 + 1;
    else
        m_clickCount = 1;

    m_lastClickMs = now;
    m_lastClickPos = local;
    m_lastEditTyping = false;
    m_preferredX = -1.f;

    const size_t pos { positionAt(local) };

    switch (m_clickCount)
    {
    case 1:
        if (KeyHeld(KEY_LEFTSHIFT, KEY_RIGHTSHIFT))
            setSelection(m_anchor, pos);
        else
            setCaret(pos);
        m_dragStart = m_dragEnd = m_anchor;
        break;
    case 2:
        wordAt(pos, m_dragStart, m_dragEnd);
        setSelection(m_dragStart, m_dragEnd);
        break;
    default:
        m_dragStart = 0;
        m_dragEnd = m_text.size();
        selectAll();
        break;
    }

    m_dragging = true;

    if (scene())
        scene()->setPointerGrab(this);
}

void AKCoreTextEditor::pointerMoveEvent(const CZPointerMoveEvent &e)
{
    AKBakeable::pointerMoveEvent(e);

    if (!m_dragging)
        return;

    e.accept();
    const size_t pos { positionAt(localPointer()) };

    if (m_clickCount == 1)
        setSelection(m_dragStart, pos);
    else if (m_clickCount == 2)
    {
        // By words, keeping the double-clicked one
        size_t start, end;
        wordAt(pos, start, end);

        if (pos < m_dragStart)
            setSelection(m_dragEnd, start);
        else
            setSelection(m_dragStart, std::max(end, m_dragEnd));
    }
}
