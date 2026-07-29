#ifndef CZ_AKSCROLLBAR_H
#define CZ_AKSCROLLBAR_H

#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKThreePatch.h>
#include <CZ/Core/CZLinearAnimation.h>
#include <CZ/Core/CZTimer.h>
#include <CZ/Core/CZEdge.h>
#include <CZ/Core/CZAdaptive.h>

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

    /** @brief Adaptive fill of the rail (track) behind the handle. Re-resolved on scheme changes. */
    void setRailBackgroundColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor railBackgroundColor() const noexcept { return m_railBackgroundColor; }

    /** @brief Adaptive color of the rail's inner border line. Re-resolved on scheme changes. */
    void setRailBorderColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor railBorderColor() const noexcept { return m_railBorderColor; }

    /** @brief Adaptive color of the draggable handle (tinted via replaceImageColor). */
    void setHandleColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor handleColor() const noexcept { return m_handleColor; }

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
    // Light values match the previous hardcoded ones; dark variants mirror them for a dark scheme.
    CZAdaptiveColor m_railBackgroundColor { 0xFFFBFBFB, 0xFF2E2E2E };
    CZAdaptiveColor m_railBorderColor { 0x19000000, 0x19FFFFFF };
    CZAdaptiveColor m_handleColor { 0x80000000, 0x80FFFFFF };
    SkScalar m_posPercent { 0.f };
    SkScalar m_sizePercent { 100.f };
    AKContainer m_space { YGFlexDirectionRow, false, this };
    AKThreePatch m_handle { CZOrientation::H, this };
    CZEdge m_edge { CZEdgeNone };
    CZTimer m_fadeOutTimer;
    CZLinearAnimation m_fadeOutAnim;
    CZLinearAnimation m_hoverAnim;
    CZWeak<AKScroll> m_scroll;
    bool m_dragging { false };
    bool m_preventHide { false };
};

#endif // CZ_AKSCROLLBAR_H
