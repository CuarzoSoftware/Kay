#include <CZ/Core/Events/CZPointerEnterEvent.h>
#include <CZ/Core/Events/CZPointerLeaveEvent.h>
#include <CZ/Core/Events/CZPointerMoveEvent.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZWindowStateEvent.h>
#include <CZ/Core/Events/CZLayoutEvent.h>
#include <CZ/Core/CZColor.h>
#include <CZ/AK/AKColorTheme.h>
#include <CZ/AK/AKScene.h>
#include <CZ/AK/Nodes/AKButton.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKLog.h>

using namespace CZ;

AKButton::AKButton(const std::string &text, AKNode *parent) noexcept :
    AKRoundSolidColor(0x00000000, AKTheme::AKButtonBorderRadius, parent),
    m_icon("star", 32, this),
    m_text(text, this)
{
    auto the { colorTheme() };
    const auto &scheme { the->scheme(colorScheme()) };
    setColor(scheme.GetPrimary());
    SkRegion empty;
    setCursor(CZCursorShape::Pointer);
    layout().setPadding(YGEdgeAll, 8.f);
    layout().setJustifyContent(YGJustifyCenter);
    layout().setAlignItems(YGAlignCenter);
    layout().setFlexDirection(YGFlexDirectionRow);
    m_text.setTextStyle(theme()->ButtonTextStyle);
    m_text.setInputRegion(&empty);
    m_text.enableReplaceImageColor(true);
    updateStyle();
}

void AKButton::setEnabled(bool enabled) noexcept
{
    if (m_enabled == enabled)
        return;

    setCursor(enabled ? CZCursorShape::Pointer : CZCursorShape::NotAllowed);
    m_enabled = enabled;
    addChange(CHEnabled);
}

void AKButton::setPressed(bool pressed) noexcept
{
    if (!enabled())
        return;

    if (m_pressed == pressed)
        return;

    m_pressed = pressed;
    addChange(CHPressed);
}

void AKButton::onSceneBegin()
{
    updateStyle();
    AKRoundSolidColor::onSceneBegin();
}

void AKButton::pointerButtonEvent(const CZPointerButtonEvent &event)
{
    AKRoundSolidColor::pointerButtonEvent(event);

    if (event.button == BTN_LEFT)
    {
        const bool triggerOnClicked { !event.pressed && pressed() && isPointerOver() };
        setPressed(event.pressed);

        if (event.pressed)
        {
            if (scene())
                scene()->setPointerGrab(this);
        }
        else
        {
            if (scene() && scene()->pointerGrab() == this)
                scene()->setPointerGrab(nullptr);
        }

        if (triggerOnClicked)
            onClick.notify(event);

        event.accept();
    }
}

void AKButton::windowStateEvent(const CZWindowStateEvent &event)
{
    AKRoundSolidColor::windowStateEvent(event);
    event.accept();
}

void AKButton::updateStyle() noexcept
{
    switch (m_variant)
    {
    case Variant::Filled:
        updateStyleFilled();
        break;
    }
}

void AKButton::updateStyleFilled() noexcept
{
    const auto &scheme { colorTheme()->scheme(colorScheme()) };

    if (m_type == Type::Default)
    {
        if (isPointerOver())
        {
            layout().setHeight(40);
            layout().setMaxHeight(40);
            layout().setMinHeight(40);
            layout().setPadding(YGEdgeTop, 0);
            layout().setPadding(YGEdgeBottom, 0);
            layout().setPadding(YGEdgeLeft, 24);
            layout().setPadding(YGEdgeRight, 24);
            layout().setGap(YGGutterAll, 8);
            setBorderRadius(20);
            setBackgroundColor(scheme.GetPrimary());
            setStrokeWidth(0);
            m_shadow.reset();
            m_icon.setSize(20);
            m_icon.setColor(scheme.GetOnPrimary());
            auto textStyle { m_text.textStyle() };
            textStyle.setFontSize(14);
            textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
            m_text.setTextStyle(textStyle);
            m_text.setColor(scheme.GetOnPrimary());
        }
        else
        {
            layout().setHeight(40);
            layout().setMaxHeight(40);
            layout().setMinHeight(40);
            layout().setPadding(YGEdgeTop, 0);
            layout().setPadding(YGEdgeBottom, 0);
            layout().setPadding(YGEdgeLeft, 24);
            layout().setPadding(YGEdgeRight, 24);
            layout().setGap(YGGutterAll, 8);
            setBorderRadius(20);

            SkColor4f overlay { SkColor4f::FromColor(scheme.GetOnPrimary()) };
            overlay.fA = 0.08f;
            //setBackgroundColor(SkColorOverInt(scheme.GetPrimary(), overlay.toSkColor()));
            setStrokeWidth(0);
            m_shadow.reset();
            m_icon.setSize(20);
            m_icon.setColor(scheme.GetOnPrimary());
            auto textStyle { m_text.textStyle() };
            textStyle.setFontSize(14);
            textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
            m_text.setTextStyle(textStyle);
            m_text.setColor(scheme.GetOnPrimary());
        }
    }
    else // Toggle
    {

    }
}
