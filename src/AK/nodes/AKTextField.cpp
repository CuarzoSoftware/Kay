#include <AK/events/AKWindowStateEvent.h>
#include <AK/events/AKPointerMoveEvent.h>
#include <AK/events/AKKeyboardEnterEvent.h>
#include <AK/events/AKKeyboardLeaveEvent.h>
#include <AK/events/AKPointerButtonEvent.h>
#include <AK/events/AKKeyboardKeyEvent.h>
#include <AK/events/AKLayoutEvent.h>
#include <AK/input/AKPointer.h>
#include <AK/nodes/AKTextField.h>
#include <AK/AKTheme.h>
#include <AK/AKLog.h>
#include <linux/input-event-codes.h>

using namespace AK;

static size_t utf8CharLenght(char c)
{
    if ((c & 0x80) == 0) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 0; // Invalid UTF-8
}

void removeUTF8CharAt(std::string &str, size_t index) {
    size_t size = 0;
    Int32 i = index;
    while (size == 0 && i >= 0) {
        size = utf8CharLenght(str[i]);

        if (size == 0)
        {
            i--;
            continue;
        }

        str.erase(i, size);
        return;
    }
}


AKTextField::AKTextField(AKNode *parent) noexcept : AKContainer(YGFlexDirection::YGFlexDirectionRow, true, parent)
{
    m_caret.setVisible(false);
    m_caret.setAnimated(false);
    m_caret.layout().setPositionType(YGPositionTypeAbsolute);
    //m_focusShadow.setStackPosition(AKBackgroundEffect::Behind);
    setCursor(AKCursor::Text);
    m_hThreePatch.layout().setFlex(1.f);
    m_hThreePatch.layout().setPadding(YGEdgeLeft, AKTheme::TextFieldPadding.left());
    m_hThreePatch.layout().setPadding(YGEdgeRight, AKTheme::TextFieldPadding.right());
    m_hThreePatch.layout().setPadding(YGEdgeTop, AKTheme::TextFieldPadding.top());
    m_hThreePatch.layout().setPadding(YGEdgeBottom, AKTheme::TextFieldPadding.bottom());
    m_hThreePatch.setSideSrcRect(AKTheme::TextFieldRoundHThreePatchSideSrcRect);
    m_hThreePatch.setCenterSrcRect(AKTheme::TextFieldRoundHThreePatchCenterSrcRect);
    m_content.layout().setFlex(1.f);
    m_content.layout().setJustifyContent(YGJustifyCenter);
    m_content.layout().setAlignItems(YGAlignCenter);
    m_content.layout().setFlexDirection(YGFlexDirectionRow);
    m_text.layout().setJustifyContent(YGJustifyCenter);
    m_text.layout().setAlignItems(YGAlignCenter);
    m_text.layout().setMargin(YGEdgeHorizontal, 4.f);
    layout().setWidth(200);
    layout().setHeight(24);
    updateDimensions();
    updateScale();

    m_text.onSelectionChanged.subscribe(this, [this]{
        m_caret.setVisible(m_text.selection()[1] == 0);
    });

    m_text.onTextChanged.subscribe(this, [this]{
        updateCaretPos();
    });
}

void AKTextField::layoutEvent(const AKLayoutEvent &event)
{
    AKContainer::layoutEvent(event);
    if (event.changes().check(AKLayoutEvent::Changes::Size))
    {
        updateDimensions();
        updateTextPosition();
    }
    if (event.changes().check(AKLayoutEvent::Changes::Scale))
        updateScale();
    event.accept();
}

void AKTextField::keyboardEnterEvent(const AKKeyboardEnterEvent &event)
{
    AKContainer::keyboardEnterEvent(event);
    m_caret.setAnimated(true);
    m_caret.setVisible(true);
    event.accept();
}

void AKTextField::keyboardKeyEvent(const AKKeyboardKeyEvent &e)
{
    AKContainer::keyboardKeyEvent(e);

    if (e.state() != AKKeyboardKeyEvent::Pressed)
        return;

    e.accept();

    if (e.keySymbol() == XKB_KEY_Left)
    {
        moveCaretLeft();
    }
    else if (e.keySymbol() == XKB_KEY_Right)
    {
        moveCaretRight();
    }
    else if (e.keySymbol() == XKB_KEY_BackSpace)
    {
        removeUTF8();
    }
    else if (e.keySymbol() == XKB_KEY_Return)
    {
        //if (m_text.setText(m_text.text() + "😊"))
        //    updateTextPosition();
        return;
    }
    else
    {
        const char *key { e.keyString() };

        if (strlen(key) > 0)
        {
            //AKLog::debug("Key %s", key);
            addUTF8(key);
        }
    }
}

void AKTextField::keyboardLeaveEvent(const AKKeyboardLeaveEvent &event)
{
    AKContainer::keyboardLeaveEvent(event);
    m_text.setSelection(0, 0);
    m_caret.setAnimated(false);
    m_caret.setVisible(false);
    event.accept();
}

void AKTextField::pointerButtonEvent(const AKPointerButtonEvent &event)
{
    AKContainer::pointerButtonEvent(event);
    setKeyboardFocus(true);

    if (event.button() == BTN_LEFT)
    {
        if (event.state() == AKPointerButtonEvent::Pressed)
        {
            m_selectionStart = m_text.codePointAt(
                akPointer().pos().x() - SkScalar(m_text.globalRect().x()),
                akPointer().pos().y() - SkScalar(m_text.globalRect().y()));
            m_caretRightOffset = m_text.codePointByteOffsets().size()  - m_selectionStart;
            updateCaretPos();
            m_interactiveSelection = true;
            enablePointerGrab(true);
            m_text.setSelection(0, 0);
        }
        else
        {
            m_interactiveSelection = false;
            enablePointerGrab(false);
        }
    }

    event.accept();
}

void AKTextField::pointerMoveEvent(const AKPointerMoveEvent &event)
{
    AKContainer::pointerMoveEvent(event);

    if (m_interactiveSelection)
    {
        size_t selectionEnd {  m_text.codePointAt(
            event.pos().x() - SkScalar(m_text.globalRect().x()),
            event.pos().y() - SkScalar(m_text.globalRect().y())) };

        m_text.setSelection(std::min(m_selectionStart, selectionEnd), std::abs(Int64(m_selectionStart) - Int64(selectionEnd)));
    }

    event.accept();
}

void AKTextField::windowStateEvent(const AKWindowStateEvent &event)
{
    AKContainer::windowStateEvent(event);

    if (activated())
    {
        if (hasKeyboardFocus())
            m_caret.setVisible(true);
    }
    else
    {
        m_caret.setVisible(false);
    }

    event.accept();
}

void AKTextField::updateDimensions() noexcept
{
    m_hThreePatch.opaqueRegion = theme()->buttonPlainOpaqueRegion(globalRect().width());
}

void AKTextField::updateScale() noexcept
{
    m_hThreePatch.setImage(theme()->textFieldRoundHThreePatchImage(scale()));
    m_hThreePatch.setImageScale(scale());
}

void AKTextField::updateTextPosition() noexcept
{
    layout().calculate();
    if (m_text.layout().calculatedWidth() + m_caret.layout().calculatedWidth() < m_content.layout().calculatedWidth())
        m_content.layout().setJustifyContent(YGJustifyCenter);
    else
        m_content.layout().setJustifyContent(YGJustifyFlexEnd);
}

void AKTextField::updateCaretPos() noexcept
{
    if (m_caretRightOffset > m_text.codePointByteOffsets().size())
        m_caretRightOffset = m_text.codePointByteOffsets().size();

    if (m_text.codePointByteOffsets().empty())
    {
        m_caret.layout().setPosition(
            YGEdgeLeft,
            -m_caret.layout().calculatedWidth()/2.f);
    }
    else if (m_caretRightOffset == 0)
        m_caret.layout().setPosition(
            YGEdgeLeft,
            m_text.glyphAtCodePoint(m_text.codePointByteOffsets().size() - 1).right() - m_caret.layout().calculatedWidth()/2.f);
    else
        m_caret.layout().setPosition(
            YGEdgeLeft,
            m_text.glyphAtCodePoint(m_text.codePointByteOffsets().size() - m_caretRightOffset).left() - m_caret.layout().calculatedWidth()/2.f);
}

void AKTextField::addUTF8(const char *utf8) noexcept
{
    if (m_text.selection()[1] == 0)
    {
        if (m_caretRightOffset == 0)
        {
            if (m_text.setText(m_text.text() + utf8))
                updateTextPosition();
        }
        else
        {
            const Int64 caretPos = Int64(m_text.codePointByteOffsets().size()) - m_caretRightOffset;
            std::string newText = m_text.skText().substr(0, m_text.codePointByteOffset(caretPos)) + utf8 + m_text.skText().substr(m_text.codePointByteOffset(caretPos));
            if (m_text.setText(newText))
                updateTextPosition();
        }
    }
    else
    {
        m_caretRightOffset = std::max(Int64(m_text.codePointByteOffsets().size()) - Int64(m_text.selection()[0] + m_text.selection()[1]), Int64(0));
        std::string newText = m_text.skText().substr(0, m_text.codePointByteOffset(m_text.selection()[0])) + utf8 + m_text.skText().substr( m_text.codePointByteOffset(m_text.selection()[0] + m_text.selection()[1]));
        if (m_text.setText(newText))
            updateTextPosition();
    }
}

void AKTextField::removeUTF8() noexcept
{
    if (m_text.text().empty())
        return;

    if (m_text.selection()[1] == 0)
    {
        if (m_caretRightOffset == 0)
        {
            std::string str { m_text.text() };
            removeUTF8CharAt(str, str.size() - 1);
            if (m_text.setText(str))
                updateTextPosition();
        }
        else
        {
            const Int64 caretPos = Int64(m_text.codePointByteOffsets().size()) - m_caretRightOffset;

            if (caretPos == 0)
                return;

            std::string newText = m_text.skText().substr(0, m_text.codePointByteOffset(caretPos - 1)) + m_text.skText().substr(m_text.codePointByteOffset(caretPos));
            if (m_text.setText(newText))
                updateTextPosition();
        }
    }
    else
    {
        m_caretRightOffset = std::max(Int64(m_text.codePointByteOffsets().size()) - Int64(m_text.selection()[0] + m_text.selection()[1]), Int64(0));
        std::string newText = m_text.skText().substr(0, m_text.codePointByteOffset(m_text.selection()[0])) +  m_text.skText().substr( m_text.codePointByteOffset(m_text.selection()[0] + m_text.selection()[1]));
        if (m_text.setText(newText))
            updateTextPosition();
    }
}

void AKTextField::moveCaretRight() noexcept
{
    if (m_text.selection()[1] == 0)
    {
        if (m_caretRightOffset > 0)
        {
            m_caretRightOffset--;
            updateCaretPos();
        }
    }
    else
    {
        m_caretRightOffset = m_text.codePointByteOffsets().size() - m_text.selection()[0] - m_text.selection()[1];
        updateCaretPos();
    }
    m_caret.setVisible(true);
    m_text.setSelection(0, 0);
}

void AKTextField::moveCaretLeft() noexcept
{
    if (m_text.selection()[1] == 0)
        m_caretRightOffset++;
    else
        m_caretRightOffset = m_text.codePointByteOffsets().size() - m_text.selection()[0];

    updateCaretPos();
    m_caret.setVisible(true);
    m_text.setSelection(0, 0);
}
