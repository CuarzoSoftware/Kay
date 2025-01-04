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
    m_text.setFont(theme()->buttonFont());
    applyLayoutConstraints();
    m_hThreePatch.layout().setFlex(1.f);
    m_hThreePatch.layout().setPadding(YGEdgeLeft, theme()->buttonPadding().left());
    m_hThreePatch.layout().setPadding(YGEdgeRight, theme()->buttonPadding().right());
    m_hThreePatch.layout().setPadding(YGEdgeTop, theme()->buttonPadding().top());
    m_hThreePatch.layout().setPadding(YGEdgeBottom, theme()->buttonPadding().bottom());
    m_content.layout().setFlex(1.f);
    m_content.layout().setJustifyContent(YGJustifyCenter);
    m_content.layout().setAlignItems(YGAlignCenter);
    m_content.layout().setFlexDirection(YGFlexDirectionRow);
}

void AKButton::setText(const std::string &text) noexcept
{
    m_text.setText(text);
}

void AKButton::setBackgroundColor(SkColor color) noexcept
{
    if (m_backgroundColor == color)
        return;

    m_backgroundColor = color;
    addChange(Chg_BackgroundColor);
}

void AKButton::setEnabled(bool enabled) noexcept
{
    if (m_enabled == enabled)
        return;

    m_enabled = enabled;
    m_hThreePatch.setOpacity(enabled ? 1.f : theme()->buttonDisabledOpacity());
    m_text.setOpacity(m_hThreePatch.opacity());
    addChange(Chg_Enabled);
}

void AKButton::setPressed(bool pressed) noexcept
{
    if (!enabled())
        return;

    if (m_pressed == pressed)
        return;

    m_pressed = pressed;
    addChange(Chg_Pressed);
}

void AKButton::onEvent(const AKEvent &event)
{
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
    applyLayoutConstraints();

    SkColor4f finalBackgroundColor { SkColor4f::FromColor(m_backgroundColor) };
    SkScalar contentOpacity { 1.f };

    if (pressed())
    {
        finalBackgroundColor.fR *= theme()->buttonPressedBackgroundDarkness();
        finalBackgroundColor.fG *= theme()->buttonPressedBackgroundDarkness();
        finalBackgroundColor.fB *= theme()->buttonPressedBackgroundDarkness();
        contentOpacity = theme()->buttonPressedContentOpacity;
    }

    if (m_backgroundColor == SK_ColorWHITE)
    {
        m_hThreePatch.setSideSrcRect(theme()->buttonPlainHThreePatchSideSrcRect());
        m_hThreePatch.setCenterSrcRect(theme()->buttonPlainHThreePatchCenterSrcRect());
        m_hThreePatch.setImage(theme()->buttonPlainHThreePatchImage(currentTarget()));
        m_text.setColorWithoutAlpha(SK_ColorBLACK);
    }
    else
    {
        m_hThreePatch.setSideSrcRect(theme()->buttonTintedHThreePatchSideSrcRect());
        m_hThreePatch.setCenterSrcRect(theme()->buttonTintedHThreePatchCenterSrcRect());
        m_hThreePatch.setImage(theme()->buttonTintedHThreePatchImage(currentTarget()));

        if (enabled())
            m_text.setColorWithoutAlpha(SK_ColorWHITE);
        else
            m_text.setColorWithoutAlpha(SK_ColorBLACK);
    }

    m_text.setOpacity(contentOpacity);
    m_hThreePatch.setColorFactor(finalBackgroundColor);
    m_hThreePatch.setScale(currentTarget()->bakedComponentsScale());
}

void AKButton::onLayoutUpdate()
{
    if (m_backgroundColor == SK_ColorWHITE)
        m_hThreePatch.opaqueRegion = theme()->buttonPlainOpaqueRegion(rect().width());
    else
        m_hThreePatch.opaqueRegion = theme()->buttonTintedOpaqueRegion(rect().width());
}

void AKButton::applyLayoutConstraints() noexcept
{
    layout().setMinWidth(2.f * theme()->buttonPlainHThreePatchSideSrcRect().width() + 1);
    layout().setMinHeight(theme()->buttonPlainHThreePatchCenterSrcRect().height());
    layout().setMaxHeight(layout().minHeight().value);
    layout().setHeight(layout().minHeight().value);
}
