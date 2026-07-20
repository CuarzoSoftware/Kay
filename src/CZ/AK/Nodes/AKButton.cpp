#include <CZ/Core/Cuarzo.h>
#include <CZ/Core/Events/CZPointerEnterEvent.h>
#include <CZ/Core/Events/CZPointerLeaveEvent.h>
#include <CZ/Core/Events/CZPointerMoveEvent.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZWindowStateEvent.h>
#include <CZ/Core/Events/CZLayoutEvent.h>
#include <CZ/AK/AKScene.h>
#include <CZ/AK/Nodes/AKButton.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKLog.h>
#include <CZ/Core/Utils/CZColorUtils.h>

using namespace CZ;

AKButton::AKButton(const std::string &text, AKNode *parent) noexcept :
    AKRoundSolidColor(0x00000000, AKTheme::AKButtonBorderRadius, parent),
    m_icon("", 0, this),
    m_text(text, this)
{
    m_icon.setVisible(false);
    init();
}

AKButton::AKButton(const std::string &text, const std::string &icon, AKNode *parent) noexcept :
    AKRoundSolidColor(0x00000000, AKTheme::AKButtonBorderRadius, parent),
    m_icon(icon, 0, this),
    m_text(text, this)
{
    init();
}

void AKButton::init() noexcept
{
    setCursor(CZCursorShape::Pointer);
    layout().setJustifyContent(YGJustifyCenter);
    layout().setAlignItems(YGAlignCenter);
    layout().setFlexDirection(YGFlexDirectionRow);

    // Fixed metrics.
    layout().setHeight(32);
    layout().setMinHeight(32);
    layout().setMaxHeight(32);
    layout().setPadding(YGEdgeLeft, 12);
    layout().setPadding(YGEdgeRight, 12);
    layout().setGap(YGGutterAll, 8);
    setBorderRadius(16);
    setStrokeWidth(0); // solid, borderless

    SkRegion empty;
    m_text.setInputRegion(&empty);
    m_text.enableReplaceImageColor(true);
    m_icon.enableReplaceImageColor(true);
    m_icon.setSize(20);

    auto textStyle { m_text.textStyle() };
    textStyle.setFontStyle(SkFontStyle(
        SkFontStyle::kMedium_Weight,
        SkFontStyle::Width::kNormal_Width,
        SkFontStyle::Slant::kUpright_Slant));
    textStyle.setFontSize(14);
    textStyle.setFontFamilies({ SkString("Roboto") });
    textStyle.setLetterSpacing(0.1);
    textStyle.setHeight(20);
    m_text.setTextStyle(textStyle);

    // Default fill = macOS control accent; content color is auto-derived from it by default
    // (the explicit content color below is the fallback used when auto is disabled).
    m_backgroundColor = AKTheme::ControlAccent;
    m_contentColor = AKTheme::AlternateSelectedControlText;

    updateStyleColor();
}

void AKButton::setEnabled(bool enabled) noexcept
{
    if (m_enabled == enabled)
        return;

    setCursor(enabled ? CZCursorShape::Pointer : CZCursorShape::NotAllowed);
    m_enabled = enabled;
    addChange(CHEnabled);
    updateStyleColor();
}

void AKButton::setPressed(bool pressed) noexcept
{
    if (!enabled())
        return;

    if (m_pressed == pressed)
        return;

    m_pressed = pressed;
    addChange(CHPressed);
    updateStyleColor();
}

void AKButton::setHovered(bool hover) noexcept
{
    if (!enabled())
        hover = false;

    if (m_hovered == hover) return;
    m_hovered = hover;
    updateStyleColor();
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

void AKButton::pointerEnterEvent(const CZPointerEnterEvent &event)
{
    AKRoundSolidColor::pointerEnterEvent(event);
    setHovered(true);
}

void AKButton::pointerLeaveEvent(const CZPointerLeaveEvent &event)
{
    AKRoundSolidColor::pointerLeaveEvent(event);
    setHovered(false);
}

void AKButton::windowStateEvent(const CZWindowStateEvent &event)
{
    AKRoundSolidColor::windowStateEvent(event);
    event.accept();
}

void AKButton::updateStyleColor() noexcept
{
    // Interaction state opacities for the translucent overlay blended over the fill.
    static constexpr Float32 hoverOpacity    { 0.08f };
    static constexpr Float32 pressedOpacity  { 0.12f };
    static constexpr Float32 disabledOpacity { 0.38f };

    const SkColor fill { m_backgroundColor.resolve(colorScheme()) };

    // Effective content color. In automatic mode it follows the fill's lightness with fixed
    // endpoints (.light(), since the choice depends on the fill, not the app's color scheme):
    // AKTheme::AlternateSelectedControlText (white) over dark fills, AKTheme::Label (primary ink)
    // over light fills. Otherwise the set content color resolved for the current scheme.
    const SkColor content {
        m_contentColorAuto
            ? (CZColorUtils::IsColorDark(fill) ? AKTheme::AlternateSelectedControlText.light()
                                               : AKTheme::Label.light())
            : m_contentColor.resolve(colorScheme()) };

    // Container fill = base color + translucent state layer (9-patch replaceColor friendly).
    SkColor bg { fill };

    if (m_enabled && (m_pressed || m_hovered))
        bg = CZColorUtils::StateSrcOver(fill, content, m_pressed ? pressedOpacity : hoverOpacity);

    AKRoundSolidColor::setBackgroundColor(bg);

    const Float32 op { m_enabled ? 1.f : disabledOpacity };
    setOpacity(op);

    // Content (label + icon) share the effective content color.
    m_text.setColor(content);
    m_text.setOpacity(op);
    m_icon.setColor(content);
    m_icon.setOpacity(op);
}

void AKButton::setBackgroundColor(CZAdaptiveColor color) noexcept
{
    if (m_backgroundColor == color)
        return;

    m_backgroundColor = color;
    updateStyleColor();
}

void AKButton::setContentColor(CZAdaptiveColor color) noexcept
{
    if (m_contentColor == color)
        return;

    m_contentColor = color;

    if (!m_contentColorAuto)
        updateStyleColor();
}

void AKButton::setAutoContentColor(bool enabled) noexcept
{
    if (m_contentColorAuto == enabled)
        return;

    m_contentColorAuto = enabled;
    updateStyleColor();
}
