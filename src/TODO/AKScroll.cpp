#include <CZ/Events/CZLayoutEvent.h>
#include <CZ/AK/Nodes/AKScroll.h>
#include <CZ/Events/CZKeyboardKeyEvent.h>
#include <CZ/AK/Events/AKSceneChangedEvent.h>
#include <CZ/Events/CZPointerScrollEvent.h>
#include <CZ/AK/AKScene.h>
#include <CZ/AK/AKTheme.h>
#include <linux/input-event-codes.h>

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

    m_kineticYAnim.setOnUpdateCallback([this](AKAnimation *a){
        if (!m_fingersDownY)
        {
            if (-offsetY() - m_vel.fY < m_contentBounds.fTop)
            {
                m_vel.fY *= std::exp(-5.f * a->value());
                m_slot.layout().setPosition(YGEdgeTop,
                    offsetY() * (1.f - a->value()) + m_contentBounds.fTop * a->value());
            }
            if (-offsetY() - m_vel.fY + layout().calculatedHeight() > m_contentBounds.fBottom)
            {
                m_vel.fY *= std::exp(-5.f * a->value());
                m_slot.layout().setPosition(YGEdgeTop,
                    offsetY() * (1.f - a->value()) + (-m_contentBounds.fBottom + layout().calculatedHeight()) * a->value());
            }
            else
            {
                m_vel.fY *= std::exp(-AKTheme::ScrollKineticFriction * a->value());
            }

            moveYPrivate(m_vel.fY);
            repaint();
        }
        else {
            a->stop();
        }
    });

    m_kineticYAnim.setOnFinishCallback([this](AKAnimation *){
        m_vel.fY = 0.f;
        moveYPrivate(m_vel.fY);
        repaint();
    });

    m_kineticXAnim.setOnUpdateCallback([this](AKAnimation *a){
        if (!m_fingersDownX)
        {                         
            if (-offsetX() < m_contentBounds.fLeft)
            {
                m_vel.fX *= std::exp(-5.f * a->value());
                m_slot.layout().setPosition(YGEdgeLeft,
                                            offsetX() * (1.f - a->value()) + m_contentBounds.fLeft * a->value());
            }
            if (-offsetX() + layout().calculatedWidth() > m_contentBounds.fRight)
            {
                m_vel.fX *= std::exp(-5.f * a->value());
                m_slot.layout().setPosition(YGEdgeLeft,
                                            offsetX() * (1.f - a->value()) + (-m_contentBounds.fRight + layout().calculatedWidth()) * a->value());
            }
            else
            {
                m_vel.fX *= std::exp(-AKTheme::ScrollKineticFriction * a->value());
            }

            moveXPrivate(m_vel.fX);
            repaint();
        }
        else {
            a->stop();
        }
    });

    m_kineticXAnim.setOnFinishCallback([this](AKAnimation *){
        m_vel.fX = 0.f;
        moveXPrivate(m_vel.fX);
        repaint();
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
    m_slot.layout().setPosition(YGEdgeTop, y);
    m_slot.layout().setPosition(YGEdgeLeft, x);
    applyConstraints();
}

void AKScroll::setOffsetX(Int32 x) noexcept
{
    m_slot.layout().setPosition(YGEdgeLeft, x);
    applyConstraints();
}

void AKScroll::setOffsetY(Int32 y) noexcept
{
    m_slot.layout().setPosition(YGEdgeTop, y);
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

void AKScroll::calculateContentBounds() noexcept
{
    m_contentBounds.setEmpty();

    for (AKNode *child : m_slot.children())
        contentBounds(child, &m_contentBounds);

    m_contentBounds.fRight += m_slot.layout().calculatedPadding(YGEdgeRight);
    m_contentBounds.fBottom += m_slot.layout().calculatedPadding(YGEdgeBottom);

    if (m_contentBounds.fLeft > 0)
        m_contentBounds.fLeft = 0;

    if (m_contentBounds.fTop > 0)
        m_contentBounds.fTop = 0;
}

void AKScroll::applyConstraints() noexcept
{
    SkScalar x { offsetX() };
    SkScalar y { offsetY() };

    if (-x < m_contentBounds.fLeft)
        x = m_contentBounds.fLeft;
    else if (m_contentBounds.fRight + x < layout().calculatedWidth())
        x = -(m_contentBounds.fRight - layout().calculatedWidth());

    if (-y < m_contentBounds.fTop)
        y = m_contentBounds.fTop;
    else if (m_contentBounds.fBottom + y < layout().calculatedHeight())
        y = -(m_contentBounds.fBottom - layout().calculatedHeight());

    m_slot.layout().setPosition(YGEdgeTop, y);
    m_slot.layout().setPosition(YGEdgeLeft, x);
    updateBarXPrivate();
    updateBarYPrivate();
}

void AKScroll::pointerScrollEvent(const CZPointerScrollEvent &e)
{
    calculateContentBounds();

    SkScalar dx { 0.f }, dy { 0.f };

    // Invert axes if needed

    if (e.hasX())
        dx = - e.axes().x() * AKTheme::ScrollKineticSpeed;

    if (e.hasY())
        dy = - e.axes().y() * AKTheme::ScrollKineticSpeed;

    if (e.source() != CZPointerScrollEvent::Finger)
    {
        m_vel.set(0.f, 0.f);

        if (e.source() != CZPointerScrollEvent::Continuous)
        {
            dx *= 4.f;
            dy *= 4.f;
        }

        // TODO: Handle each source type case ?

        if (e.hasX())
            moveXPrivate(dx);

        if (e.hasY())
            moveYPrivate(dy);

        applyConstraints();
        repaint();
        return;
    }

    if (e.hasX())
    {
        // If == 0.f it means kinetic stop (fingers released)
        m_fingersDownX = e.axes().x() != 0.f;

        if (m_fingersDownX)
        {
            m_lastFingerTimeX = AKTime::ms();
            m_kineticXAnim.stop();

            m_vel.fX = dx;

            const SkScalar futureX { offsetX() + m_vel.x() };

            // The further the user tries to scroll beyond the edges the less delta (elastic)

            if (-futureX < m_contentBounds.fLeft && m_vel.fX > 0.f)
            {
                SkScalar diff { std::abs(m_contentBounds.fLeft + futureX) };

                if (diff > AKTheme::ScrollBounceOffsetLimit)
                    diff = AKTheme::ScrollBounceOffsetLimit;

                const SkScalar brake { std::pow(1.f - diff/AKTheme::ScrollBounceOffsetLimit, 4.f) };
                m_vel.fX *= brake;
            }
            else if (-futureX + layout().calculatedWidth() > m_contentBounds.fRight && m_vel.fX < 0.f)
            {
                SkScalar diff { std::abs(m_contentBounds.fRight + futureX - layout().calculatedWidth()) };

                if (diff > AKTheme::ScrollBounceOffsetLimit)
                    diff = AKTheme::ScrollBounceOffsetLimit;

                const SkScalar brake { std::pow(1.f - diff/AKTheme::ScrollBounceOffsetLimit, 4.f) };
                m_vel.fX *= brake;
            }

            moveXPrivate(m_vel.fX);
        }

        // Kinetic stop!
        else
        {
            const bool fingersStillForTooLong { AKTime::ms() - m_lastFingerTimeX > 64 };
            const bool outOfBounds { -offsetX() < m_contentBounds.fLeft || -offsetX() + layout().calculatedWidth() > m_contentBounds.fRight };

            if (outOfBounds || !fingersStillForTooLong)
            {
                m_kineticXAnim.setDuration(std::min(std::max(AKTheme::ScrollKineticInertia * SkScalarAbs(m_vel.fX), 1000.f), 3000.f));
                m_kineticXAnim.start();
            }
            else
                m_kineticXAnim.stop();
        }
    }

    if (e.hasY())
    {
        m_fingersDownY = e.axes().y() != 0.f;

        if (m_fingersDownY)
        {
            m_lastFingerTimeY = AKTime::ms();
            m_kineticYAnim.stop();

            m_vel.fY = dy;

            const SkScalar futureY { offsetY() + m_vel.y() };

            // The further the user tries to scroll beyond the edges the less delta (elastic)

            if (-futureY < m_contentBounds.fTop && m_vel.fY > 0.f)
            {
                SkScalar diff { std::abs(m_contentBounds.fTop + futureY) };
                if (diff > AKTheme::ScrollBounceOffsetLimit)
                    diff = AKTheme::ScrollBounceOffsetLimit;
                const SkScalar brake { std::pow(1.f - diff/AKTheme::ScrollBounceOffsetLimit, 4.f) };
                m_vel.fY *= brake;
            }
            else if (-futureY + layout().calculatedHeight() > m_contentBounds.fBottom && m_vel.fY < 0.f)
            {
                SkScalar diff { std::abs(m_contentBounds.fBottom + futureY - layout().calculatedHeight()) };
                if (diff > AKTheme::ScrollBounceOffsetLimit)
                    diff = AKTheme::ScrollBounceOffsetLimit;
                const SkScalar brake { std::pow(1.f - diff/AKTheme::ScrollBounceOffsetLimit, 4.f) };
                m_vel.fY *= brake;
            }

            moveYPrivate(m_vel.fY);
        }
        // Kinetic stop!
        else
        {
            const bool fingersStillForTooLong { AKTime::ms() - m_lastFingerTimeY > 64 };
            const bool outOfBounds { -offsetY() - m_vel.fY < m_contentBounds.fTop || -offsetY() - m_vel.fY + layout().calculatedHeight() > m_contentBounds.fBottom };

            if (outOfBounds || !fingersStillForTooLong)
            {
                m_kineticYAnim.setDuration(std::min(std::max(AKTheme::ScrollKineticInertia * SkScalarAbs(m_vel.fY), 1000.f), 3000.f));
                m_kineticYAnim.start();
            }
            else
                m_kineticYAnim.stop();
        }
    }

    repaint();
    AKContainer::pointerScrollEvent(e);
}

void AKScroll::sceneChangedEvent(const AKSceneChangedEvent &e)
{
    if (e.oldScene())
        e.oldScene()->removeEventFilter(this);
    if (e.newScene())
        e.newScene()->installEventFilter(this);
}

void AKScroll::layoutEvent(const CZLayoutEvent &e)
{
    AKContainer::layoutEvent(e);

    if (!e.changes().has(CZLayoutEvent::Size)) return;

    m_vel.set(0.f, 0.f);
    calculateContentBounds();
    applyConstraints();
    layout().calculate();
}

bool AKScroll::eventFilter(const CZEvent &ev, AKObject &t)
{
    if (ev.type() != CZEvent::KeyboardKey)
        return AKContainer::eventFilter(ev, t);

    const auto &e { static_cast<const CZKeyboardKeyEvent&>(ev) };

    if (e.state() == CZKeyboardKeyEvent::Pressed)
    {
        constexpr SkScalar delta { 128.f };

        if (e.keyCode() == KEY_DOWN)
            setOffsetY(offsetY() - delta);
        else if (e.keyCode() == KEY_UP)
            setOffsetY(offsetY() + delta);
        else if (e.keyCode() == KEY_RIGHT)
            setOffsetX(offsetX() - delta);
        else if (e.keyCode() == KEY_LEFT)
            setOffsetX(offsetX() + delta);
    }

    return AKContainer::eventFilter(e, t);
}

void AKScroll::updateBarXPrivate() noexcept
{
    if (layout().calculatedWidth() >= m_contentBounds.width())
        m_hBar.setSizePercent(1.f);
    else
        m_hBar.setSizePercent(layout().calculatedWidth() / m_contentBounds.width());

    SkScalar x { -(offsetX() - m_contentBounds.fLeft) };
    SkScalar realWidth { m_contentBounds.width() - layout().calculatedWidth() };

    if (realWidth <= 0.f)
        m_hBar.setPosPercent(0.f);
    else
    {
        x = x/realWidth;
        m_hBar.setPosPercent(x);

        if (x < 0.f)
            m_hBar.setSizePercent(m_hBar.sizePercent() + x * (1.f - m_hBar.sizePercent()));
        else if (x > 1.f)
            m_hBar.setSizePercent(m_hBar.sizePercent() + (1.f - x) * (1.f - m_hBar.sizePercent()));
    }
}

void AKScroll::updateBarYPrivate() noexcept
{
    if (layout().calculatedHeight() >= m_contentBounds.height())
        m_vBar.setSizePercent(1.f);
    else
        m_vBar.setSizePercent(layout().calculatedHeight() / m_contentBounds.height());

    SkScalar y { -(offsetY() - m_contentBounds.fTop) };
    SkScalar realHeight { m_contentBounds.height() - layout().calculatedHeight() };

    if (realHeight <= 0.f)
        m_vBar.setPosPercent(0.f);
    else
    {
        y = y/realHeight;
        m_vBar.setPosPercent(y);

        if (y < 0.f)
            m_vBar.setSizePercent(m_vBar.sizePercent() + y * (1.f - m_vBar.sizePercent()));
        else if (y > 1.f)
            m_vBar.setSizePercent(m_vBar.sizePercent() + (1.f - y) * (1.f - m_vBar.sizePercent()));
    }
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
