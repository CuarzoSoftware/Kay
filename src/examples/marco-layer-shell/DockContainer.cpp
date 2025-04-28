#include <DockContainer.h>
#include <Theme.h>
#include <AK/events/AKLayoutEvent.h>
#include <Marco/MApplication.h>

DockContainer::DockContainer(AKNode *parent) noexcept :
    AKThreeImagePatch(AKHorizontal, parent)
{
    //layout().setFlex(1.f);
    layout().setFlexDirection(YGFlexDirectionRow);
    layout().setPosition(YGEdgeBottom, -32.f);
    //layout().setHeightPercent(100.f);
    layout().setHeightAuto();
    layout().setGap(YGGutterAll, 8.f);
    layout().setPadding(YGEdgeTop, SkScalar(Theme::DockShadowRadius - Theme::DockShadowOffsetY) + 8);
    layout().setPadding(YGEdgeBottom, SkScalar(Theme::DockShadowRadius + Theme::DockShadowOffsetY) + 8);
    layout().setPadding(YGEdgeLeft, SkScalar(Theme::DockShadowRadius) + 16);
    layout().setPadding(YGEdgeRight, SkScalar(Theme::DockShadowRadius) + 16);
    layout().setJustifyContent(YGJustifyCenter);
    layout().setAlignItems(YGAlignCenter);
    updateImage();
}

void DockContainer::layoutEvent(const AKLayoutEvent &event)
{
    AKThreeImagePatch::layoutEvent(event);

    if (event.changes().check(AKLayoutEvent::Scale))
        updateImage();
}

void DockContainer::updateImage() noexcept
{
    Theme *theme { static_cast<Theme*>(app()->theme()) };
    setSideSrcRect(theme->DockHThreePatchSideSrcRect);
    setCenterSrcRect(theme->DockHThreePatchCenterSrcRect);
    setImageScale(scale());
    setImage(theme->dockHThreePatchImage(scale()));
    layout().setWidthAuto();
}
