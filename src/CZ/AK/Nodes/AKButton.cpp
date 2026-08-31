#include <CZ/Core/Cuarzo.h>
#include <CZ/Core/Events/CZPointerEnterEvent.h>
#include <CZ/Core/Events/CZPointerLeaveEvent.h>
#include <CZ/Core/Events/CZPointerMoveEvent.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZWindowStateEvent.h>
#include <CZ/Core/Events/CZColorSchemeEvent.h>
#include <CZ/Core/Events/CZLayoutEvent.h>
#include <CZ/AK/AKScene.h>
#include <CZ/AK/Nodes/AKButton.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKLog.h>
#include <CZ/Core/Utils/CZColorUtils.h>
#include <CZ/Core/CZEase.h>

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
    setBorderRadius(10);
    setStrokeWidth(0); // solid, borderless

    SkRegion empty;
    m_text.setInputRegion(&empty);
    m_icon.setInputRegion(&empty);
    m_text.enableReplaceImageColor(true);
    m_icon.enableReplaceImageColor(true);
    m_icon.setSize(20);

    m_text.setFontStyle(SkFontStyle(
        SkFontStyle::kMedium_Weight,
        SkFontStyle::Width::kNormal_Width,
        SkFontStyle::Slant::kUpright_Slant));
    m_text.setFontSize(14);
    m_text.setFontFamilies({ SkString("Roboto") });
    m_text.setLetterSpacing(0.1);
    m_text.setLineHeight(20);

    // Default fill = macOS control accent; content color is auto-derived from it by default
    // (the explicit content color below is the fallback used when auto is disabled).
    m_backgroundColor = AKTheme::ControlAccent;
    m_offToggleBackgroundColor = AKTheme::SystemFill;
    m_contentColor = AKTheme::AlternateSelectedControlText;

    // Short color transitions: apply the eased colors each step and settle exactly on finish. An
    // interruption (a new target while running) is handled in startColorTransition(), which resets
    // the animation from the current interpolated color.
    m_colorAnim.setDuration(140);
    m_colorAnim.onUpdate.subscribe(&m_colorAnim, [this](CZAnimation*){ applyAnimatedColors(); });
    m_colorAnim.onFinish.subscribe(&m_colorAnim, [this](CZAnimation*){ applyColors(m_bgTo, m_contentTo); });

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
        {
            if (m_toggleable)
                setToggled(!m_toggled);

            onClick.notify(event);
        }

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

void AKButton::colorSchemeEvent(const CZColorSchemeEvent &event)
{
    AKRoundSolidColor::colorSchemeEvent(event); // updates colorScheme()
    updateStyleColor();                         // re-resolve the adaptive fills/content for it
}

void AKButton::updateStyleColor() noexcept
{
    // Interaction state opacities for the translucent overlay blended over the fill.
    static constexpr Float32 hoverOpacity    { 0.08f };
    static constexpr Float32 pressedOpacity  { 0.12f };
    static constexpr Float32 disabledOpacity { 0.38f };

    const CZColorScheme scheme { colorScheme() };

    // The base (idle) fill follows the toggle state: backgroundColor() when on, offToggleBackgroundColor()
    // when off. Non-toggle buttons always use backgroundColor(). This also drives the content color.
    const bool off { m_toggleable && !m_toggled };
    const SkColor fill { (off ? m_offToggleBackgroundColor : m_backgroundColor).resolve(scheme) };

    // Effective content color. In automatic mode it contrasts with what the fill actually looks
    // like: white (AKTheme::AlternateSelectedControlText) over dark fills, dark ink (AKTheme::Label)
    // over light ones. Two refinements over a naive luma check:
    //  - The fill is first composited over the surface behind the button (SystemBackground for the
    //    current scheme), so a translucent fill is judged by what actually shows through rather than
    //    its own RGB — e.g. a low-opacity light fill over a dark surface reads dark and gets light
    //    content, and the same fill over a light surface reads light and gets dark content.
    //  - The darkness cutoff is raised above the 0.5 midpoint so mid-luminance fills (e.g. the
    //    system green) count as dark and get readable white content.
    // Endpoints are fixed (.light()) since the choice depends on the fill's apparent luminance, not
    // the app's scheme. Otherwise the set content color resolved for the current scheme is used.
    SkColor content;

    if (m_contentColorAuto)
    {
        static constexpr Float32 darkFillThreshold { 0.6f };
        const SkColor surface { AKTheme::SystemBackground.resolve(scheme) };
        const SkColor effectiveFill { CZColorUtils::SrcOver(surface, fill) };
        content = CZColorUtils::IsColorDark(effectiveFill, darkFillThreshold)
            ? AKTheme::AlternateSelectedControlText.light()
            : AKTheme::Label.light();
    }
    else
        content = m_contentColor.resolve(colorScheme());

    // Precompute the fill for each interaction state (idle / hover / pressed) of the active toggle
    // state, then pick the current one — no dynamic blending in the hot path. In automatic mode the
    // hover/pressed fills are the idle fill plus a translucent state layer (the content color); in
    // manual mode the explicit per-state colors are used verbatim (see setAutoBackgroundColor).
    SkColor hover, pressed;

    if (m_backgroundColorAuto)
    {
        hover   = CZColorUtils::StateSrcOver(fill, content, hoverOpacity);
        pressed = CZColorUtils::StateSrcOver(fill, content, pressedOpacity);
    }
    else
    {
        hover   = (off ? m_offToggleHoverBackgroundColor   : m_hoverBackgroundColor).resolve(scheme);
        pressed = (off ? m_offTogglePressedBackgroundColor : m_pressedBackgroundColor).resolve(scheme);
    }

    SkColor bg { fill };

    if (m_enabled && m_pressed)
        bg = pressed;
    else if (m_enabled && m_hovered)
        bg = hover;

    // Opacity (the disabled dim) is applied directly, not animated.
    const Float32 op { m_enabled ? 1.f : disabledOpacity };
    setOpacity(op);
    m_text.setOpacity(op);
    m_icon.setOpacity(op);

    // Record the desired colors and drive them through the color animation.
    m_bgTarget = bg;
    m_contentTarget = content;

    if (!m_colorsSeeded)
    {
        // First run: no animation, snap to the initial colors.
        m_colorsSeeded = true;
        m_bgFrom = m_bgTo = bg;
        m_contentFrom = m_contentTo = content;
        applyColors(bg, content);
        return;
    }

    startColorTransition();
}

void AKButton::startColorTransition() noexcept
{
    // Start from the currently displayed colors (the interpolated value if a transition is already
    // in flight), so an interruption redirects immediately from where we are instead of waiting.
    const Float64 t { CZEase::InOutCubic(m_colorAnim.value()) };
    const SkColor curBg { CZColorUtils::Mix(m_bgTo, m_bgFrom, t) };
    const SkColor curContent { CZColorUtils::Mix(m_contentTo, m_contentFrom, t) };

    m_bgFrom = curBg;
    m_bgTo = m_bgTarget;
    m_contentFrom = curContent;
    m_contentTo = m_contentTarget;

    if (m_bgFrom == m_bgTo && m_contentFrom == m_contentTo)
    {
        // Already at the target: settle here and stop any running transition.
        m_colorAnim.pause();
        applyColors(m_bgTo, m_contentTo);
        return;
    }

    m_colorAnim.reset(); // restart from the beginning, even if currently running
}

void AKButton::applyAnimatedColors() noexcept
{
    const Float64 t { CZEase::InOutCubic(m_colorAnim.value()) };
    applyColors(CZColorUtils::Mix(m_bgTo, m_bgFrom, t),
                CZColorUtils::Mix(m_contentTo, m_contentFrom, t));
}

void AKButton::applyColors(SkColor bg, SkColor content) noexcept
{
    AKRoundSolidColor::setBackgroundColor(bg);
    m_text.setColor(content);
    m_icon.setColor(content);
}

void AKButton::setBackgroundColor(CZAdaptiveColor color) noexcept
{
    if (m_backgroundColor == color)
        return;

    m_backgroundColor = color;
    updateStyleColor();
}

void AKButton::setOffToggleBackgroundColor(CZAdaptiveColor color) noexcept
{
    if (m_offToggleBackgroundColor == color)
        return;

    m_offToggleBackgroundColor = color;
    updateStyleColor();
}

void AKButton::setToggleable(bool toggleable) noexcept
{
    if (m_toggleable == toggleable)
        return;

    m_toggleable = toggleable;
    updateStyleColor();
}

void AKButton::setToggled(bool toggled) noexcept
{
    if (m_toggled == toggled)
        return;

    m_toggled = toggled;
    addChange(CHToggled);
    updateStyleColor();
    onToggled.notify(m_toggled);
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

void AKButton::setHoverBackgroundColor(CZAdaptiveColor color) noexcept
{
    if (m_hoverBackgroundColor == color)
        return;

    m_hoverBackgroundColor = color;

    if (!m_backgroundColorAuto)
        updateStyleColor();
}

void AKButton::setPressedBackgroundColor(CZAdaptiveColor color) noexcept
{
    if (m_pressedBackgroundColor == color)
        return;

    m_pressedBackgroundColor = color;

    if (!m_backgroundColorAuto)
        updateStyleColor();
}

void AKButton::setOffToggleHoverBackgroundColor(CZAdaptiveColor color) noexcept
{
    if (m_offToggleHoverBackgroundColor == color)
        return;

    m_offToggleHoverBackgroundColor = color;

    if (!m_backgroundColorAuto)
        updateStyleColor();
}

void AKButton::setOffTogglePressedBackgroundColor(CZAdaptiveColor color) noexcept
{
    if (m_offTogglePressedBackgroundColor == color)
        return;

    m_offTogglePressedBackgroundColor = color;

    if (!m_backgroundColorAuto)
        updateStyleColor();
}

void AKButton::setAutoBackgroundColor(bool enabled) noexcept
{
    if (m_backgroundColorAuto == enabled)
        return;

    m_backgroundColorAuto = enabled;
    updateStyleColor();
}
