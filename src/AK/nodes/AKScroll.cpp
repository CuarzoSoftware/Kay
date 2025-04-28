#include <AK/events/AKLayoutEvent.h>
#include <AK/nodes/AKScroll.h>
#include <AK/events/AKKeyboardKeyEvent.h>
#include <AK/events/AKSceneChangedEvent.h>
#include <AK/events/AKPointerScrollEvent.h>
#include <AK/AKScene.h>
#include <AK/AKTheme.h>
#include <linux/input-event-codes.h>

using namespace AK;

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
            if (-m_slot.layout().position(YGEdgeTop).value - m_vel.fY < m_contentBounds.fTop)
            {
                m_vel.fY *= std::exp(-5.f * a->value());
                m_slot.layout().setPosition(YGEdgeTop,
                    m_slot.layout().position(YGEdgeTop).value * (1.f - a->value()) + m_contentBounds.fTop * a->value());
            }
            if (-m_slot.layout().position(YGEdgeTop).value - m_vel.fY + layout().calculatedHeight() > m_contentBounds.fBottom)
            {
                m_vel.fY *= std::exp(-5.f * a->value());
                m_slot.layout().setPosition(YGEdgeTop,
                    m_slot.layout().position(YGEdgeTop).value * (1.f - a->value()) + (-m_contentBounds.fBottom + layout().calculatedHeight()) * a->value());
            }
            else
            {
                m_vel.fY *= std::exp(-AKTheme::ScrollKineticFiction * a->value());
            }

            updateViewport();
            repaint();
        }
        else {
            a->stop();
        }
    });

    m_kineticYAnim.setOnFinishCallback([this](AKAnimation *){
        m_vel.fY = 0.f;
        updateViewport();
        repaint();
    });

    m_kineticXAnim.setOnUpdateCallback([this](AKAnimation *a){
        if (!m_fingersDownX)
        {
            if (-m_slot.layout().position(YGEdgeLeft).value < m_contentBounds.fLeft)
            {
                m_vel.fX *= std::exp(-5.f * a->value());
                m_slot.layout().setPosition(YGEdgeLeft,
                                            m_slot.layout().position(YGEdgeLeft).value * (1.f - a->value()) + m_contentBounds.fLeft * a->value());
            }
            if (-m_slot.layout().position(YGEdgeLeft).value + layout().calculatedWidth() > m_contentBounds.fRight)
            {
                m_vel.fX *= std::exp(-5.f * a->value());
                m_slot.layout().setPosition(YGEdgeLeft,
                                            m_slot.layout().position(YGEdgeLeft).value * (1.f - a->value()) + (-m_contentBounds.fRight + layout().calculatedWidth()) * a->value());
            }
            else
            {
                m_vel.fX *= std::exp(-AKTheme::ScrollKineticFiction * a->value());
            }

            updateViewport();
            repaint();
        }
        else {
            a->stop();
        }
    });

    m_kineticXAnim.setOnFinishCallback([this](AKAnimation *){
        m_vel.fX = 0.f;
        updateViewport();
        repaint();
    });
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

void AKScroll::updateViewport() noexcept
{
    m_slot.layout().setPosition(YGEdgeLeft, m_slot.layout().position(YGEdgeLeft).value + m_vel.x());
    m_slot.layout().setPosition(YGEdgeTop, m_slot.layout().position(YGEdgeTop).value + m_vel.y());
    updateBars();
}

void AKScroll::updateBars() noexcept
{
    if (layout().calculatedWidth() >= m_contentBounds.width())
        m_hBar.setSizePercent(1.f);
    else
        m_hBar.setSizePercent(layout().calculatedWidth() / m_contentBounds.width());

    if (layout().calculatedHeight() >= m_contentBounds.height())
        m_vBar.setSizePercent(1.f);
    else
        m_vBar.setSizePercent(layout().calculatedHeight() / m_contentBounds.height());

    SkScalar x { -(m_slot.layout().position(YGEdgeLeft).value - m_contentBounds.fLeft) };
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

    SkScalar y { -(m_slot.layout().position(YGEdgeTop).value - m_contentBounds.fTop) };
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

void AKScroll::applyConstraints() noexcept
{
    SkScalar x { m_slot.layout().position(YGEdgeLeft).value };
    SkScalar y { m_slot.layout().position(YGEdgeTop).value };

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
    updateBars();
}

void AKScroll::pointerScrollEvent(const AKPointerScrollEvent &e)
{
    calculateContentBounds();

    const bool isKinetic { e.source() == AKPointerScrollEvent::Continuous || e.source() == AKPointerScrollEvent::Finger };

    if (!isKinetic)
    {
        m_vel = e.axes() * 3.f;
        updateViewport();
        m_vel.set(0.f, 0.f);
        applyConstraints();
        repaint();
        return;
    }

    m_fingersDownX = e.axes().x() != 0.f;
    m_fingersDownY = e.axes().y() != 0.f;

    if (m_fingersDownX)
    {
        m_vel.fX = e.axes().x();

        SkScalar x { m_slot.layout().position(YGEdgeLeft).value + m_vel.x() };
        if (-x < m_contentBounds.fLeft && m_vel.fX > 0.f)
        {
            SkScalar diff { std::abs(m_contentBounds.fLeft + x) };
            if (diff > AKTheme::ScrollBounceOffsetLimit)
                diff = AKTheme::ScrollBounceOffsetLimit;
            const SkScalar percent { std::pow(1.f - diff/AKTheme::ScrollBounceOffsetLimit, 4.f) };
            m_vel.fX *= percent;
        }
        else if (-x + layout().calculatedWidth() > m_contentBounds.fRight && m_vel.fX < 0.f)
        {
            SkScalar diff { std::abs(m_contentBounds.fRight + x - layout().calculatedWidth()) };
            if (diff > AKTheme::ScrollBounceOffsetLimit)
                diff = AKTheme::ScrollBounceOffsetLimit;
            const SkScalar percent { std::pow(1.f - diff/AKTheme::ScrollBounceOffsetLimit, 4.f) };
            m_vel.fX *= percent;
        }
    }

    if (m_fingersDownY)
    {
        m_vel.fY = e.axes().y();
        SkScalar y { m_slot.layout().position(YGEdgeTop).value + m_vel.y() };
        if (-y < m_contentBounds.fTop && m_vel.fY > 0.f)
        {
            SkScalar diff { std::abs(m_contentBounds.fTop + y) };
            if (diff > AKTheme::ScrollBounceOffsetLimit)
                diff = AKTheme::ScrollBounceOffsetLimit;
            const SkScalar percent { std::pow(1.f - diff/AKTheme::ScrollBounceOffsetLimit, 4.f) };
            m_vel.fY *= percent;
        }
        else if (-y + layout().calculatedHeight() > m_contentBounds.fBottom && m_vel.fY < 0.f)
        {
            SkScalar diff { std::abs(m_contentBounds.fBottom + y - layout().calculatedHeight()) };
            if (diff > AKTheme::ScrollBounceOffsetLimit)
                diff = AKTheme::ScrollBounceOffsetLimit;
            const SkScalar percent { std::pow(1.f - diff/AKTheme::ScrollBounceOffsetLimit, 4.f) };
            m_vel.fY *= percent;
        }
    }

    if (!m_fingersDownY)
    {
        m_kineticYAnim.setDuration(std::min(std::max(AKTheme::ScrollKineticFictionTime * SkScalarAbs(m_vel.fY), 1000.f), 3000.f));
        m_kineticYAnim.start();
    }
    else
        m_kineticYAnim.stop();

    if (!m_fingersDownX)
    {
        m_kineticXAnim.setDuration(std::min(std::max(AKTheme::ScrollKineticFictionTime * SkScalarAbs(m_vel.fX), 1000.f), 3000.f));
        m_kineticXAnim.start();
    }
    else
        m_kineticXAnim.stop();

    updateViewport();
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

void AKScroll::layoutEvent(const AKLayoutEvent &e)
{
    AKContainer::layoutEvent(e);

    if (!e.changes().check(AKLayoutEvent::Size)) return;

    m_vel.set(0.f, 0.f);
    calculateContentBounds();
    applyConstraints();
    layout().calculate();
}

bool AKScroll::eventFilter(const AKEvent &ev, AKObject &t)
{
    if (ev.type() != AKEvent::KeyboardKey)
        return AKContainer::eventFilter(ev, t);

    const auto &e { static_cast<const AKKeyboardKeyEvent&>(ev) };

    if (e.keyCode() == KEY_DOWN)
        m_slot.layout().setPosition(YGEdgeTop, m_slot.layout().position(YGEdgeTop).value - 10);
    else if (e.keyCode() == KEY_UP)
        m_slot.layout().setPosition(YGEdgeTop, m_slot.layout().position(YGEdgeTop).value + 10);

    updateBars();

    return AKContainer::eventFilter(e, t);
}
