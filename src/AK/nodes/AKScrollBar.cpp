#include <AK/events/AKLayoutEvent.h>
#include <AK/nodes/AKScrollBar.h>
#include <AK/AKTheme.h>

using namespace AK;

AKScrollBar::AKScrollBar(AKEdge edge, AKNode *parent) noexcept :
    AKThreeImagePatch(edge == AKEdgeLeft || edge == AKEdgeRight ? AKVertical : AKHorizontal, parent)
{
    m_handle.enableCustomTextureColor(true);
    m_handle.setColorWithAlpha(SkColorSetARGB(128, 0, 0, 0));
    layout().setPositionType(YGPositionTypeAbsolute);
    layout().setPadding(YGEdgeAll, 2.f);
    setEdge(edge);

    m_fadeOutAnim.setDuration(300);
    m_fadeOutAnim.setOnUpdateCallback([this](AKAnimation *a){
        setOpacity(1.0 - a->value());
        m_handle.setOpacity(0.5 * (1.0 - a->value()));
    });

    m_fadeOutAnim.setOnFinishCallback([this](AKAnimation *){
        setOpacity(0.f);
        m_handle.setOpacity(0.f);
    });

    m_fadeOutTimer.setCallback([this](AKTimer *){
        m_fadeOutAnim.start();
    });

    m_fadeOutTimer.start(1000);
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
        layout().setFlexDirection(YGFlexDirectionColumn);
        setOrientation(AKVertical);
        layout().setWidthAuto();
        layout().setHeightPercent(100.f);
        m_handle.setOrientation(AKVertical);
        m_handle.layout().setMinWidth(AKTheme::ScrollBarHandleWidth);
        m_handle.layout().setMinHeight(AKTheme::ScrollBarHandleWidth * 4.f);
    }
    else
    {
        layout().setFlexDirection(YGFlexDirectionRow);
        setOrientation(AKHorizontal);
        layout().setHeightAuto();
        layout().setWidthPercent(100.f);
        m_handle.setOrientation(AKHorizontal);
        m_handle.layout().setMinHeight(AKTheme::ScrollBarHandleWidth);
        m_handle.layout().setMinWidth(AKTheme::ScrollBarHandleWidth * 4.f);
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

    if (sizePercent() != 1.f)
    {
        m_fadeOutTimer.start(1000);
        setOpacity(1.f);
        m_handle.setOpacity(0.5f);
    }
    else if (opacity() != 0.f)
    {
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

    if (size != 1.f)
    {
        m_fadeOutTimer.start(1000);
        setOpacity(1.f);
        m_handle.setOpacity(0.5f);
    }
    else if (opacity() != 0.f)
    {
        m_fadeOutTimer.stop();
        m_fadeOutAnim.start();
    }

    return true;
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
        AKTheme::ScrollBarHandleWidth,
        scale(),
        &side, &center));

    m_handle.setSideSrcRect(side);
    m_handle.setCenterSrcRect(center);
    m_handle.setImageScale(scale());
}

void AKScrollBar::updateGeometry() noexcept
{
    if (orientation() == AKVertical)
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
