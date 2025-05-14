#ifndef AKSCROLL_H
#define AKSCROLL_H

#include <AK/nodes/AKContainer.h>
#include <AK/AKAnimation.h>
#include <AK/nodes/AKScrollBar.h>

class AK::AKScroll : public AKContainer
{
public:
    AKScroll(AKNode *parent = nullptr) noexcept;

    SkScalar offsetX() const noexcept;
    SkScalar offsetY() const noexcept;

    void setOffset(SkScalar x, SkScalar y) noexcept;
    void setOffsetX(Int32 x) noexcept;
    void setOffsetY(Int32 y) noexcept;

    void setOffsetXPercent(SkScalar x) noexcept;
    void setOffsetYPercent(SkScalar y) noexcept;

    AKScrollBar &horizontalBar() noexcept
    {
        return m_hBar;
    }

    AKScrollBar &verticalBar() noexcept
    {
        return m_vBar;
    }

protected:
    void calculateContentBounds() noexcept;
    void applyConstraints() noexcept;
    void pointerScrollEvent(const AKPointerScrollEvent &e) override;
    void sceneChangedEvent(const AKSceneChangedEvent &e) override;
    void layoutEvent(const AKLayoutEvent &e) override;
    bool eventFilter(const AKEvent &e, AKObject &target) override;

    void updateBarXPrivate() noexcept;
    void updateBarYPrivate() noexcept;
    void moveXPrivate(SkScalar dx) noexcept;
    void moveYPrivate(SkScalar dy) noexcept;

    AKContainer m_slot;
    AKAnimation m_kineticYAnim;
    AKAnimation m_kineticXAnim;
    SkPoint m_vel { 0.f, 0.f };
    SkIRect m_contentBounds { 0, 0, 0, 0};
    Int64 m_lastFingerTimeX;
    Int64 m_lastFingerTimeY;
    AKScrollBar m_hBar { this, AKEdgeBottom, this };
    AKScrollBar m_vBar { this, AKEdgeRight, this };
    bool m_fingersDownX { false };
    bool m_fingersDownY { false };
};

#endif // AKSCROLL_H
