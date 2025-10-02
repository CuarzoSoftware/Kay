#include <CZ/AK/AKLog.h>
#include <CZ/Marco/private/MToplevelPrivate.h>
#include <CZ/Marco/input/MPointer.h>
#include <CZ/Marco/MApplication.h>

#include <CZ/Events/CZWindowStateEvent.h>
#include <CZ/AK/Events/AKWindowCloseEvent.h>

using namespace CZ;

MToplevel::Imp::Imp(MToplevel &obj) noexcept : obj(obj), shadow(&obj)
{
    obj.MSurface::imp()->flags.add(MSurface::Imp::PendingNullCommit);
    xdgSurfaceListener.configure = xdg_surface_configure;
    xdgToplevelListener.configure = xdg_toplevel_configure;
    xdgToplevelListener.configure_bounds = xdg_toplevel_configure_bounds;
    xdgToplevelListener.close = xdg_toplevel_close;
    xdgToplevelListener.wm_capabilities = xdg_toplevel_wm_capabilities;
    xdgDecorationListener.configure = xdg_decoration_configure;
}

void MToplevel::Imp::applyPendingParent() noexcept
{
    if (!obj.MSurface::imp()->flags.has(MSurface::Imp::PendingParent))
        return;

    obj.MSurface::imp()->flags.remove(MSurface::Imp::PendingParent);
    xdg_toplevel_set_parent(xdgToplevel, parentToplevel ? parentToplevel->imp()->xdgToplevel : nullptr);
}

void MToplevel::Imp::applyPendingChildren() noexcept
{
    if (!obj.MSurface::imp()->flags.has(MSurface::Imp::PendingChildren))
        return;

    obj.MSurface::imp()->flags.remove(MSurface::Imp::PendingChildren);

    for (const auto &child : childToplevels)
        xdg_toplevel_set_parent(child->imp()->xdgToplevel, xdgToplevel);
}

void MToplevel::Imp::setShadowMargins(const SkIRect &margins) noexcept
{
    if (margins == shadowMargins)
        return;

    // This is used to ensure the cursor position keeps relative to the central node
    // otherwise stuff like clicking a button won't work properly
    if (app()->pointer().focus() == &obj)
    {
        const SkIPoint offset { margins.topLeft() - shadowMargins.topLeft() };
        akPointer().setPos(akPointer().pos() + SkPoint::Make(offset.x(), offset.y()));
    }

    shadowMargins = margins;
    obj.update(true);
}

void MToplevel::Imp::xdg_surface_configure(void *data, xdg_surface */*xdgSurface*/, UInt32 serial)
{
    auto &role { *static_cast<MToplevel*>(data) };

    if (!role.MSurface::imp()->flags.has(MSurface::Imp::UserMapped))
    {
        xdg_surface_ack_configure(role.imp()->xdgSurface, serial);
        return;
    }

    role.MSurface::imp()->flags.add(MSurface::Imp::PendingConfigureAck);
    role.imp()->configureSerial = serial;

    bool notifyStates { role.MSurface::imp()->flags.has(MSurface::Imp::PendingFirstConfigure) };
    bool notifySuggestedSize { notifyStates };
    bool notifyBounds { false };
    bool notifyWMCaps { false };
    bool notifyDecoration { false };

    role.MSurface::imp()->flags.remove(MSurface::Imp::PendingFirstConfigure);

    const CZBitset<AKWindowState> stateChanges {
        (role.imp()->currentStates.get() ^ role.imp()->pendingStates.get()) & AKAllWindowStates };

    if (stateChanges.get() != 0)
    {
        role.imp()->currentStates = role.imp()->pendingStates;
        notifyStates = true;
    }

    if (role.imp()->pendingSuggestedSize != role.imp()->currentSuggestedSize)
    {
        role.imp()->currentSuggestedSize = role.imp()->pendingSuggestedSize;
        notifySuggestedSize = true;
    }

    if (role.imp()->pendingSuggestedBounds != role.imp()->currentSuggestedBounds)
    {
        notifyBounds = true;
        role.imp()->currentSuggestedBounds = role.imp()->pendingSuggestedBounds;
    }

    if (role.imp()->pendingWMCaps.get() != role.imp()->currentWMCaps.get())
    {
        role.imp()->currentWMCaps = role.imp()->pendingWMCaps;
        notifyWMCaps = true;
    }

    if (role.imp()->pendingDecorationMode != role.imp()->currentDecorationMode)
    {
        role.imp()->currentDecorationMode = role.imp()->pendingDecorationMode;
        notifyDecoration = true;
    }

    CZWeak<MToplevel> ref { &role };

    if (ref && notifyDecoration)
        role.decorationModeChanged();

    if (ref && notifyWMCaps)
        role.wmCapabilitiesChanged();

    if (notifySuggestedSize)
        role.suggestedSizeChanged();

    if (ref && notifyBounds)
        role.suggestedBoundsChanged();

    if (ref && notifyStates)
    {
        CZCore::Get()->sendEvent(CZWindowStateEvent(role.states(), stateChanges), role.scene());
        role.update(true);
    }

    if (ref && !role.MSurface::imp()->flags.has(MSurface::Imp::Mapped))
    {
        xdg_toplevel_set_title(role.imp()->xdgToplevel, role.title().c_str());
        xdg_toplevel_set_app_id(role.imp()->xdgToplevel, app()->appId().c_str());
        role.MSurface::imp()->setMapped(true);
        role.imp()->applyPendingParent();
        role.update(true);
    }
}

void MToplevel::Imp::xdg_toplevel_configure(void *data, xdg_toplevel *, Int32 width, Int32 height, wl_array *states)
{
    auto &role { *static_cast<MToplevel*>(data) };

    if (!role.MSurface::imp()->flags.has(MSurface::Imp::UserMapped))
        return;

    role.imp()->pendingStates.set(0);
    const UInt32 *stateVals = (UInt32*)states->data;
    for (UInt32 i = 0; i < states->size/sizeof(*stateVals); i++)
        role.imp()->pendingStates.add(1 << stateVals[i]);

    role.imp()->pendingStates.set(role.imp()->pendingStates.get() & AKAllWindowStates);

    // Just in case the compositor is insane
    role.imp()->pendingSuggestedSize.fWidth = width < 0 ? 0 : width;
    role.imp()->pendingSuggestedSize.fHeight = height < 0 ? 0 : height;
}

void MToplevel::Imp::xdg_toplevel_close(void *data, xdg_toplevel *)
{
    auto &role { *static_cast<MToplevel*>(data) };
    CZWeak<MToplevel> ref { &role };
    const bool accepted = app()->sendEvent(AKWindowCloseEvent(), role);

    if (ref && accepted)
        role.setMapped(false);
}

void MToplevel::Imp::xdg_toplevel_configure_bounds(void *data, xdg_toplevel */*xdgToplevel*/, Int32 width, Int32 height)
{
    auto &role { *static_cast<MToplevel*>(data) };
    role.imp()->pendingSuggestedBounds = { width, height };
}

void MToplevel::Imp::xdg_toplevel_wm_capabilities(void *data, xdg_toplevel */*xdgToplevel*/, wl_array *capabilities)
{
    auto &role { *static_cast<MToplevel*>(data) };
    role.imp()->pendingWMCaps.set(0);

    const Int32 *caps { static_cast<const Int32*>(capabilities->data) };

    for (size_t i = 0; i < capabilities->size/sizeof(*caps); i++)
        role.imp()->pendingWMCaps.add(1 << caps[i]);
}

void MToplevel::Imp::xdg_decoration_configure(void *data, zxdg_toplevel_decoration_v1 *, UInt32 mode)
{
    auto &role { *static_cast<MToplevel*>(data) };
    role.imp()->pendingDecorationMode = (DecorationMode)mode;
}

void MToplevel::Imp::handleRootPointerButtonEvent(const CZPointerButtonEvent &event) noexcept
{
    if (!obj.visible() || event.button() != BTN_LEFT || event.state() != CZPointerButtonEvent::Pressed)
        return;

    const SkPoint &pointerPos { pointer().eventHistory().move.pos() };
    const Int32 resizeMargins { MTheme::CSDResizeOutset };
    const Int32 moveTopMargin { MTheme::CSDMoveOutset };
    UInt32 resizeEdges { 0 };

    if (obj.globalRect().x() - resizeMargins <= pointerPos.x() && obj.globalRect().x() + 1 >= pointerPos.x())
        resizeEdges |= XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
    else if (obj.globalRect().right() - 1 <= pointerPos.x() && obj.globalRect().right() + resizeMargins >= pointerPos.x())
        resizeEdges |= XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;

    if (obj.globalRect().y() - resizeMargins <= pointerPos.y() && obj.globalRect().y() + 1 >= pointerPos.y())
        resizeEdges |= XDG_TOPLEVEL_RESIZE_EDGE_TOP;
    else if (obj.globalRect().bottom() - 1 <= pointerPos.y() && obj.globalRect().bottom() + resizeMargins >= pointerPos.y())
        resizeEdges |= XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;

    if (resizeEdges)
        xdg_toplevel_resize(xdgToplevel, app()->wayland().seat, event.serial(), resizeEdges);
    else if (
        obj.scene().pointerFocus() &&
        obj.scene().pointerFocus()->userCaps.has(UCWindowMove) &&
        obj.globalRect().x() <= pointerPos.x() &&
        obj.globalRect().right() >= pointerPos.x() &&
        obj.globalRect().y() <= pointerPos.y() &&
        obj.globalRect().y() + moveTopMargin >= pointerPos.y())
    {
        xdg_toplevel_move(xdgToplevel, app()->wayland().seat, event.serial());
    }
}

void MToplevel::Imp::handleRootPointerMoveEvent(const CZPointerMoveEvent &event) noexcept
{
    if (!obj.visible() || obj.fullscreen())
    {
        obj.rootNode()->setCursor(CZCursorShape::Default);
        return;
    }

    const SkPoint &pointerPos { event.pos() };
    const Int32 resizeMargins { MTheme::CSDResizeOutset };
    UInt32 resizeEdges { 0 };

    if (obj.globalRect().x() - resizeMargins <= pointerPos.x() && obj.globalRect().x() + 1 >= pointerPos.x())
        resizeEdges |= XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
    else if (obj.globalRect().right() - 1 <= pointerPos.x() && obj.globalRect().right() + resizeMargins >= pointerPos.x())
        resizeEdges |= XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;

    if (obj.globalRect().y() - resizeMargins <= pointerPos.y() && obj.globalRect().y() + 1 >= pointerPos.y())
        resizeEdges |= XDG_TOPLEVEL_RESIZE_EDGE_TOP;
    else if (obj.globalRect().bottom() - 1 <= pointerPos.y() && obj.globalRect().bottom() + resizeMargins >= pointerPos.y())
        resizeEdges |= XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;

    if (resizeEdges == XDG_TOPLEVEL_RESIZE_EDGE_LEFT || resizeEdges == XDG_TOPLEVEL_RESIZE_EDGE_RIGHT)
        obj.rootNode()->setCursor(CZCursorShape::EWResize);
    else if (resizeEdges == XDG_TOPLEVEL_RESIZE_EDGE_TOP || resizeEdges == XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM)
        obj.rootNode()->setCursor(CZCursorShape::NSResize);
    else if (resizeEdges == (XDG_TOPLEVEL_RESIZE_EDGE_TOP | XDG_TOPLEVEL_RESIZE_EDGE_LEFT) ||
             resizeEdges == (XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM | XDG_TOPLEVEL_RESIZE_EDGE_RIGHT))
        obj.rootNode()->setCursor(CZCursorShape::NWSEResize);
    else if (resizeEdges == (XDG_TOPLEVEL_RESIZE_EDGE_TOP | XDG_TOPLEVEL_RESIZE_EDGE_RIGHT) ||
             resizeEdges == (XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM | XDG_TOPLEVEL_RESIZE_EDGE_LEFT))
        obj.rootNode()->setCursor(CZCursorShape::NESWResize);
    else
        obj.rootNode()->setCursor(CZCursorShape::Default);
}

void MToplevel::Imp::unmap() noexcept
{
    using SF = MSurface::Imp::Flags;
    obj.MSurface::imp()->flags.add(SF::PendingNullCommit);
    obj.MSurface::imp()->flags.remove(SF::PendingConfigureAck | SF::PendingFirstConfigure);
    wl_surface_attach(obj.wlSurface(), nullptr, 0, 0);
    wl_surface_commit(obj.wlSurface());
    pendingStates.set(0);
    pendingSuggestedSize.setEmpty();
    configureSerial = 0;
    obj.MSurface::imp()->setMapped(false);
}
