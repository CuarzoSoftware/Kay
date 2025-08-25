#include <CZ/AK/Events/AKWindowCloseEvent.h>
#include <CZ/Marco/private/MLayerSurfacePrivate.h>
#include <CZ/Marco/private/MSurfacePrivate.h>

using namespace CZ;

void MLayerSurface::Imp::configure(void *data, zwlr_layer_surface_v1 *, UInt32 serial, UInt32 width, UInt32 height)
{
    auto &role { *static_cast<MLayerSurface*>(data) };
    role.MSurface::imp()->flags.add(MSurface::Imp::PendingConfigureAck);
    role.imp()->configureSerial = serial;

    bool notifyStates { role.MSurface::imp()->flags.has(MSurface::Imp::PendingFirstConfigure) };
    bool notifySuggestedSize { notifyStates };
    role.MSurface::imp()->flags.remove(MSurface::Imp::PendingFirstConfigure);

    const SkISize suggestedSize (width, height);

    if (role.suggestedSize() != suggestedSize )
    {
        role.imp()->suggestedSize = suggestedSize;
        notifySuggestedSize = true;
    }

    if (notifySuggestedSize)
    {
        role.suggestedSizeChanged();
    }

    if (notifyStates)
    {
        role.MSurface::imp()->flags.add(MSurface::Imp::ForceUpdate);
        role.update();
    }

    if (!role.MSurface::imp()->flags.has(MSurface::Imp::Mapped))
    {
        role.MSurface::imp()->setMapped(true);
        role.update(true);
    }
}

void MLayerSurface::Imp::closed(void *data, zwlr_layer_surface_v1 *)
{
    auto &role { *static_cast<MLayerSurface*>(data) };

    AKWindowCloseEvent event;
    event.accept();
    role.onBeforeClose.notify(event);

    role.imp()->reset();

    if (event.isAccepted())
        role.setMapped(false);
    else
        role.update(true);
}

void MLayerSurface::Imp::reset() noexcept
{
    obj.MSurface::imp()->setMapped(false);

    if (layerSurface)
    {
        zwlr_layer_surface_v1_destroy(layerSurface);
        layerSurface = nullptr;
        wl_surface_attach(obj.wlSurface(), NULL, 0, 0);
        wl_surface_commit(obj.wlSurface());
        using SF = MSurface::Imp::Flags;
        obj.MSurface::imp()->flags.add(SF::PendingNullCommit);
        obj.MSurface::imp()->flags.remove(SF::PendingConfigureAck | SF::PendingFirstConfigure);
    }
}

void MLayerSurface::Imp::createRole() noexcept
{
    assert(layerSurface == nullptr);

    layerSurface = zwlr_layer_shell_v1_get_layer_surface(
        app()->wayland().layerShell,
        obj.wlSurface(),
        currentScreen.get() ? currentScreen->wlOutput() : NULL,
        pendingLayer,
        currentScope.c_str());

    zwlr_layer_surface_v1_add_listener(layerSurface, &layerSurfaceListener, &obj);

    /* ANCHOR */

    currentAnchor = pendingAnchor;
    zwlr_layer_surface_v1_set_anchor(layerSurface, currentAnchor.get());

    obj.scene().root()->layout().calculate();

    const Int32 w = obj.layout().calculatedWidth();
    const Int32 h = obj.layout().calculatedHeight();

    const Int32 finalW = requestAvailableWidth && pendingAnchor.checkAll(CZEdgeLeft | CZEdgeRight) ? 0 : (w != 0 ? w : 8);
    const Int32 finalH = requestAvailableHeight && pendingAnchor.checkAll(CZEdgeTop | CZEdgeBottom) ? 0 : (h != 0 ? h : 8);

    requestAvailableWidth = false;
    requestAvailableHeight = false;

    /* PREVENT SETTING AN INVALID SIZE */

    zwlr_layer_surface_v1_set_size(layerSurface, finalW, finalH);

    /* PREVENT SETTING AN INVALID EXCLUSIVE EDGE */

    currentExclusiveEdge = pendingExclusiveEdge;
    if (zwlr_layer_surface_v1_get_version(layerSurface) >= 5)
    {
        zwlr_layer_surface_v1_set_exclusive_edge(
            layerSurface,
            currentAnchor.has(currentExclusiveEdge) ? currentExclusiveEdge : CZEdgeNone);
    }

    /* EXCLUSIVE ZONE */

    currentExclusiveZone = pendingExclusiveZone;
    zwlr_layer_surface_v1_set_exclusive_zone(layerSurface, currentExclusiveZone);

    /* MARGIN */

    currentMargin = pendingMargin;
    zwlr_layer_surface_v1_set_margin(layerSurface,
        currentMargin.fTop,
        currentMargin.fRight,
        currentMargin.fBottom,
        currentMargin.fLeft);

    /* KEYBOARD INT */

    currentKeyboardInteractivity = pendingKeyboardInteractivity;
    zwlr_layer_surface_v1_set_keyboard_interactivity(layerSurface, currentKeyboardInteractivity);
}

MLayerSurface::Imp::Imp(MLayerSurface &obj) noexcept : obj(obj)
{
    obj.MSurface::imp()->flags.add(MSurface::Imp::PendingNullCommit);
    layerSurfaceListener.configure = &configure;
    layerSurfaceListener.closed = &closed;
}
