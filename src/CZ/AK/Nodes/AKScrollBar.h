#ifndef CZ_AKSCROLLBAR_H
#define CZ_AKSCROLLBAR_H

#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKThreePatch.h>
#include <CZ/Core/CZAnimation.h>
#include <CZ/Core/CZTimer.h>
#include <CZ/Core/CZEdge.h>

class CZ::AKScrollBar : public AKThreePatch
{
public:
    enum Changes
    {
        CHEdge = AKThreePatch::CHLast,
        CHPosPercent,
        CHSizePercent,
        CHLast
    };

    AKScrollBar(AKScroll *scroll, CZEdge edge, AKNode *parent = nullptr) noexcept;

    bool setEdge(CZEdge edge) noexcept;
    CZEdge edge() const noexcept
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
    using AKThreePatch::setOrientation;
    using AKThreePatch::setImage;
    using AKThreePatch::setImageScale;
    void updatePosByPointer(SkScalar pointerPos) noexcept;
    void pointerButtonEvent(const CZPointerButtonEvent &e) override;
    void pointerMoveEvent(const CZPointerMoveEvent &e) override;
    void pointerEnterEvent(const CZPointerEnterEvent &e) override;
    void pointerLeaveEvent(const CZPointerLeaveEvent &e) override;
    void layoutEvent(const CZLayoutEvent &e) override;
    void updateImages() noexcept;
    void updateGeometry() noexcept;
    SkScalar m_posPercent { 0.f };
    SkScalar m_sizePercent { 100.f };
    AKContainer m_space { YGFlexDirectionRow, false, this };
    AKThreePatch m_handle { CZOrientation::H, this };
    CZEdge m_edge { CZEdgeNone };
    CZTimer m_fadeOutTimer;
    CZAnimation m_fadeOutAnim;
    CZAnimation m_hoverAnim;
    CZWeak<AKScroll> m_scroll;
    bool m_dragging { false };
    bool m_preventHide { false };
};

#endif // CZ_AKSCROLLBAR_H
