#ifndef AKSCROLL_H
#define AKSCROLL_H

#include <AK/nodes/AKContainer.h>
#include <AK/AKAnimation.h>
#include <AK/nodes/AKScrollBar.h>

class AK::AKScroll : public AKContainer
{
public:
    AKScroll(AKNode *parent = nullptr) noexcept;

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
    void updateViewport() noexcept;
    void updateBars() noexcept;
    void applyConstraints() noexcept;
    void pointerScrollEvent(const AKPointerScrollEvent &e) override;
    void sceneChangedEvent(const AKSceneChangedEvent &e) override;
    void layoutEvent(const AKLayoutEvent &e) override;
    bool eventFilter(const AKEvent &e, AKObject &target) override;
    AKContainer m_slot;
    AKAnimation m_kineticYAnim;
    AKAnimation m_kineticXAnim;
    SkPoint m_vel { 0.f, 0.f };
    SkIRect m_contentBounds { 0, 0, 0, 0};
    bool m_fingersDownX { false };
    bool m_fingersDownY { false };
    AKScrollBar m_hBar { AKEdgeBottom, this };
    AKScrollBar m_vBar { AKEdgeRight, this };
};

#endif // AKSCROLL_H
