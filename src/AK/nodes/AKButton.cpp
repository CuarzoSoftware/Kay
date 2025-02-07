#include <AK/nodes/AKButton.h>
#include <AK/AKTheme.h>
#include <AK/events/AKPointerEnterEvent.h>
#include <AK/events/AKPointerLeaveEvent.h>
#include <AK/events/AKPointerMoveEvent.h>
#include <AK/events/AKPointerButtonEvent.h>

using namespace AK;

AKButton::AKButton(const std::string &text, AKNode *parent) noexcept : AKSubScene(parent),
    m_text(text, &m_content)
{
    setCursor(AKCursor::Pointer);
    m_text.setFont(AKTheme::ButtonFont);
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

    signalLayoutChanged.subscribe(this, [this](AKBitset<LayoutChanges> changes){
        if (changes.check(LayoutChanges::Size))
            updateOpaqueRegion();
    });
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
    addChange(Chg_BackgroundColor);
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
    addChange(Chg_Enabled);
    updateStyle();
}

void AKButton::setPressed(bool pressed) noexcept
{
    if (!enabled())
        return;

    if (m_pressed == pressed)
        return;

    m_pressed = pressed;
    addChange(Chg_Pressed);
    updateStyle();
}

void AKButton::onEvent(const AKEvent &event)
{
    AKSubScene::onEvent(event);

    if (event.type() == AKEvent::Type::State)
    {
        if (event.subtype() == AKEvent::Subtype::Activated || event.subtype() == AKEvent::Subtype::Deactivated)
        {
            updateStyle();
            return;
        }
    }

    if (event.type() != AKEvent::Type::Pointer || event.subtype() != AKEvent::Subtype::Button)
        return;

    auto &e { static_cast<const AKPointerButtonEvent&>(event) };
    if (e.button() == AKPointerButtonEvent::Left)
    {
        const bool triggerOnClicked { !e.state() && pressed() };
        setPressed(e.state());
        enablePointerGrab(e.state());

        if (triggerOnClicked)
            on.clicked.notify();
    }
}

void AKButton::onSceneBegin()
{
    if (!activated() || m_backgroundColor == SK_ColorWHITE)
        m_hThreePatch.setImage(theme()->buttonPlainHThreePatchImage(currentTarget()));
    else
        m_hThreePatch.setImage(theme()->buttonTintedHThreePatchImage(currentTarget()));

    m_hThreePatch.setScale(currentTarget()->bakedComponentsScale());
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
    updateOpaqueRegion();
    applyLayoutConstraints();

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
        m_hThreePatch.setSideSrcRect(AKTheme::ButtonPlainHThreePatchSideSrcRect);
        m_hThreePatch.setCenterSrcRect(AKTheme::ButtonPlainHThreePatchCenterSrcRect);
        m_text.setColorWithoutAlpha(SK_ColorBLACK);
    }
    else
    {
        m_hThreePatch.setSideSrcRect(AKTheme::ButtonTintedHThreePatchSideSrcRect);
        m_hThreePatch.setCenterSrcRect(AKTheme::ButtonTintedHThreePatchCenterSrcRect);

        if (enabled())
            m_text.setColorWithoutAlpha(SK_ColorWHITE);
        else
            m_text.setColorWithoutAlpha(SK_ColorBLACK);
    }

    m_text.setOpacity(contentOpacity);
    m_hThreePatch.setColorFactor(finalBackgroundColor);
}
