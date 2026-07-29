#include <CZ/Core/Events/CZPointerEnterEvent.h>
#include <CZ/Core/Events/CZPointerLeaveEvent.h>
#include <CZ/Core/Events/CZWindowStateEvent.h>
#include <CZ/Core/Events/CZColorSchemeEvent.h>
#include <CZ/AK/Nodes/AKButtonGroup.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/Core/Utils/CZColorUtils.h>
#include <CZ/Core/CZEase.h>
#include <CZ/skia/core/SkColor.h>

using namespace CZ;

AKButtonGroup::AKButtonGroup(AKNode *parent) noexcept :
    AKRoundSolidColor(SK_ColorTRANSPARENT, 10, parent)
{
    setStrokeWidth(0);
    m_hoverColor = AKTheme::TertiarySystemFill;

    // Pack the buttons in a row, no gaps, and shrink to fit them (no padding, no cross-axis stretch).
    layout().setFlexDirection(YGFlexDirectionRow);
    layout().setAlignItems(YGAlignCenter);
    layout().setJustifyContent(YGJustifyCenter);
    layout().setGap(YGGutterAll, 0.f);
    layout().setWidthAuto();
    layout().setAlignSelf(YGAlignFlexStart);

    // Short hover-fill fade. On interruption startHoverTransition() restarts it from the current
    // color, so entering/leaving quickly redirects smoothly.
    m_hoverAnim.setDuration(140);
    m_hoverAnim.onUpdate.subscribe(&m_hoverAnim, [this](CZAnimation*){ applyAnimatedHover(); });
    m_hoverAnim.onFinish.subscribe(&m_hoverAnim, [this](CZAnimation*){ AKRoundSolidColor::setBackgroundColor(m_hoverTo); });

    // Style AKButton children as they are added, and revert on removal. A child being destroyed
    // fails the dynamic_cast (its derived part is already gone) and is safely ignored.
    onChildAdded.subscribe(this, [this](AKNode *added, AKNode*)
    {
        if (AKButton *button { dynamic_cast<AKButton*>(added) })
            styleButton(button);
    });

    onChildRemoved.subscribe(this, [this](AKNode *removed)
    {
        if (AKButton *button { dynamic_cast<AKButton*>(removed) })
            unstyleButton(button);
    });
}

void AKButtonGroup::setToggleMode(ToggleMode mode) noexcept
{
    if (m_toggleMode == mode)
        return;

    m_toggleMode = mode;
    addChange(CHToggleMode);

    // Re-apply the toggle capability and the mode-dependent color ladder to the current buttons.
    for (AKNode *child : children())
    {
        AKButton *button { dynamic_cast<AKButton*>(child) };
        if (!button)
            continue;

        button->setToggleable(mode != ToggleMode::None);

        if (mode == ToggleMode::None)
            button->setToggled(false);

        applyButtonColors(button);
    }
}

void AKButtonGroup::setHoverColor(CZAdaptiveColor color) noexcept
{
    if (m_hoverColor == color)
        return;

    m_hoverColor = color;

    if (m_hovered)
    {
        m_hoverTarget = m_hoverColor.resolve(colorScheme());
        startHoverTransition();
    }
}

void AKButtonGroup::setButtonColor(CZAdaptiveColor color) noexcept
{
    if (m_buttonColor == color)
        return;

    m_buttonColor = color;

    for (AKNode *child : children())
        if (AKButton *button { dynamic_cast<AKButton*>(child) })
            applyButtonColors(button);
}

void AKButtonGroup::setBorderRadius(const CZBorderRadius &borderRadius) noexcept
{
    AKRoundSolidColor::setBorderRadius(borderRadius);

    // Keep the grouped buttons' corners equal to the group's.
    for (AKNode *child : children())
        if (AKButton *button { dynamic_cast<AKButton*>(child) })
            button->setBorderRadius(borderRadius);
}

// Builds an adaptive color from a tint at a given opacity (applied to both light and dark variants).
static CZAdaptiveColor tintAt(const CZAdaptiveColor &tint, Float32 alpha) noexcept
{
    const UInt8 a { static_cast<UInt8>(alpha * 255.f + 0.5f) };
    return CZAdaptiveColor(SkColorSetA(tint.light(), a), SkColorSetA(tint.dark(), a));
}

void AKButtonGroup::applyButtonColors(AKButton *button) const noexcept
{
    // Alpha ladder over the group tint. When the buttons are toggles, the "on" set (used while
    // toggled on) is the selected, stronger ladder and the "off" set (toggled off) is the subtle
    // one; for a plain group (no toggle) the "on" set — the only one in use — is the subtle ladder.
    // Per state: transparent at rest, a subtle tint on hover / when toggled on, stronger when pressed
    // (or toggled and hovered/pressed).
    const bool toggle { m_toggleMode != ToggleMode::None };

    button->setBackgroundColor(tintAt(m_buttonColor, toggle ? 0.06f : 0.00f));
    button->setHoverBackgroundColor(tintAt(m_buttonColor, toggle ? 0.11f : 0.06f));
    button->setPressedBackgroundColor(tintAt(m_buttonColor, toggle ? 0.16f : 0.11f));

    button->setOffToggleBackgroundColor(tintAt(m_buttonColor, 0.00f));
    button->setOffToggleHoverBackgroundColor(tintAt(m_buttonColor, 0.06f));
    button->setOffTogglePressedBackgroundColor(tintAt(m_buttonColor, 0.11f));
}

void AKButtonGroup::styleButton(AKButton *button) noexcept
{
    if (m_prevStyle.find(button) != m_prevStyle.end())
        return; // already styled

    // Remember the properties we override so removal can restore them.
    m_prevStyle[button] = ButtonStyle {
        button->toggleable(),
        button->borderRadius(),
        button->hasAutoContentColor(),
        button->contentColor(),
        button->hasAutoBackgroundColor(),
        button->backgroundColor(),
        button->hoverBackgroundColor(),
        button->pressedBackgroundColor(),
        button->offToggleBackgroundColor(),
        button->offToggleHoverBackgroundColor(),
        button->offTogglePressedBackgroundColor()
    };

    button->setToggleable(m_toggleMode != ToggleMode::None);
    button->setBorderRadius(borderRadius());     // same corners as the group
    button->layout().setMargin(YGEdgeAll, 0.f);  // packed, no margins

    // Keep the label readable over the mostly-transparent fills in either color scheme.
    button->setAutoContentColor(false);
    button->setContentColor(AKTheme::Label);

    // The group owns the fill: explicit per-state colors from its tint (no auto state layer).
    button->setAutoBackgroundColor(false);
    applyButtonColors(button);

    // Enforce single-selection: when a button turns on, turn the others off. Subscribed regardless
    // of the mode (the handler checks it live), so mode changes never need to re-subscribe. Gated on
    // parent()==this so a button removed but still alive no longer drives the group.
    button->onToggled.subscribe(this, [this, button](bool on)
    {
        if (m_toggleMode != ToggleMode::Single || !on || button->parent() != this)
            return;

        for (AKNode *child : children())
        {
            AKButton *other { dynamic_cast<AKButton*>(child) };
            if (other && other != button)
                other->setToggled(false);
        }
    });
}

void AKButtonGroup::unstyleButton(AKButton *button) noexcept
{
    auto it { m_prevStyle.find(button) };
    if (it == m_prevStyle.end())
        return;

    // Revert to the style the button had before joining the group.
    const ButtonStyle &prev { it->second };
    button->setToggled(false);
    button->setToggleable(prev.toggleable);
    button->setBorderRadius(prev.borderRadius);
    button->setAutoContentColor(prev.autoContentColor);
    button->setContentColor(prev.contentColor);
    button->setAutoBackgroundColor(prev.autoBackgroundColor);
    button->setBackgroundColor(prev.backgroundColor);
    button->setHoverBackgroundColor(prev.hoverBackgroundColor);
    button->setPressedBackgroundColor(prev.pressedBackgroundColor);
    button->setOffToggleBackgroundColor(prev.offToggleBackgroundColor);
    button->setOffToggleHoverBackgroundColor(prev.offToggleHoverBackgroundColor);
    button->setOffTogglePressedBackgroundColor(prev.offTogglePressedBackgroundColor);
    m_prevStyle.erase(it);
}

void AKButtonGroup::onSceneBegin()
{
    // A button parented at construction time (AKButton{..., &group}) fires onChildAdded from the base
    // AKNode constructor, before its AKButton part exists — so the dynamic_cast there fails and it is
    // not styled at add time. Catch any not-yet-styled button children here, once fully constructed.
    for (AKNode *child : children())
        if (AKButton *button { dynamic_cast<AKButton*>(child) })
            styleButton(button);

    AKRoundSolidColor::onSceneBegin();
}

void AKButtonGroup::pointerEnterEvent(const CZPointerEnterEvent &event)
{
    AKRoundSolidColor::pointerEnterEvent(event);
    setHovered(true);
}

void AKButtonGroup::pointerLeaveEvent(const CZPointerLeaveEvent &event)
{
    AKRoundSolidColor::pointerLeaveEvent(event);
    setHovered(false);
}

void AKButtonGroup::windowStateEvent(const CZWindowStateEvent &event)
{
    AKRoundSolidColor::windowStateEvent(event);

    // Re-resolve the hover fill for a possible color-scheme change.
    if (m_hovered)
    {
        m_hoverTarget = m_hoverColor.resolve(colorScheme());
        startHoverTransition();
    }

    event.accept();
}

void AKButtonGroup::colorSchemeEvent(const CZColorSchemeEvent &event)
{
    AKRoundSolidColor::colorSchemeEvent(event);

    // The grouped buttons re-resolve their own colors; re-resolve our hover backdrop too.
    if (m_hovered)
    {
        m_hoverTarget = m_hoverColor.resolve(colorScheme());
        startHoverTransition();
    }
}

void AKButtonGroup::setHovered(bool hovered) noexcept
{
    if (m_hovered == hovered)
        return;

    m_hovered = hovered;
    m_hoverTarget = hovered ? m_hoverColor.resolve(colorScheme()) : SK_ColorTRANSPARENT;
    startHoverTransition();
}

void AKButtonGroup::startHoverTransition() noexcept
{
    // Restart from the currently displayed color (the interpolated value if a fade is in flight),
    // so an interruption redirects immediately from where we are.
    const Float64 t { CZEase::InOutCubic(m_hoverAnim.value()) };
    m_hoverFrom = CZColorUtils::Mix(m_hoverTo, m_hoverFrom, t);
    m_hoverTo = m_hoverTarget;

    if (m_hoverFrom == m_hoverTo)
    {
        m_hoverAnim.pause();
        AKRoundSolidColor::setBackgroundColor(m_hoverTo);
        return;
    }

    m_hoverAnim.reset();
}

void AKButtonGroup::applyAnimatedHover() noexcept
{
    const Float64 t { CZEase::InOutCubic(m_hoverAnim.value()) };
    AKRoundSolidColor::setBackgroundColor(CZColorUtils::Mix(m_hoverTo, m_hoverFrom, t));
}
