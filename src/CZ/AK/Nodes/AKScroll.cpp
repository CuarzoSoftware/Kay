#include <CZ/Core/Events/CZLayoutEvent.h>
#include <CZ/AK/Nodes/AKScroll.h>
#include <CZ/Core/Events/CZKeyboardKeyEvent.h>
#include <CZ/AK/Events/AKSceneChangedEvent.h>
#include <CZ/Core/Events/CZPointerScrollEvent.h>
#include <CZ/AK/AKScene.h>
#include <CZ/AK/AKTheme.h>
#include <linux/input-event-codes.h>
#include <algorithm>
#include <cmath>
#include <CZ/Core/CZTime.h>

using namespace CZ;

AKScroll::AKScroll(AKNode *parent) noexcept :
    AKContainer(YGFlexDirectionColumn, true, parent),
    m_slot(YGFlexDirectionColumn, false, this)
{

    setSlot(&m_slot);
    layout().setFlexGrow(1.f);
    layout().setOverflow(YGOverflowScroll);
    m_slot.layout().setOverflow(YGOverflowScroll);
    m_slot.layout().setPositionType(YGPositionTypeAbsolute);
    m_slot.layout().setPosition(YGEdgeTop, 0.f);
    m_slot.layout().setPosition(YGEdgeLeft, 0.f);
    m_slot.layout().setWidthPercent(100.f);
    m_slot.layout().setHeightPercent(100.f);

    // A ticker: the axes compute their own time steps (see tick()).
    m_ticker.setDuration(3600000);
    m_ticker.onUpdate.subscribe(&m_ticker, [this](CZAnimation*){ tick(); });
    m_ticker.onFinish.subscribe(&m_ticker, [this](CZAnimation*){
        for (const Axis &a : m_axes)
            if (a.phase != Axis::Phase::Idle && a.phase != Axis::Phase::Tracking)
                return m_ticker.start();
    });
}

SkScalar AKScroll::offsetX() const noexcept
{
    return m_slot.layout().position(YGEdgeLeft).value;
}

SkScalar AKScroll::offsetY() const noexcept
{
    return m_slot.layout().position(YGEdgeTop).value;
}

void AKScroll::setOffset(SkScalar x, SkScalar y) noexcept
{
    stopKinetic();
    m_slot.layout().setPosition(YGEdgeTop, y);
    m_slot.layout().setPosition(YGEdgeLeft, x);
    calculateContentBounds(); // The content may have changed since (e.g. more items)
    applyConstraints();
}

void AKScroll::setOffsetX(Int32 x) noexcept
{
    stopKinetic();
    m_slot.layout().setPosition(YGEdgeLeft, x);
    calculateContentBounds(); // The content may have changed since (e.g. more items)
    applyConstraints();
}

void AKScroll::setOffsetY(Int32 y) noexcept
{
    stopKinetic();
    m_slot.layout().setPosition(YGEdgeTop, y);
    calculateContentBounds(); // The content may have changed since (e.g. more items)
    applyConstraints();
}

void AKScroll::setOffsetXPercent(SkScalar x) noexcept
{
    if (x < 0.f) x = 0.f;
    else if (x > 100.f) x = 100.f;
    setOffsetX(-m_contentBounds.width() * x + SkScalar(m_contentBounds.fLeft));
}

void AKScroll::setOffsetYPercent(SkScalar y) noexcept
{
    if (y < 0.f) y = 0.f;
    else if (y > 100.f) y = 100.f;
    setOffsetY(-m_contentBounds.height() * y + SkScalar(m_contentBounds.fTop));
}

static void contentBounds(AKNode *root, SkIRect *bounds) noexcept
{
    if (!root->visible())
        return;

    if (root->layout().calculatedLeft() < bounds->fLeft)
        bounds->fLeft = root->layout().calculatedLeft();

    if (root->layout().calculatedTop() < bounds->fTop)
        bounds->fTop = root->layout().calculatedTop();

    const Int32 right = root->layout().calculatedLeft() + root->layout().calculatedWidth();
    if (right > bounds->fRight)
        bounds->fRight = right;

    const Int32 bottom = root->layout().calculatedTop() + root->layout().calculatedHeight();
    if (bottom > bounds->fBottom)
        bounds->fBottom = bottom;

    if (root->childrenClippingEnabled())
        return;

    for (AKNode *child : root->children())
        contentBounds(child, bounds);
}

bool AKScroll::contentHidden(CZEdge edge) const noexcept
{
    // Half a pixel of tolerance (offsets aren't whole while kinetic scrolling settles)
    switch (edge)
    {
    case CZEdgeTop:    return offsetY() < -SkScalar(m_contentBounds.fTop) - 0.5f;
    case CZEdgeBottom: return SkScalar(m_contentBounds.fBottom) + offsetY() > layout().calculatedHeight() + 0.5f;
    case CZEdgeLeft:   return offsetX() < -SkScalar(m_contentBounds.fLeft) - 0.5f;
    case CZEdgeRight:  return SkScalar(m_contentBounds.fRight) + offsetX() > layout().calculatedWidth() + 0.5f;
    default:           return false;
    }
}

void AKScroll::calculateContentBounds() noexcept
{
    const SkIRect prevBounds { m_contentBounds };
    m_contentBounds.setEmpty();

    for (AKNode *child : m_slot.children())
        contentBounds(child, &m_contentBounds);

    m_contentBounds.fRight += m_slot.layout().calculatedPadding(YGEdgeRight);
    m_contentBounds.fBottom += m_slot.layout().calculatedPadding(YGEdgeBottom);

    if (m_contentBounds.fLeft > 0)
        m_contentBounds.fLeft = 0;

    if (m_contentBounds.fTop > 0)
        m_contentBounds.fTop = 0;

    // Content smaller than the viewport stays at its top-left: as if it filled it (otherwise the
    // bottom/right edge checks pull it down/right, e.g. a short list in a tall scroll).
    m_contentBounds.fRight = std::max(m_contentBounds.fRight, m_contentBounds.fLeft + Int32(layout().calculatedWidth()));
    m_contentBounds.fBottom = std::max(m_contentBounds.fBottom, m_contentBounds.fTop + Int32(layout().calculatedHeight()));

    if (m_contentBounds != prevBounds)
        onContentBoundsChanged.notify();
}

void AKScroll::applyConstraints() noexcept
{
    // The visible range [-offset, -offset + size] stays within the content bounds (at least as
    // big as the viewport, see calculateContentBounds()).
    const SkScalar w { layout().calculatedWidth() };
    const SkScalar h { layout().calculatedHeight() };
    const SkScalar x { std::clamp(offsetX(), std::min(w - SkScalar(m_contentBounds.fRight), -SkScalar(m_contentBounds.fLeft)), -SkScalar(m_contentBounds.fLeft)) };
    const SkScalar y { std::clamp(offsetY(), std::min(h - SkScalar(m_contentBounds.fBottom), -SkScalar(m_contentBounds.fTop)), -SkScalar(m_contentBounds.fTop)) };

    m_slot.layout().setPosition(YGEdgeTop, y);
    m_slot.layout().setPosition(YGEdgeLeft, x);

    // Not the user scrolling (other content, a new size, setOffset()): the bars don't show up
    updateBarXPrivate(false);
    updateBarYPrivate(false);
}

/* ------------------------------------ Kinetic scrolling ------------------------------------ */

// Apple's rubber band: past an edge, the content moves less the further it goes, never more than
// d (the viewport size, up to AKTheme::ScrollRubberBandLimit).
static SkScalar RubberBand(SkScalar x, SkScalar d) noexcept
{
    if (d <= 0.f) return 0.f;
    const SkScalar c { AKTheme::ScrollRubberBandCoefficient };
    const SkScalar a { std::abs(x) };
    return std::copysign((1.f - 1.f / (a * c / d + 1.f)) * d, x);
}

// Inverse of RubberBand(): the finger distance that shows @p f.
static SkScalar RubberBandInverse(SkScalar f, SkScalar d) noexcept
{
    if (d <= 0.f) return 0.f;
    const SkScalar c { AKTheme::ScrollRubberBandCoefficient };
    const SkScalar a { std::min(std::abs(f), d * 0.99f) };
    return std::copysign((d / c) * (1.f / (1.f - a / d) - 1.f), f);
}

SkScalar AKScroll::viewportSize(int axis) const noexcept
{
    return axis == 0 ? layout().calculatedWidth() : layout().calculatedHeight();
}

void AKScroll::scrollRange(int axis, SkScalar &min, SkScalar &max) const noexcept
{
    // See applyConstraints()
    const SkScalar start { SkScalar(axis == 0 ? m_contentBounds.fLeft : m_contentBounds.fTop) };
    const SkScalar end { SkScalar(axis == 0 ? m_contentBounds.fRight : m_contentBounds.fBottom) };
    max = -start;
    min = std::min(viewportSize(axis) - end, max);
}

void AKScroll::setOffsetPrivate(int axis, SkScalar offset) noexcept
{
    if (axis == 0)
    {
        m_slot.layout().setPosition(YGEdgeLeft, offset);
        updateBarXPrivate();
    }
    else
    {
        m_slot.layout().setPosition(YGEdgeTop, offset);
        updateBarYPrivate();
    }
}

void AKScroll::stopKinetic() noexcept
{
    for (Axis &a : m_axes)
    {
        a.phase = Axis::Phase::Idle;
        a.vel = 0.f;
    }

    m_ticker.pause();
}

void AKScroll::scrollAxis(int axis, SkScalar delta, CZPointerScrollEvent::Source source, bool stop) noexcept
{
    Axis &a { m_axes[axis] };
    SkScalar min, max;
    scrollRange(axis, min, max);
    const SkScalar d { std::min(viewportSize(axis), AKTheme::ScrollRubberBandLimit) };
    const UInt64 now { CZTime::Us() };
    const SkScalar current { offset(axis) };

    if (source == CZPointerScrollEvent::Finger)
    {
        if (stop)
        {
            // Fingers lifted: keep the momentum (none if they rested before lifting), or spring
            // back if past an edge.
            if (a.phase != Axis::Phase::Tracking)
                return;

            if ((now - a.lastEventUs) > 64000)
                a.vel = 0.f;

            a.vel = std::clamp(a.vel, -10.f, 10.f);

            if (current > max || current < min)
            {
                // Continues from the finger distance (the rubber band is undone)
                a.phase = Axis::Phase::Spring;
                a.edge = std::clamp(current, min, max);
                a.overshoot = RubberBandInverse(current - a.edge, d);
            }
            else if (std::abs(a.vel) > 0.05f)
                a.phase = Axis::Phase::Momentum;
            else
            {
                a.phase = Axis::Phase::Idle;
                a.vel = 0.f;
                return;
            }

            m_lastTickUs = now;
            m_ticker.start();
            return;
        }

        if (a.phase != Axis::Phase::Tracking)
        {
            // Caught (even while it springs back): continue from where it's shown.
            const SkScalar edge { std::clamp(current, min, max) };
            a.raw = edge + RubberBandInverse(current - edge, d);
            a.vel = 0.f;
            a.lastEventUs = 0;
            a.phase = Axis::Phase::Tracking;
        }

        // Velocity: smoothed over the latest events (a long pause restarts it).
        const SkScalar dtMs { a.lastEventUs ? std::clamp(SkScalar(now - a.lastEventUs) / 1000.f, 1.f, 100.f) : 16.f };
        const SkScalar instant { delta / dtMs };
        a.vel = (a.lastEventUs && dtMs < 50.f) ? a.vel * 0.4f + instant * 0.6f : instant;
        a.lastEventUs = now;

        a.raw += delta;
        const SkScalar edge { std::clamp(a.raw, min, max) };
        setOffsetPrivate(axis, edge + RubberBand(a.raw - edge, d));
        return;
    }

    if (source == CZPointerScrollEvent::Wheel)
    {
        // Glides to the target, steps accumulate.
        const SkScalar from { a.phase == Axis::Phase::Wheel ? a.wheelTarget : std::clamp(current, min, max) };
        a.wheelTarget = std::clamp(from + delta, min, max);
        a.phase = Axis::Phase::Wheel;
        a.vel = 0.f;
        m_lastTickUs = now;
        m_ticker.start();
        return;
    }

    // Continuous (e.g. high resolution wheels) or unknown: moved right away.
    a.phase = Axis::Phase::Idle;
    a.vel = 0.f;
    setOffsetPrivate(axis, std::clamp(current + delta, min, max));
}

void AKScroll::tick() noexcept
{
    const UInt64 now { CZTime::Us() };
    const SkScalar dt { std::clamp(SkScalar(now - m_lastTickUs) / 1000.f, 0.f, 64.f) };
    m_lastTickUs = now;
    bool running { false };

    for (int axis = 0; axis < 2; axis++)
    {
        Axis &a { m_axes[axis] };

        if (a.phase == Axis::Phase::Idle || a.phase == Axis::Phase::Tracking)
            continue;

        SkScalar min, max;
        scrollRange(axis, min, max);
        const SkScalar d { std::min(viewportSize(axis), AKTheme::ScrollRubberBandLimit) };
        SkScalar o { offset(axis) };

        if (a.phase == Axis::Phase::Wheel)
        {
            const SkScalar target { std::clamp(a.wheelTarget, min, max) };
            o += (target - o) * (1.f - std::exp(-dt / 40.f));

            if (std::abs(target - o) < 0.5f)
            {
                o = target;
                a.phase = Axis::Phase::Idle;
            }
        }
        else if (a.phase == Axis::Phase::Momentum)
        {
            // v(t) = v0 * rate^t: integrated exactly over the step.
            const SkScalar decay { std::pow(AKTheme::ScrollDecelerationRate, dt) };
            const SkScalar k { -std::log(AKTheme::ScrollDecelerationRate) };
            o += a.vel * (1.f - decay) / k;
            a.vel *= decay;

            if (o > max || o < min)
            {
                // Overshoots the edge (rubber-banded), then springs back
                a.phase = Axis::Phase::Spring;
                a.edge = std::clamp(o, min, max);
                a.overshoot = o - a.edge;
                o = a.edge + RubberBand(a.overshoot, d);
            }
            else if (std::abs(a.vel) < 0.01f)
            {
                a.phase = Axis::Phase::Idle;
                a.vel = 0.f;
            }
        }
        else // Spring
        {
            // Critically damped, on the distance past the edge without the rubber band:
            // x(t) = (x0 + (v0 + w x0) t) e^(-w t). Shown rubber-banded.
            const SkScalar edge { std::clamp(a.edge, min, max) };
            const SkScalar w { AKTheme::ScrollBounceFrequency };
            const SkScalar x0 { a.overshoot }, v0 { a.vel };
            const SkScalar e { std::exp(-w * dt) };
            const SkScalar x { (x0 + (v0 + w * x0) * dt) * e };
            a.vel = (v0 - w * (v0 + w * x0) * dt) * e;
            a.overshoot = x;
            o = edge + RubberBand(x, d);

            // Back inside (it never overshoots it) or close enough: done.
            if ((x0 != 0.f && std::signbit(x) != std::signbit(x0)) || (std::abs(o - edge) < 0.5f && std::abs(a.vel) < 0.02f))
            {
                o = edge;
                a.phase = Axis::Phase::Idle;
                a.vel = 0.f;
            }
        }

        setOffsetPrivate(axis, o);
        running |= a.phase != Axis::Phase::Idle;
    }

    if (!running)
        m_ticker.pause();

    repaint();
}

void AKScroll::pointerScrollEvent(const CZPointerScrollEvent &e)
{
    calculateContentBounds();

    const auto source { e.source };
    const SkScalar speed { AKTheme::ScrollKineticSpeed * (source == CZPointerScrollEvent::Wheel ? 4.f : 1.f) };

    // Finger sources send 0 when the fingers are lifted (kinetic stop).
    if (e.hasX)
        scrollAxis(0, -e.axes.x() * speed, source, source == CZPointerScrollEvent::Finger && e.axes.x() == 0.f);

    if (e.hasY)
        scrollAxis(1, -e.axes.y() * speed, source, source == CZPointerScrollEvent::Finger && e.axes.y() == 0.f);

    repaint();
    AKContainer::pointerScrollEvent(e);
}

void AKScroll::sceneChangedEvent(const AKSceneChangedEvent &e)
{
    if (e.oldScene)
        e.oldScene->removeEventFilter(this);
    if (e.newScene)
        e.newScene->installEventFilter(this);
}

void AKScroll::layoutEvent(const CZLayoutEvent &e)
{
    AKContainer::layoutEvent(e);

    if (!e.changes.has(CZLayoutChangeSize)) return;

    stopKinetic();
    calculateContentBounds();
    applyConstraints();

    // The new offset is laid out in the next frame. Never calculate() here: during the scene's
    // layout pass it'd lay out the parent as a root, resetting its position (e.g. a list below a
    // search field drawn over it after the window changes size).
    repaint();
}

bool AKScroll::eventFilter(const CZEvent &event, CZObject &t) noexcept
{
    if (event.type() != CZEvent::Type::KeyboardKey)
        return AKContainer::eventFilter(event, t);

    const auto &e { static_cast<const CZKeyboardKeyEvent&>(event) };

    if (e.isPressed)
    {
        constexpr SkScalar delta { 128.f };

        if (e.code == KEY_DOWN)
            setOffsetY(offsetY() - delta);
        else if (e.code == KEY_UP)
            setOffsetY(offsetY() + delta);
        else if (e.code == KEY_RIGHT)
            setOffsetX(offsetX() - delta);
        else if (e.code == KEY_LEFT)
            setOffsetX(offsetX() + delta);
    }

    return AKContainer::eventFilter(e, t);
}

void AKScroll::updateBarXPrivate(bool reveal) noexcept
{
    if (layout().calculatedWidth() >= m_contentBounds.width())
        m_hBar.setSizePercent(1.f, reveal);
    else
        m_hBar.setSizePercent(layout().calculatedWidth() / m_contentBounds.width(), reveal);

    SkScalar x { -(offsetX() - m_contentBounds.fLeft) };
    SkScalar realWidth { m_contentBounds.width() - layout().calculatedWidth() };

    if (realWidth <= 0.f)
        m_hBar.setPosPercent(0.f, reveal);
    else
    {
        x = x/realWidth;
        m_hBar.setPosPercent(x, reveal);

        if (x < 0.f)
            m_hBar.setSizePercent(m_hBar.sizePercent() + x * (1.f - m_hBar.sizePercent()), reveal);
        else if (x > 1.f)
            m_hBar.setSizePercent(m_hBar.sizePercent() + (1.f - x) * (1.f - m_hBar.sizePercent()), reveal);
    }

    notifyOffset();
}

void AKScroll::updateBarYPrivate(bool reveal) noexcept
{
    if (layout().calculatedHeight() >= m_contentBounds.height())
        m_vBar.setSizePercent(1.f, reveal);
    else
        m_vBar.setSizePercent(layout().calculatedHeight() / m_contentBounds.height(), reveal);

    SkScalar y { -(offsetY() - m_contentBounds.fTop) };
    SkScalar realHeight { m_contentBounds.height() - layout().calculatedHeight() };

    if (realHeight <= 0.f)
        m_vBar.setPosPercent(0.f, reveal);
    else
    {
        y = y/realHeight;
        m_vBar.setPosPercent(y, reveal);

        if (y < 0.f)
            m_vBar.setSizePercent(m_vBar.sizePercent() + y * (1.f - m_vBar.sizePercent()), reveal);
        else if (y > 1.f)
            m_vBar.setSizePercent(m_vBar.sizePercent() + (1.f - y) * (1.f - m_vBar.sizePercent()), reveal);
    }

    notifyOffset();
}

void AKScroll::notifyOffset() noexcept
{
    const SkPoint offset { offsetX(), offsetY() };

    if (offset == m_notifiedOffset)
        return;

    m_notifiedOffset = offset;
    onOffsetChanged.notify();
}

void AKScroll::moveXPrivate(SkScalar dx) noexcept
{
    m_slot.layout().setPosition(YGEdgeLeft, offsetX() + dx);
    updateBarXPrivate();
}

void AKScroll::moveYPrivate(SkScalar dy) noexcept
{
    m_slot.layout().setPosition(YGEdgeTop, offsetY() + dy);
    updateBarYPrivate();
}
