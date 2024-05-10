#ifndef AKSURFACEP_H
#define AKSURFACEP_H

#include <private/AKFrame.h>
#include <AKSurface.h>
#include <AKPoint.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>

class AK::AKSurface::Private
{
public:
    Float32 scale { 1.f };
    wl_surface *wlSurface { nullptr };
    wl_egl_window *wlEGLWindow { nullptr};
    EGLSurface eglSurface { EGL_NO_SURFACE };
    std::unique_ptr<AKFrame> frame;
    std::list<AKSurface*>::iterator appLink;
};

#endif // AKSURFACEP_H
