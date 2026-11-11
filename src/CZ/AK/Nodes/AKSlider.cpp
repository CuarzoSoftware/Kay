#include <CZ/AK/Nodes/AKSlider.h>
#include <CZ/AK/AKScene.h>
#include <CZ/AK/AKApp.h>
#include <CZ/Core/Events/CZLayoutEvent.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZPointerMoveEvent.h>
#include <linux/input-event-codes.h>
#include <algorithm>
#include <cmath>

using namespace CZ;

AKSlider::AKSlider(AKNode *parent) noexcept :
    AKContainer(YGFlexDirectionRow, false, parent),
    m_track(0, CZBorderRadius::Make(0), this),
    m_fill(0, CZBorderRadius::Make(0), this),
    m_icon(this),
    m_knob(0, CZBorderRadius::Make(0), this)
{
    layout().setHeight(22.f);

    // The parts take no input: the slider gets it all.
    const SkRegion empty;

    for (AKNode *part : std::initializer_list<AKNode*>{ &m_track, &m_fill, &m_icon, &m_knob })
    {
        part->setInputRegion(&empty);
        part->layout().setPositionType(YGPositionTypeAbsolute);
    }

    m_track.layout().setPosition(YGEdgeLeft, 0.f);
    m_track.layout().setPosition(YGEdgeTop, 0.f);
    m_track.layout().setWidthPercent(100.f);
    m_track.layout().setHeightPercent(100.f);
    m_fill.layout().setPosition(YGEdgeLeft, 0.f);
    m_fill.layout().setPosition(YGEdgeTop, 0.f);
    m_fill.layout().setHeightPercent(100.f);
    m_knob.layout().setPosition(YGEdgeTop, 0.f);
    m_knob.layout().setHeightPercent(100.f);

    m_icon.setVisible(false);
    m_icon.setSizeMode(AKImageFrame::SizeMode::Contain);
    m_icon.setAlignment(CZAlignCenter);
    m_icon.enableReplaceImageColor(true);
    m_knob.setStrokeWidth(1);

    updateColors();
    updateGeometry();
}

bool AKSlider::setValue(SkScalar value) noexcept
{
    value = std::clamp(value, 0.f, 1.f);

    if (m_dragging || value == m_value)
        return false;

    m_value = value;
    updateGeometry();
    return true;
}

void AKSlider::setIcon(std::shared_ptr<RImage> icon) noexcept
{
    m_icon.setImage(icon);
    m_icon.setVisible(icon != nullptr);
}

void AKSlider::setTrackColor(CZAdaptiveColor color) noexcept { m_trackColor = color; updateColors(); }
void AKSlider::setFillColor(CZAdaptiveColor color) noexcept { m_fillColor = color; updateColors(); }
void AKSlider::setKnobColor(CZAdaptiveColor color) noexcept { m_knobColor = color; updateColors(); }
void AKSlider::setIconColor(CZAdaptiveColor color) noexcept { m_iconColor = color; updateColors(); }

void AKSlider::updateColors() noexcept
{
    const CZColorScheme scheme { colorScheme() };
    m_track.setBackgroundColor(m_trackColor.resolve(scheme));
    m_fill.setBackgroundColor(m_fillColor.resolve(scheme));
    m_knob.setBackgroundColor(m_knobColor.resolve(scheme));
    m_knob.setStrokeColor(m_knobStroke.resolve(scheme));
    m_icon.setColor(m_iconColor.resolve(scheme));
}

void AKSlider::updateGeometry() noexcept
{
    const SkScalar w { layout().calculatedWidth() }, h { layout().calculatedHeight() };

    if (!(w > 0.f) || !(h > 0.f))
        return;

    // The knob travels over [0, w - h]; the fill reaches its right end (never shorter than the knob).
    const SkScalar knobX { std::round(m_value * (w - h)) };
    const Int32 radius { Int32(h / 2.f) };

    m_track.setBorderRadius(CZBorderRadius::Make(radius));
    m_fill.setBorderRadius(CZBorderRadius::Make(radius));
    m_knob.setBorderRadius(CZBorderRadius::Make(radius));
    m_fill.layout().setWidth(knobX + h);
    m_knob.layout().setPosition(YGEdgeLeft, knobX);
    m_knob.layout().setWidth(h);

    // The icon at the left end, inside the knob-sized area (under the knob when the value is 0).
    const SkScalar iconSize { std::round(h * 0.6f) };
    m_icon.layout().setPosition(YGEdgeLeft, std::round((h - iconSize) / 2.f) + 2.f);
    m_icon.layout().setPosition(YGEdgeTop, std::round((h - iconSize) / 2.f));
    m_icon.layout().setWidth(iconSize);
    m_icon.layout().setHeight(iconSize);
    m_icon.insertBefore(&m_knob);
}

void AKSlider::setValueByPointer() noexcept
{
    const SkIRect r { worldRect() };
    const SkScalar h { SkScalar(r.height()) };
    const SkScalar range { SkScalar(r.width()) - h };

    if (range <= 0.f)
        return;

    // The knob's center follows the cursor.
    const SkScalar x { AKApp::Get()->pointer().pos().x() - r.x() - h / 2.f };
    const SkScalar value { std::clamp(x / range, 0.f, 1.f) };

    if (value == m_value)
        return;

    m_value = value;
    m_changedWhileDragging = true;
    updateGeometry();
    onValueChanged.notify(m_value);
}

void AKSlider::layoutEvent(const CZLayoutEvent &e)
{
    AKContainer::layoutEvent(e);

    if (e.changes.has(CZLayoutChangeSize))
        updateGeometry();
}

void AKSlider::colorSchemeEvent(const CZColorSchemeEvent &e)
{
    AKContainer::colorSchemeEvent(e);
    updateColors();
}

void AKSlider::pointerButtonEvent(const CZPointerButtonEvent &e)
{
    AKContainer::pointerButtonEvent(e);

    if (e.button != BTN_LEFT)
        return;

    if (e.pressed)
    {
        m_dragging = true;
        m_changedWhileDragging = false;

        if (scene())
            scene()->setPointerGrab(this);

        setValueByPointer();
    }
    else if (m_dragging)
    {
        m_dragging = false;

        if (scene() && scene()->pointerGrab() == this)
            scene()->setPointerGrab(nullptr);

        if (m_changedWhileDragging)
            onReleased.notify(m_value);
    }
}

void AKSlider::pointerMoveEvent(const CZPointerMoveEvent &e)
{
    AKContainer::pointerMoveEvent(e);

    if (m_dragging)
        setValueByPointer();
}
