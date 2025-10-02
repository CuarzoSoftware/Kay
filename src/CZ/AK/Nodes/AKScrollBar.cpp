#include <CZ/Marco/MApplication.h>
#include <CZ/Events/CZPointerButtonEvent.h>
#include <CZ/Events/CZLayoutEvent.h>
#include <CZ/AK/Nodes/AKScrollBar.h>
#include <CZ/AK/Nodes/AKScroll.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKScene.h>

using namespace CZ;

AKScrollBar::AKScrollBar(AKScroll *scroll, CZEdge edge, AKNode *parent) noexcept :
    AKThreeImagePatch(edge == CZEdgeLeft || edge == CZEdgeRight ? CZOrientation::V : CZOrientation::H, parent),
    m_scroll(scroll)
{
    opaqueRegion.setRect(AK_IRECT_INF);
    setOpacity(0.f);
    setVisible(false);
    m_handle.enableReplaceImageColor(true);
    m_handle.setColor(SkColorSetARGB(128, 0, 0, 0));
    setKeepSidesAspectRatio(false);
    m_handle.setKeepSidesAspectRatio(false);
    layout().setPositionType(YGPositionTypeAbsolute);
    setEdge(edge);

    m_fadeOutAnim.setDuration(300);
    m_fadeOutAnim.setOnUpdateCallback([this](CZAnimation *a){

        if (opacity() > 0.f)
            setOpacity(1.0 - a->value());
        m_handle.setOpacity(0.5 * (1.0 - a->value()));
    });

    m_fadeOutAnim.setOnFinishCallback([this](CZAnimation *){

        if (m_preventHide)
            return;

        setOpacity(0.f);
        m_handle.setOpacity(0.f);

        if (m_handle.orientation() == CZOrientation::H)
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

    m_fadeOutTimer.setCallback([this](CZTimer *){
        if (!isPointerOver() && !m_dragging)
            m_fadeOutAnim.start();
    });

    m_hoverAnim.setDuration(100);
    m_hoverAnim.setOnUpdateCallback([this](CZAnimation *a){
        setOpacity(a->value());

        if (m_handle.orientation() == CZOrientation::H)
            m_handle.layout().setMinHeight(AKTheme::ScrollBarHandleWidth +
                (AKTheme::ScrollBarHandleWidthHover - AKTheme::ScrollBarHandleWidth) * a->value());
        else
            m_handle.layout().setMinWidth(AKTheme::ScrollBarHandleWidth +
                (AKTheme::ScrollBarHandleWidthHover - AKTheme::ScrollBarHandleWidth) * a->value());
    });

    m_hoverAnim.setOnFinishCallback([this](CZAnimation *){
        setOpacity(1.f);
    });

    m_fadeOutTimer.start(1);
}

bool AKScrollBar::setEdge(CZEdge edge) noexcept
{
    if (edge == CZEdgeNone)
        edge = CZEdgeRight;

    if (m_edge == edge)
        return false;

    m_edge = edge;
    addChange(CHEdge);

    if (edge == CZEdgeLeft || edge == CZEdgeRight)
    {
        layout().setPadding(YGEdgeAll, 2.f);
        layout().setPadding(YGEdgeLeft, 3.f);
        layout().setFlexDirection(YGFlexDirectionColumn);
        setOrientation(CZOrientation::H);
        layout().setWidthAuto();
        layout().setHeightPercent(100.f);
        m_handle.setOrientation(CZOrientation::V);
        m_handle.layout().setMinWidth(AKTheme::ScrollBarHandleWidth);
        m_handle.layout().setMinHeight(AKTheme::ScrollBarHandleWidth * 2.f);
    }
    else
    {
        layout().setPadding(YGEdgeAll, 2.f);
        layout().setPadding(YGEdgeTop, 3.f);
        layout().setFlexDirection(YGFlexDirectionRow);
        setOrientation(CZOrientation::V);
        layout().setHeightAuto();
        layout().setWidthPercent(100.f);
        m_handle.setOrientation(CZOrientation::H);
        m_handle.layout().setMinHeight(AKTheme::ScrollBarHandleWidth);
        m_handle.layout().setMinWidth(AKTheme::ScrollBarHandleWidth * 2.f);
    }

    layout().setPosition(YGEdgeAll, YGUndefined);

    switch (edge)
    {
    case CZEdgeTop:
        layout().setPosition(YGEdgeTop, 0.f);
        break;
    case CZEdgeBottom:
        layout().setPosition(YGEdgeBottom, 0.f);
        break;
    case CZEdgeLeft:
        layout().setPosition(YGEdgeLeft, 0.f);
        break;
    case CZEdgeRight:
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

    SkIRect realRect { worldRect() };

    if (m_handle.orientation() == CZOrientation::H)
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

void AKScrollBar::pointerButtonEvent(const CZPointerButtonEvent &e)
{
    AKThreeImagePatch::pointerButtonEvent(e);

    if (e.button != BTN_LEFT)
        return;

    if (e.pressed)
    {
        m_dragging = true;

        if (scene())
            scene()->setPointerGrab(this);

        /*
        if (m_handle.orientation() == CZOrientation::H)
            updatePosByPointer(app()->pointer().pos().x());
        else
            updatePosByPointer(app()->pointer().pos().y());*/
    }
    else
    {
        m_dragging = false;

        if (scene() && scene()->pointerGrab() == this)
            scene()->setPointerGrab(nullptr);
    }
}

void AKScrollBar::pointerMoveEvent(const CZPointerMoveEvent &e)
{
    AKThreeImagePatch::pointerMoveEvent(e);

    if (!m_dragging)
        return;

    if (m_handle.orientation() == CZOrientation::H)
        updatePosByPointer(e.pos.x());
    else
        updatePosByPointer(e.pos.y());
}

void AKScrollBar::pointerEnterEvent(const CZPointerEnterEvent &)
{
    if (opacity() == 0.f && m_handle.opacity() != 0.f)
        m_hoverAnim.start();

    m_preventHide = true;
}

void AKScrollBar::pointerLeaveEvent(const CZPointerLeaveEvent &)
{
    m_preventHide = false;
    m_dragging = false;
    if (scene() && scene()->pointerGrab() == this)
        scene()->setPointerGrab(nullptr);
    m_fadeOutTimer.start(1000);
}

void AKScrollBar::layoutEvent(const CZLayoutEvent &e)
{
    AKThreeImagePatch::layoutEvent(e);

    if (e.changes.has(CZLayoutChangeScale | CZLayoutChangeSize))
        updateImages();
}

void AKScrollBar::updateImages() noexcept
{
    SkRect side, center;
    m_handle.setImage(theme()->roundLineThreePatchImage(
        m_handle.orientation(),
        m_handle.orientation() == CZOrientation::H
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
    if (m_handle.orientation() == CZOrientation::V)
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
