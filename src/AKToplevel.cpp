#include <iostream>
#include <private/AKToplevel.h>
#include <private/AKApplication.h>
#include <private/AKSurface.h>

using namespace AK;

AK::AKToplevel::AKToplevel(const AKSize &size) noexcept : AKSurface(size), m_imp(std::make_unique<Private>())
{
    static const xdg_surface_listener xdgSurfaceListener
    {
        .configure = [](void *data, xdg_surface *xdgSurface, UInt32 serial)
        {
            xdg_surface_ack_configure(xdgSurface, serial);
        }
    };

    imp()->xdgSurface = xdg_wm_base_get_xdg_surface(app()->imp()->xdgWmBase, AKSurface::imp()->wlSurface);
    xdg_surface_add_listener(imp()->xdgSurface, &xdgSurfaceListener, this);

    static const xdg_toplevel_listener xdgToplevelListener
    {
        .configure = [](void *data, xdg_toplevel *xdgToplevel, Int32 w, Int32 h, wl_array *states)
        {
            std::cout << "Configured\n";
        },
        .close = [](void *data, xdg_toplevel *xdgToplevel)
        {

        },
        .configure_bounds = NULL,
        .wm_capabilities = NULL,
    };

    imp()->xdgToplevel = xdg_surface_get_toplevel(imp()->xdgSurface);
    xdg_toplevel_add_listener(imp()->xdgToplevel, &xdgToplevelListener, this);
    wl_surface_commit(AKSurface::imp()->wlSurface);
    wl_display_roundtrip(app()->imp()->wlDisplay);
    //eglSwapBuffers(app()->imp()->eglDisplay, AKSurface::imp()->eglSurface);
    std::cout << "Toplevel created\n";

}

AKToplevel::~AKToplevel() noexcept
{

}
