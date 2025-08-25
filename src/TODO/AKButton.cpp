#include <CZ/Events/CZPointerEnterEvent.h>
#include <CZ/Events/CZPointerLeaveEvent.h>
#include <CZ/Events/CZPointerMoveEvent.h>
#include <CZ/Events/CZPointerButtonEvent.h>
#include <CZ/Events/CZWindowStateEvent.h>
#include <CZ/Events/CZLayoutEvent.h>
#include <CZ/AK/Nodes/AKButton.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKLog.h>

using namespace CZ;

AKButton::AKButton(const std::string &text, AKNode *parent) noexcept : AKSubScene(parent),
    m_text(text, &m_content)
{
    setCursor(AKCursor::Pointer);
    m_text.setTextStyle(theme()->ButtonTextStyle);
    m_hThreePatch.layout().setFlex(1.f);
    m_hThreePatch.layout().setPadding(YGEdgeLeft, AKTheme::ButtonPadding.left());
    m_hThreePatch.layout().setPadding(YGEdgeRight, AKTheme::ButtonPadding.right());
    m_hThreePatch.layout().setPadding(YGEdgeTop, AKTheme::ButtonPadding.top());
    m_hThreePatch.layout().setPadding(YGEdgeBottom, AKTheme::ButtonPadding.bottom());
    m_content.layout().setFlex(1.f);
    m_content.layout().setJustifyContent(YGJustifyCenter);
    m_content.layout().setAlignItems(YGAlignCenter);
    m_content.layout().setFlexDirection(YGFlexDirectionRow);
    updateStyle();
}

void AKButton::setText(const std::string &text) noexcept
{
    if (text == m_text.text())
        return;

    m_text.setText(text);
    updateStyle();
}

void AKButton::setBackgroundColor(SkColor color) noexcept
{
    if (m_backgroundColor == color)
        return;

    m_backgroundColor = color;
    addChange(CHBackgroundColor);
    updateStyle();
}

void AKButton::setEnabled(bool enabled) noexcept
{
    if (m_enabled == enabled)
        return;

    setCursor(enabled ? AKCursor::Pointer : AKCursor::NotAllowed);
    m_enabled = enabled;
    m_hThreePatch.setOpacity(enabled ? 1.f : AKTheme::ButtonDisabledOpacity);
    m_text.setOpacity(m_hThreePatch.opacity());
    addChange(CHEnabled);
    updateStyle();
}

void AKButton::setPressed(bool pressed) noexcept
{
    if (!enabled())
        return;

    if (m_pressed == pressed)
        return;

    m_pressed = pressed;
    addChange(CHPressed);
    updateStyle();
}

void AKButton::pointerButtonEvent(const CZPointerButtonEvent &event)
{
    AKSubScene::pointerButtonEvent(event);

    if (event.button() == CZPointerButtonEvent::Left)
    {
        const bool triggerOnClicked { !event.state() && pressed() && isPointerOver() };
        setPressed(event.state());
        enablePointerGrab(event.state());

        if (triggerOnClicked)
            onClick.notify(event);

        event.accept();
    }
}

void AKButton::windowStateEvent(const CZWindowStateEvent &event)
{
    AKSubScene::windowStateEvent(event);
    if (event.changes().has(AKActivated))
        updateStyle();
    event.accept();
}

void AKButton::layoutEvent(const CZLayoutEvent &event)
{
    AKSubScene::layoutEvent(event);
    if (event.changes().has(CZLayoutChangeScale))
        updateStyle();
    else if (event.changes().has(CZLayoutChangeSize))
        updateOpaqueRegion();
    event.accept();
}

void AKButton::applyLayoutConstraints() noexcept
{
    layout().setMinWidth(2.f * AKTheme::ButtonPlainHThreePatchSideSrcRect.width() + 1);
    layout().setMinHeight(AKTheme::ButtonPlainHThreePatchCenterSrcRect.height());
    layout().setMaxHeight(layout().minHeight().value);
    layout().setHeight(layout().minHeight().value);
}

void AKButton::updateOpaqueRegion() noexcept
{
    if (!activated() || m_backgroundColor == SK_ColorWHITE)
        m_hThreePatch.opaqueRegion = theme()->buttonPlainOpaqueRegion(globalRect().width());
    else
        m_hThreePatch.opaqueRegion = theme()->buttonTintedOpaqueRegion(globalRect().width());
}

void AKButton::updateStyle() noexcept
{
    SkColor4f finalBackgroundColor { SkColor4f::FromColor(activated() ? m_backgroundColor : SK_ColorWHITE) };
    SkScalar contentOpacity { 1.f };

    if (pressed())
    {
        finalBackgroundColor.fR *= AKTheme::ButtonPressedBackgroundDarkness;
        finalBackgroundColor.fG *= AKTheme::ButtonPressedBackgroundDarkness;
        finalBackgroundColor.fB *= AKTheme::ButtonPressedBackgroundDarkness;
        contentOpacity = AKTheme::ButtonContentPressedOpacity;
    }

    if (!activated() || m_backgroundColor == SK_ColorWHITE)
    {
        m_hThreePatch.setImage(theme()->buttonPlainHThreePatchImage(scale()));
        m_hThreePatch.setSideSrcRect(AKTheme::ButtonPlainHThreePatchSideSrcRect);
        m_hThreePatch.setCenterSrcRect(AKTheme::ButtonPlainHThreePatchCenterSrcRect);
        m_text.enableCustomTextureColor(false);
    }
    else
    {
        m_hThreePatch.setImage(theme()->buttonTintedHThreePatchImage(scale()));
        m_hThreePatch.setSideSrcRect(AKTheme::ButtonTintedHThreePatchSideSrcRect);
        m_hThreePatch.setCenterSrcRect(AKTheme::ButtonTintedHThreePatchCenterSrcRect);
        m_text.enableCustomTextureColor(true);

        if (enabled())
            m_text.setColorWithoutAlpha(SK_ColorWHITE);
        else
            m_text.enableCustomTextureColor(false);
    }

    m_text.setOpacity(contentOpacity);
    m_hThreePatch.setColorFactor(finalBackgroundColor);
    m_hThreePatch.setImageScale(scale());
    updateOpaqueRegion();
    applyLayoutConstraints();
}
