#ifndef MPOPUP_H
#define MPOPUP_H

#include <CZ/Marco/roles/MSurface.h>

class CZ::MPopup : public MSurface
{
public:
    enum class Anchor
    {
        None,
        Top,
        Bottom,
        Left,
        Right,
        TopLeft,
        BottomLeft,
        TopRight,
        BottomRight
    };

    enum class Gravity
    {
        None,
        Top,
        Bottom,
        Left,
        Right,
        TopLeft,
        BottomLeft,
        TopRight,
        BottomRight
    };

    enum ConstraintAdjustment
    {
        SlideX  = 1,
        SlideY  = 2,
        FlipX   = 4,
        FlipY   = 8,
        ResizeX = 16,
        ResizeY = 32,
        All     = SlideX | SlideY | FlipX | FlipY | ResizeX | ResizeY
    };

    MPopup() noexcept;
    CZ_DISABLE_COPY(MPopup)
    ~MPopup();

    // Applied only during map

    // setSize internal

    // Only Toplevel, Popup, LayerShell
    bool setParent(MSurface *parent) noexcept;
    MSurface *parent() const noexcept;
    const std::unordered_set<MPopup*> &childPopups() const noexcept;

    // If any comp is negative the full parent geo is used
    void setAnchorRect(const SkIRect &rect) noexcept;
    const SkIRect &anchorRect() const noexcept;

    // Default is top-left
    void setAnchor(Anchor anchor) noexcept;
    Anchor anchor() const noexcept;

    // Default is bottom-right
    void setGravity(Gravity gravity) noexcept;
    Gravity gravity() const noexcept;

    // Default All
    void setConstraintAdjustment(CZBitset<ConstraintAdjustment> adjustment) noexcept;
    CZBitset<ConstraintAdjustment> constraintAdjustment() const noexcept;

    // Default (0,0)
    void setOffset(const SkIPoint &offset) noexcept;
    void setOffset(Int32 x, Int32 y) noexcept { setOffset({x,y}); }
    const SkIPoint &offset() const noexcept;

    // Reset after unmap
    void setGrab(const AKInputEvent *event) noexcept;
    AKInputEvent *grab() const noexcept;

    const SkIRect &assignedRect() const noexcept;

    CZSignal<> onAssignedRectChanged;
    CZSignal<> onDecorationMarginsChanged;
    CZSignal<const AKWindowCloseEvent&> onClose;

    class Imp;
    Imp *imp() const noexcept;
protected:
    virtual void closeEvent(const AKWindowCloseEvent &event);
    virtual void decorationMarginsChanged();
    virtual void assignedRectChanged();
    bool event(const CZEvent &e) override;
    void onUpdate() noexcept override;

private:
    std::unique_ptr<Imp> m_imp;
    void render() noexcept;
};

#endif // MPOPUP_H
