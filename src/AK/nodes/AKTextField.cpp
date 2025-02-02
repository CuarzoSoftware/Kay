#include <AK/nodes/AKTextField.h>
#include <AK/events/AKKeyboardKeyEvent.h>
#include <AK/AKTheme.h>
#include <linux/input-event-codes.h>

using namespace AK;

AKTextField::AKTextField(AKNode *parent) noexcept : AKSubScene(parent)
{
    m_focusShadow.setStackPosition(AKBackgroundEffect::Behind);
    setCursor(AKCursor::Text);
    m_hThreePatch.layout().setFlex(1.f);
    m_hThreePatch.layout().setWidthPercent(100.f);
    m_hThreePatch.layout().setHeightPercent(100.f);
    m_hThreePatch.layout().setPadding(YGEdgeLeft, AKTheme::TextFieldPadding.left());
    m_hThreePatch.layout().setPadding(YGEdgeRight, AKTheme::TextFieldPadding.right());
    m_hThreePatch.layout().setPadding(YGEdgeTop, AKTheme::TextFieldPadding.top());
    m_hThreePatch.layout().setPadding(YGEdgeBottom, AKTheme::TextFieldPadding.bottom());
    m_content.layout().setFlex(1.f);
    m_content.layout().setJustifyContent(YGJustifyCenter);
    m_content.layout().setAlignItems(YGAlignCenter);
    m_content.layout().setFlexDirection(YGFlexDirectionRow);
    layout().setWidth(200);
    layout().setHeight(24);
}

void AKTextField::onSceneBegin()
{
    m_hThreePatch.setSideSrcRect(AKTheme::TextFieldRoundHThreePatchSideSrcRect);
    m_hThreePatch.setCenterSrcRect(AKTheme::TextFieldRoundHThreePatchCenterSrcRect);
    m_hThreePatch.setImage(theme()->textFieldRoundHThreePatchImage(currentTarget()));
    m_hThreePatch.setScale(currentTarget()->bakedComponentsScale());
}

void AKTextField::onEvent(const AKEvent &event)
{
    AKSubScene::onEvent(event);

    if (event.type() != AKEvent::Type::Keyboard || event.subtype() != AKEvent::Subtype::Key)
        return;

    const auto &e { static_cast<const AKKeyboardKeyEvent&>(event) };

    if (e.state() != AKKeyboardKeyEvent::Pressed)
        return;

    if (e.keySymbol() == XKB_KEY_BackSpace)
    {
        if (m_text.text().empty())
            return;

        m_text.setText(m_text.text().substr(0, m_text.text().size() - 1));
    }
    else
    {
        m_text.setText(m_text.text() + e.keyString());
    }

}
