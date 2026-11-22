#include <CZ/AK/Nodes/AKSwitch.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/Core/CZEase.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZPointerLeaveEvent.h>
#include <CZ/Core/Events/CZColorSchemeEvent.h>
#include <linux/input-event-codes.h>

using namespace CZ;

static constexpr SkScalar Inset { 2.f }; // Between the knob and the pill's edge
static constexpr CZAdaptiveColor OffColor { 0xFFE3E3E8, 0xFF4A4A4E };
static constexpr CZAdaptiveColor KnobStroke { 0x1F000000, 0x33000000 };

AKSwitch::AKSwitch(bool on, AKNode *parent) noexcept :
    AKRoundSolidColor(0, CZBorderRadius::Make(Int32(Height / 2.f)), parent),
    m_onColor(AKTheme::ControlAccent),
    m_on(on)
{
    layout().setWidth(Width);
    layout().setHeight(Height);
    layout().setFlexShrink(0.f);

    const SkScalar knobSize { Height - 2.f * Inset };
    const SkRegion empty;
    knob.setInputRegion(&empty);
    knob.setBorderRadius(CZBorderRadius::Make(Int32(knobSize / 2.f)));
    knob.setStrokeWidth(1);
    knob.layout().setPositionType(YGPositionTypeAbsolute);
    knob.layout().setPosition(YGEdgeTop, Inset);
    knob.layout().setWidth(knobSize);
    knob.layout().setHeight(knobSize);

    m_anim.setDuration(AnimationMs);
    m_anim.onUpdate.subscribe(this, [this](CZAnimation *a){
        const SkScalar to { m_on ? 1.f : 0.f };
        setProgress(m_from + (to - m_from) * SkScalar(CZEase::OutCubic(a->value())));
    });
    m_anim.onFinish.subscribe(this, [this](CZAnimation*){ setProgress(m_on ? 1.f : 0.f); });

    setProgress(on ? 1.f : 0.f);
}

void AKSwitch::setOn(bool on, bool animated) noexcept
{
    if (m_on == on)
        return;

    m_on = on;
    m_anim.pause();

    if (!animated)
    {
        setProgress(on ? 1.f : 0.f);
        return;
    }

    m_from = m_progress;
    m_anim.reset();
}

void AKSwitch::setEnabled(bool enabled) noexcept
{
    m_enabled = enabled;
    m_pressed = false;
    setOpacity(enabled ? 1.f : 0.5f);
    knob.setOpacity(enabled ? 1.f : 0.5f);
}

void AKSwitch::setOnColor(CZAdaptiveColor color) noexcept
{
    m_onColor = color;
    updateColors();
}

void AKSwitch::setProgress(SkScalar progress) noexcept
{
    m_progress = progress;
    const SkScalar knobSize { Height - 2.f * Inset };
    knob.layout().setPosition(YGEdgeLeft, Inset + (Width - knobSize - 2.f * Inset) * progress);
    updateColors();
}

void AKSwitch::updateColors() noexcept
{
    // The pill blends from gray to the accent color as the knob slides
    const CZColorScheme scheme { colorScheme() };
    const SkColor4f off { SkColor4f::FromColor(OffColor.resolve(scheme)) };
    const SkColor4f onC { SkColor4f::FromColor(m_onColor.resolve(scheme)) };
    const SkScalar t { m_progress };
    const SkColor4f mix { off.fR + (onC.fR - off.fR) * t, off.fG + (onC.fG - off.fG) * t,
                          off.fB + (onC.fB - off.fB) * t, off.fA + (onC.fA - off.fA) * t };
    setBackgroundColor(mix.toSkColor());
    knob.setStrokeColor(KnobStroke.resolve(scheme));
}

void AKSwitch::pointerButtonEvent(const CZPointerButtonEvent &e)
{
    AKRoundSolidColor::pointerButtonEvent(e);

    if (!m_enabled || e.button != BTN_LEFT)
        return;

    if (e.pressed)
        m_pressed = true;
    else if (m_pressed)
    {
        m_pressed = false;
        setOn(!m_on);
        onToggled.notify(m_on);
    }
}

void AKSwitch::pointerLeaveEvent(const CZPointerLeaveEvent &e)
{
    AKRoundSolidColor::pointerLeaveEvent(e);
    m_pressed = false;
}

void AKSwitch::colorSchemeEvent(const CZColorSchemeEvent &e)
{
    AKRoundSolidColor::colorSchemeEvent(e);
    updateColors();
}
