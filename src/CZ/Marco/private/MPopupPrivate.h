#ifndef MPOPUPPRIVATE_H
#define MPOPUPPRIVATE_H

#include <CZ/Marco/protocols/xdg-shell-client.h>
#include <CZ/Marco/roles/MPopup.h>

class CZ::MPopup::Imp
{
public:
    Imp(MPopup &obj) noexcept;
    MPopup &obj;
    SkIRect anchorRect { -1, -1, -1, -1 };
    SkIPoint offset { 0, 0 };
    Anchor anchor { Anchor::TopLeft };
    Gravity gravity { Gravity::BottomRight };
    CZBitset<ConstraintAdjustment> constraintAdjustment { ConstraintAdjustment::All };
    std::unique_ptr<AKInputEvent> grab;
    UInt32 configureSerial { 0 };

    SkIRect currentAssignedRect { 0, 0, 0, 0 };
    SkIRect pendingAssignedRect { 0, 0, 0, 0 };

    // Set by the user
    CZWeak<MSurface> parent;

    // When actually applied
    bool paramsChanged { true };
    std::unordered_set<MPopup*> childPopups;
    xdg_surface *xdgSurface { nullptr };
    xdg_popup *xdgPopup { nullptr };
    static inline xdg_surface_listener xdgSurfaceListener;
    static inline xdg_popup_listener xdgPopupListener;
    static void xdg_surface_configure(void *data, xdg_surface *xdgSurface, UInt32 serial);
    static void xdg_popup_configure(void *data, xdg_popup *xdgPopup, Int32 x, Int32 y, Int32 width, Int32 height);
    static void xdg_popup_done(void *data, xdg_popup *xdgPopup);
    static void xdg_popup_repositioned(void *data, xdg_popup *xdgPopup, UInt32 token);

    xdg_positioner *createPositioner() noexcept;
    void create() noexcept;
    void destroy() noexcept;
    void unsetParent() noexcept;
    void setParent(MSurface *surface) noexcept;
};


#endif // MPOPUPPRIVATE_H
