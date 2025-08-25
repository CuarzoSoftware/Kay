#include "AK/AKLog.h"
#include <CZ/AK/Events/AKWindowCloseEvent.h>
#include <CZ/Marco/private/MPopupPrivate.h>
#include <CZ/Marco/private/MToplevelPrivate.h>
#include <CZ/Marco/private/MLayerSurfacePrivate.h>

using namespace CZ;

MPopup::Imp::Imp(MPopup &obj) noexcept : obj(obj)
{
    obj.MSurface::imp()->flags.add(MSurface::Imp::PendingNullCommit);
    xdgSurfaceListener.configure = &xdg_surface_configure;
    xdgPopupListener.configure = &xdg_popup_configure;
    xdgPopupListener.popup_done = &xdg_popup_done;
    xdgPopupListener.repositioned = &xdg_popup_repositioned;
}

void MPopup::Imp::xdg_surface_configure(void *data, xdg_surface */*xdgSurface*/, UInt32 serial)
{
    CZWeak<MPopup> role { static_cast<MPopup*>(data) };

    if (!role->MSurface::imp()->flags.has(MSurface::Imp::UserMapped))
    {
        xdg_surface_ack_configure(role->imp()->xdgSurface, serial);
        return;
    }

    role->MSurface::imp()->flags.add(MSurface::Imp::PendingConfigureAck);
    role->imp()->configureSerial = serial;

    const bool update { role->MSurface::imp()->flags.has(MSurface::Imp::PendingFirstConfigure) };
    const bool notifyAssignedRect { role->imp()->pendingAssignedRect != role->imp()->currentAssignedRect };
    role->MSurface::imp()->flags.remove(MSurface::Imp::PendingFirstConfigure);

    if (notifyAssignedRect)
    {
        role->imp()->currentAssignedRect = role->imp()->pendingAssignedRect;
        role->assignedRectChanged();
    }

    if (role && update)
        role->update(true);
}

void MPopup::Imp::xdg_popup_configure(void *data, xdg_popup */*xdgPopup*/, Int32 x, Int32 y, Int32 width, Int32 height)
{
    auto &popup { *static_cast<MPopup*>(data) };
    popup.imp()->pendingAssignedRect = SkIRect::MakeXYWH(x, y, width, height);
}

void MPopup::Imp::xdg_popup_done(void *data, xdg_popup */*xdgPopup*/)
{
    CZWeak<MPopup> popup { static_cast<MPopup*>(data) };
    app()->sendEvent(AKWindowCloseEvent(), *popup);

    if (popup)
    {
        popup->imp()->destroy();
        popup->setMapped(false);
    }
}

void MPopup::Imp::xdg_popup_repositioned(void */*data*/, xdg_popup */*xdgPopup*/, UInt32 /*token*/) {}

xdg_positioner *MPopup::Imp::createPositioner() noexcept
{
    assert(parent);

    xdg_positioner *positioner { xdg_wm_base_create_positioner(app()->wayland().xdgWmBase) };

    obj.layout().setPosition(YGEdgeLeft, 0.f);
    obj.layout().setPosition(YGEdgeTop, 0.f);
    obj.layout().setMargin(YGEdgeAll, 0.f);
    obj.scene().root()->layout().calculate();

    SkISize size {
        SkScalarFloorToInt(obj.layout().calculatedWidth()),
        SkScalarFloorToInt(obj.layout().calculatedHeight())
    };

    if (size.fWidth < 8) size.fWidth = 8;
    if (size.fHeight < 8) size.fHeight = 8;

    xdg_positioner_set_size(positioner, size.width(), size.height());

    // TODO: Use the user defined rect
    xdg_positioner_set_anchor_rect(positioner, 0, 0, parent->globalRect().width(), parent->globalRect().height());
    xdg_positioner_set_anchor(positioner, (UInt32)anchor);
    xdg_positioner_set_gravity(positioner, (UInt32)gravity);
    xdg_positioner_set_constraint_adjustment(positioner, constraintAdjustment.get());
    xdg_positioner_set_offset(positioner, offset.x(), offset.y());

    if (app()->wayland().xdgWmBase.version() >= 3)
    {
        parent->scene().root()->layout().calculate();
        xdg_positioner_set_parent_size(positioner, parent->layout().calculatedWidth(), parent->layout().calculatedHeight());

        if (parent->role() == Role::Toplevel)
            xdg_positioner_set_parent_configure(positioner, ((MToplevel*)parent.get())->imp()->configureSerial);
        else if (parent->role() == Role::Popup)
            xdg_positioner_set_parent_configure(positioner, ((MPopup*)parent.get())->imp()->configureSerial);
    }

    // set reactive
    // set parent size
    // set parent configure

    return positioner;
}

void MPopup::Imp::create() noexcept
{
    if (xdgPopup || !parent || !parent->mapped()) return;

    xdgSurface = xdg_wm_base_get_xdg_surface(app()->wayland().xdgWmBase, obj.wlSurface());
    xdg_surface_add_listener(xdgSurface, &xdgSurfaceListener, &obj);

    xdg_positioner *positioner { createPositioner() };

    switch (parent->role())
    {
    case Role::Popup:
        xdgPopup = xdg_surface_get_popup(xdgSurface, static_cast<MPopup*>(parent.get())->imp()->xdgSurface, positioner);
        break;
    case Role::Toplevel:
        xdgPopup = xdg_surface_get_popup(xdgSurface, static_cast<MToplevel*>(parent.get())->imp()->xdgSurface, positioner);
        break;
    case Role::LayerSurface:
        xdgPopup = xdg_surface_get_popup(xdgSurface, nullptr, positioner);
        zwlr_layer_surface_v1_get_popup(static_cast<MLayerSurface*>(parent.get())->imp()->layerSurface, xdgPopup);
        break;
    default:
        break;
    }

    if (grab)
    {
        xdg_popup_grab(xdgPopup, app()->wayland().seat, grab->serial());
        grab.reset();
    }

    xdg_popup_add_listener(xdgPopup, &xdgPopupListener, &obj);
    xdg_positioner_destroy(positioner);
    wl_surface_attach(obj.wlSurface(), nullptr, 0, 0);
    wl_surface_commit(obj.wlSurface());
}

void MPopup::Imp::destroy() noexcept
{
    for (auto *child : childPopups)
        child->imp()->destroy();

    obj.MSurface::imp()->flags.remove(MSurface::Imp::PendingConfigureAck | MSurface::Imp::PendingFirstConfigure);
    obj.MSurface::imp()->flags.add(MSurface::Imp::PendingNullCommit);

    if (xdgPopup)
    {
        wl_surface_attach(obj.wlSurface(), NULL, 0, 0);
        wl_surface_commit(obj.wlSurface());
        obj.MSurface::imp()->flags.remove(MSurface::Imp::HasBufferAttached);

        xdg_popup_destroy(xdgPopup);
        xdgPopup = nullptr;

        xdg_surface_destroy(xdgSurface);
        xdgSurface = nullptr;
    }

    obj.MSurface::imp()->setMapped(false);
}

void MPopup::Imp::unsetParent() noexcept
{
    if (!parent)
        return;

    switch (parent->role())
    {
    case Role::Popup:
        static_cast<MPopup*>(parent.get())->imp()->childPopups.erase(&obj);
        break;
    case Role::Toplevel:
        static_cast<MToplevel*>(parent.get())->imp()->childPopups.erase(&obj);
        break;
    case Role::LayerSurface:
        static_cast<MLayerSurface*>(parent.get())->imp()->childPopups.erase(&obj);
        break;
    default:
        break;
    }

    parent.reset();
}

void MPopup::Imp::setParent(MSurface *surface) noexcept
{
    unsetParent();
    if (!surface) return;
    parent.reset(surface);

    switch (parent->role())
    {
    case Role::Popup:
        static_cast<MPopup*>(parent.get())->imp()->childPopups.insert(&obj);
        return;
    case Role::Toplevel:
        static_cast<MToplevel*>(parent.get())->imp()->childPopups.insert(&obj);
        return;
    case Role::LayerSurface:
        static_cast<MLayerSurface*>(parent.get())->imp()->childPopups.insert(&obj);
        return;
    default:
        break;
    }

    parent.reset();
}
