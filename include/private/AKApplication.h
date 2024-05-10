#ifndef AKAPPLICATIONP_H
#define AKAPPLICATIONP_H

#include <xdg-shell-client-protocol.h>
#include <AKApplication.h>
#include <wayland-client.h>
#include <EGL/egl.h>
#include <list>

#include "include/gpu/GrDirectContext.h"
#include "include/core/SkColorSpace.h"

class AK::AKApplication::Private
{
public:
    bool initialized { false };
    bool running { false };

    wl_display *wlDisplay { nullptr };
    wl_registry *wlRegistry { nullptr };
    wl_compositor *wlCompositor { nullptr };
    xdg_wm_base *xdgWmBase { nullptr };

    EGLDisplay eglDisplay { EGL_NO_DISPLAY };
    EGLContext eglContext { EGL_NO_CONTEXT };
    EGLConfig eglConfig { EGL_FALSE };

    GrDirectContext *skiaContext { nullptr };
    sk_sp<SkColorSpace> colorSpace;

    std::list<AKSurface*> surfaces;

    bool initWayland() noexcept;
    bool initEGL() noexcept;
    bool initSkia() noexcept;
};

#endif //AKAPPLICATIONP
