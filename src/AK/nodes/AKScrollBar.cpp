#include <Marco/MApplication.h>
#include <AK/events/AKPointerButtonEvent.h>
#include <AK/events/AKLayoutEvent.h>
#include <AK/nodes/AKScrollBar.h>
#include <AK/nodes/AKScroll.h>
#include <AK/AKTheme.h>

using namespace AK;

AKScrollBar::AKScrollBar(AKScroll *scroll, AKEdge edge, AKNode *parent) noexcept :
    AKThreeImagePatch(edge == AKEdgeLeft || edge == AKEdgeRight ? AKVertical : AKHorizontal, parent),
    m_scroll(scroll)
{
    opaqueRegion.setRect(AK_IRECT_INF);
    setOpacity(0.f);
    setVisible(false);
    m_handle.enableCustomTextureColor(true);
    m_handle.setColorWithAlpha(SkColorSetARGB(128, 0, 0, 0));
    setKeepSidesAspectRatio(false);
    m_handle.setKeepSidesAspectRatio(false);
    layout().setPositionType(YGPositionTypeAbsolute);
    setEdge(edge);

    m_fadeOutAnim.setDuration(300);
    m_fadeOutAnim.setOnUpdateCallback([this](AKAnimation *a){

        if (opacity() > 0.f)
            setOpacity(1.0 - a->value());
        m_handle.setOpacity(0.5 * (1.0 - a->value()));
    });

    m_fadeOutAnim.setOnFinishCallback([this](AKAnimation *){

        if (m_preventHide)
            return;

        setOpacity(0.f);
        m_handle.setOpacity(0.f);

        if (m_handle.orientation() == AKHorizontal)
        {
            m_handle.layout().setMinHeight(AKTheme::ScrollBarHandleWidth);
            m_handle.layout().setMinWidth(AKTheme::ScrollBarHandleWidth * 2.f);
        }
        else
        {
            m_handle.layout().setMinWidth(AKTheme::ScrollBarHandleWidth);
            m_handle.layout().setMinHeight(AKTheme::ScrollBarHandleWidth * 2.f);
        }

        setVisible(false);
    });

    m_fadeOutTimer.setCallback([this](AKTimer *){
        if (!isPointerOver() && !m_dragging)
            m_fadeOutAnim.start();
    });

    m_hoverAnim.setDuration(100);
    m_hoverAnim.setOnUpdateCallback([this](AKAnimation *a){
        setOpacity(a->value());

        if (m_handle.orientation() == AKHorizontal)
            m_handle.layout().setMinHeight(AKTheme::ScrollBarHandleWidth +
                (AKTheme::ScrollBarHandleWidthHover - AKTheme::ScrollBarHandleWidth) * a->value());
        else
            m_handle.layout().setMinWidth(AKTheme::ScrollBarHandleWidth +
                (AKTheme::ScrollBarHandleWidthHover - AKTheme::ScrollBarHandleWidth) * a->value());
    });

    m_hoverAnim.setOnFinishCallback([this](AKAnimation *){
        setOpacity(1.f);
    });

    m_fadeOutTimer.start(1);
}

bool AKScrollBar::setEdge(AKEdge edge) noexcept
{
    if (edge == AKEdgeNone)
        edge = AKEdgeRight;

    if (m_edge == edge)
        return false;

    m_edge = edge;
    addChange(CHEdge);

    if (edge == AKEdgeLeft || edge == AKEdgeRight)
    {
        layout().setPadding(YGEdgeAll, 2.f);
        layout().setPadding(YGEdgeLeft, 3.f);
        layout().setFlexDirection(YGFlexDirectionColumn);
        setOrientation(AKHorizontal);
        layout().setWidthAuto();
        layout().setHeightPercent(100.f);
        m_handle.setOrientation(AKVertical);
        m_handle.layout().setMinWidth(AKTheme::ScrollBarHandleWidth);
        m_handle.layout().setMinHeight(AKTheme::ScrollBarHandleWidth * 2.f);
    }
    else
    {
        layout().setPadding(YGEdgeAll, 2.f);
        layout().setPadding(YGEdgeTop, 3.f);
        layout().setFlexDirection(YGFlexDirectionRow);
        setOrientation(AKVertical);
        layout().setHeightAuto();
        layout().setWidthPercent(100.f);
        m_handle.setOrientation(AKHorizontal);
        m_handle.layout().setMinHeight(AKTheme::ScrollBarHandleWidth);
        m_handle.layout().setMinWidth(AKTheme::ScrollBarHandleWidth * 2.f);
    }

    layout().setPosition(YGEdgeAll, YGUndefined);

    switch (edge)
    {
    case AKEdgeTop:
        layout().setPosition(YGEdgeTop, 0.f);
        break;
    case AKEdgeBottom:
        layout().setPosition(YGEdgeBottom, 0.f);
        break;
    case AKEdgeLeft:
        layout().setPosition(YGEdgeLeft, 0.f);
        break;
    case AKEdgeRight:
        layout().setPosition(YGEdgeRight, 0.f);
        break;
    default:
        break;
    }

    updateGeometry();
    updateImages();
    return true;
}

bool AKScrollBar::setPosPercent(SkScalar pos) noexcept
{
    if (pos < 0.f)
        pos = 0.f;
    else if (pos > 1.f)
        pos = 1.f;

    if (pos == m_posPercent)
        return false;

    m_posPercent = pos;
    addChange(CHPosPercent);
    updateGeometry();

    m_preventHide = true;
    m_fadeOutAnim.stop();
    m_preventHide = false;

    if (sizePercent() != 1.f)
    {
        setVisible(true);
        m_fadeOutTimer.start(1000);
        m_handle.setOpacity(0.5f);
    }
    else if (opacity() != 0.f)
    {
        setVisible(true);
        m_fadeOutTimer.stop();
        m_fadeOutAnim.start();
    }

    return true;
}

bool AKScrollBar::setSizePercent(SkScalar size) noexcept
{
    if (size < 0.f)
        size = 0.f;
    else if (size > 1.f)
        size = 1.f;

    if (size == m_sizePercent)
        return false;

    m_sizePercent = size;
    addChange(CHSizePercent);
    updateGeometry();

    m_preventHide = true;
    m_fadeOutAnim.stop();
    m_preventHide = false;

    if (size != 1.f)
    {
        setVisible(true);
        m_fadeOutTimer.start(1000);
        m_handle.setOpacity(0.5f);
    }
    else if (opacity() != 0.f)
    {
        setVisible(true);
        m_fadeOutTimer.stop();
        m_fadeOutAnim.start();
    }

    return true;
}

void AKScrollBar::updatePosByPointer(SkScalar pointerPos) noexcept
{
    if (!m_scroll)
        return;

    SkIRect realRect { globalRect() };

    if (m_handle.orientation() == AKHorizontal)
    {
        realRect.fLeft += layout().calculatedPadding(YGEdgeLeft);
        realRect.fRight -= layout().calculatedPadding(YGEdgeRight);

        if (pointerPos <= realRect.fLeft || realRect.width() == 0)
            m_scroll->setOffsetXPercent(0.f);
        else if (pointerPos >= realRect.fRight)
            m_scroll->setOffsetXPercent(100.f);
        else
        {
            // For some reason Yoga varies the handle size by aprox 1 while moving...
            // this hack keeps a consistent size and avoids wiggling
            Int32 handleSize = m_handle.layout().calculatedWidth();
            handleSize += handleSize % 2;
            m_scroll->setOffsetXPercent((pointerPos - SkScalar(realRect.fLeft) - SkScalar(handleSize) * 0.5f)/SkScalar(realRect.width()));
        }
    }
    else
    {
        realRect.fTop += layout().calculatedPadding(YGEdgeTop);
        realRect.fBottom -= layout().calculatedPadding(YGEdgeBottom);

        if (pointerPos <= realRect.fTop || realRect.height() == 0)
            m_scroll->setOffsetYPercent(0.f);
        else if (pointerPos >= realRect.fBottom)
            m_scroll->setOffsetYPercent(100.f);
        else
        {
            // For some reason Yoga varies the handle size by aprox 1 while moving...
            // this hack keeps a consistent size and avoids wiggling
            Int32 handleSize = m_handle.layout().calculatedHeight();
            handleSize += handleSize % 2;
            m_scroll->setOffsetYPercent((pointerPos - SkScalar(realRect.fTop) - SkScalar(handleSize) * 0.5f)/SkScalar(realRect.height()));
        }
    }
}

void AKScrollBar::pointerButtonEvent(const AKPointerButtonEvent &e)
{
    AKThreeImagePatch::pointerButtonEvent(e);

    if (e.button() != BTN_LEFT)
        return;

    if (e.state() == AKPointerButtonEvent::Pressed)
    {
        m_dragging = true;
        enablePointerGrab(true);

        if (m_handle.orientation() == AKHorizontal)
            updatePosByPointer(app()->pointer().pos().x());
        else
            updatePosByPointer(app()->pointer().pos().y());
    }
    else
    {
        m_dragging = false;
        enablePointerGrab(false);
    }
}

void AKScrollBar::pointerMoveEvent(const AKPointerMoveEvent &e)
{
    AKThreeImagePatch::pointerMoveEvent(e);

    if (!m_dragging)
        return;

    if (m_handle.orientation() == AKHorizontal)
        updatePosByPointer(e.pos().x());
    else
        updatePosByPointer(e.pos().y());
}

void AKScrollBar::pointerEnterEvent(const AKPointerEnterEvent &)
{
    if (opacity() == 0.f && m_handle.opacity() != 0.f)
        m_hoverAnim.start();

    m_preventHide = true;
}

void AKScrollBar::pointerLeaveEvent(const AKPointerLeaveEvent &)
{
    m_preventHide = false;
    m_dragging = false;
    enablePointerGrab(false);
    m_fadeOutTimer.start(1000);
}

void AKScrollBar::layoutEvent(const AKLayoutEvent &e)
{
    AKThreeImagePatch::layoutEvent(e);

    if (e.changes().check(AKLayoutEvent::Scale | AKLayoutEvent::Size))
        updateImages();
}

void AKScrollBar::updateImages() noexcept
{
    SkRect side, center;
    m_handle.setImage(theme()->roundLineThreePatchImage(
        m_handle.orientation(),
        m_handle.orientation() == AKHorizontal
            ?
            m_handle.layout().calculatedHeight()
            :
            m_handle.layout().calculatedWidth(),
        scale(),
        &side, &center));

    m_handle.setSideSrcRect(side);
    m_handle.setCenterSrcRect(center);
    m_handle.setImageScale(scale());

    setImage(theme()->scrollRailThreePatchImage(
        orientation(),
        scale(),
        &side, &center));

    setSideSrcRect(side);
    setCenterSrcRect(center);
    setImageScale(scale());
}

void AKScrollBar::updateGeometry() noexcept
{
    if (m_handle.orientation() == AKVertical)
    {
        m_space.layout().setWidth(0.f);
        m_space.layout().setHeightPercent(posPercent() * (100.f * (1.f - sizePercent())));
        m_handle.layout().setHeightPercent(sizePercent() * 100.f);
        m_handle.layout().setWidth(AKTheme::ScrollBarHandleWidth);
    }
    else
    {
        m_space.layout().setHeight(0.f);
        m_space.layout().setWidthPercent(posPercent() * (100.f * (1.f - sizePercent())));
        m_handle.layout().setWidthPercent(sizePercent() * 100.f);
        m_handle.layout().setHeight(AKTheme::ScrollBarHandleWidth);
    }
}
