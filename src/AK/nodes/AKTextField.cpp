#include <AK/nodes/AKTextField.h>
#include <AK/events/AKKeyboardKeyEvent.h>
#include <AK/AKTheme.h>
#include <linux/input-event-codes.h>

using namespace AK;

AKTextField::AKTextField(AKNode *parent) noexcept : AKContainer(YGFlexDirection::YGFlexDirectionRow, true, parent)
{
    m_caret.setAnimated(true);
    //m_focusShadow.setStackPosition(AKBackgroundEffect::Behind);
    setCursor(AKCursor::Text);
    m_hThreePatch.layout().setFlex(1.f);
    m_hThreePatch.layout().setWidthPercent(100.f);
    m_hThreePatch.layout().setHeightPercent(100.f);
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
    layout().setWidth(200);
    layout().setHeight(24);

    signalLayoutChanged.subscribe(this, [this](AKBitset<LayoutChanges> changes){
        if (changes.check(LayoutChanges::Size))
        {
            updateDimensions();
            updateTextPosition();
        }
        if (changes.check(LayoutChanges::Scale))
            updateScale();
    });

    updateDimensions();
    updateScale();
}

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

void AKTextField::onEvent(const AKEvent &event)
{
    AKContainer::onEvent(event);

    if (event.type() != AKEvent::KeyboardKey)
        return;

    const auto &e { static_cast<const AKKeyboardKeyEvent&>(event) };

    if (e.state() != AKKeyboardKeyEvent::Pressed)
        return;

    if (e.keySymbol() == XKB_KEY_BackSpace)
    {
        if (m_text.text().empty())
            return;

        std::string str { m_text.text() };
        removeUTF8CharAt(str, str.size() - 1);
        if (m_text.setText(str))
            updateTextPosition();
    }
    else if (e.keySymbol() == XKB_KEY_Return)
    {
        return;
    }
    else
    {
        if (m_text.setText(m_text.text() + e.keyString()))
            updateTextPosition();
    }

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
