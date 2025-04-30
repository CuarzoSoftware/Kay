#ifndef AKSCROLLBAR_H
#define AKSCROLLBAR_H

#include <AK/nodes/AKContainer.h>
#include <AK/nodes/AKThreeImagePatch.h>
#include <AK/AKAnimation.h>
#include <AK/AKTimer.h>
#include <AK/AKEdge.h>

class AK::AKScrollBar : public AKThreeImagePatch
{
public:
    enum Changes
    {
        CHEdge = AKThreeImagePatch::CHLast,
        CHPosPercent,
        CHSizePercent,
        CHLast
    };

    AKScrollBar(AKScroll *scroll, AKEdge edge, AKNode *parent = nullptr) noexcept;

    bool setEdge(AKEdge edge) noexcept;
    AKEdge edge() const noexcept
    {
        return m_edge;
    }

    // [0,1] This does not affect AKScrol
    bool setPosPercent(SkScalar pos) noexcept;
    SkScalar posPercent() const noexcept { return m_posPercent; }

    // [0,1] This does not affect AKScrol
    bool setSizePercent(SkScalar size) noexcept;
    SkScalar sizePercent() const noexcept { return m_sizePercent; }

protected:
    using AKThreeImagePatch::setOrientation;
    using AKThreeImagePatch::setImage;
    using AKThreeImagePatch::setImageScale;
    void updatePosByPointer(SkScalar pointerPos) noexcept;
    void pointerButtonEvent(const AKPointerButtonEvent &e) override;
    void pointerMoveEvent(const AKPointerMoveEvent &e) override;
    void pointerEnterEvent(const AKPointerEnterEvent &e) override;
    void pointerLeaveEvent(const AKPointerLeaveEvent &e) override;
    void layoutEvent(const AKLayoutEvent &e) override;
    void updateImages() noexcept;
    void updateGeometry() noexcept;
    SkScalar m_posPercent { 0.f };
    SkScalar m_sizePercent { 100.f };
    AKContainer m_space { YGFlexDirectionRow, false, this };
    AKThreeImagePatch m_handle { AKHorizontal, this };
    AKEdge m_edge { AKEdgeNone };
    AKTimer m_fadeOutTimer;
    AKAnimation m_fadeOutAnim;
    AKAnimation m_hoverAnim;
    AKWeak<AKScroll> m_scroll;
    bool m_dragging { false };
    bool m_preventHide { false };
};

#endif // AKSCROLLBAR_H
