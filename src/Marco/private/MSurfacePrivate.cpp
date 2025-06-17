#include <Marco/private/MSurfacePrivate.h>
#include <Marco/private/MPopupPrivate.h>
#include <Marco/private/MToplevelPrivate.h>
#include <Marco/private/MLayerSurfacePrivate.h>
#include <Marco/MApplication.h>

#include <AK/events/AKVibrancyEvent.h>
#include <AK/AKGLContext.h>

#include <CZ/skia/gpu/ganesh/gl/GrGLBackendSurface.h>
#include <CZ/skia/gpu/ganesh/GrBackendSurface.h>
#include <CZ/skia/gpu/ganesh/GrRecordingContext.h>
#include <CZ/skia/gpu/ganesh/GrDirectContext.h>
#include <CZ/skia/gpu/ganesh/gl/GrGLTypes.h>
#include <CZ/skia/gpu/ganesh/SkSurfaceGanesh.h>
#include <CZ/skia/core/SkColorSpace.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

using namespace AK;

static wl_surface_listener wlSurfaceListener;
static wl_callback_listener wlCallbackListener;
static lvr_background_blur_listener backgroundBlurListener;

MSurface::Imp::Imp(MSurface &obj) noexcept : obj(obj), root(obj)
{
    wlSurfaceListener.enter = wl_surface_enter;
    wlSurfaceListener.leave = wl_surface_leave;
    wlSurfaceListener.preferred_buffer_scale = wl_surface_preferred_buffer_scale;
    wlSurfaceListener.preferred_buffer_transform = wl_surface_preferred_buffer_transform;
    wlCallbackListener.done = wl_callback_done;
    backgroundBlurListener.state = background_blur_state;
    backgroundBlurListener.color_hint = background_blur_color_hint;
    backgroundBlurListener.configure = background_blur_configure;
}

void MSurface::Imp::wl_surface_enter(void *data, wl_surface */*surface*/, wl_output *output)
{
    MSurface &surface { *static_cast<MSurface*>(data) };
    MScreen *screen { static_cast<MScreen*>(wl_output_get_user_data(output)) };

    if (surface.imp()->screens.contains(screen))
        return;

    surface.imp()->screens.insert(screen);
    surface.imp()->tmpFlags.add(Imp::ScreensChanged);
    surface.onEnteredScreen.notify(*screen);

    if (surface.imp()->preferredBufferScale == -1)
        surface.update();

    // TODO: Listen for screen prop changes
}

void MSurface::Imp::wl_surface_leave(void *data, wl_surface */*surface*/, wl_output *output)
{
    MSurface &surface { *static_cast<MSurface*>(data) };
    MScreen *screen { static_cast<MScreen*>(wl_output_get_user_data(output)) };
    auto it = surface.imp()->screens.find(screen);

    if (it == surface.imp()->screens.end())
        return;

    surface.imp()->screens.erase(it);
    surface.imp()->tmpFlags.add(Imp::ScreensChanged);
    surface.onLeftScreen.notify(*screen);

    if (surface.imp()->preferredBufferScale == -1)
        surface.update();
}

void MSurface::Imp::wl_surface_preferred_buffer_scale(void *data, wl_surface */*surface*/, Int32 factor)
{
    MSurface &surface { *static_cast<MSurface*>(data) };
    if (factor <= 0)
        factor = 1;

    if (surface.imp()->preferredBufferScale == factor)
        return;

    surface.imp()->preferredBufferScale = factor;
    surface.imp()->tmpFlags.add(Imp::PreferredScaleChanged);
    surface.update();
}

void MSurface::Imp::wl_surface_preferred_buffer_transform(void */*data*/, wl_surface */*surface*/, UInt32 /*transform*/)
{
    // TODO
}

void MSurface::Imp::wl_callback_done(void *data, wl_callback *callback, UInt32 ms)
{
    MSurface &surface { *static_cast<MSurface*>(data) };
    wl_callback_destroy(callback);
    surface.imp()->wlCallback = nullptr;
    surface.onCallbackDone.notify(ms);

    if (surface.imp()->flags.check(PendingUpdate) || surface.target()->isDirty())
        surface.update();
}

void MSurface::Imp::background_blur_state(void *data, lvr_background_blur *, UInt32 state)
{
    MSurface &surface { *static_cast<MSurface*>(data) };
    surface.imp()->pendingVibrancyState = (AKVibrancyState)state;
}

void MSurface::Imp::background_blur_color_hint(void *data, lvr_background_blur *, UInt32 style)
{
    MSurface &surface { *static_cast<MSurface*>(data) };
    surface.imp()->pendingVibrancyStyle = (AKVibrancyStyle)style;
}

void MSurface::Imp::background_blur_configure(void *data, lvr_background_blur *backgroundBlur, UInt32 serial)
{
    MSurface &surface { *static_cast<MSurface*>(data) };
    lvr_background_blur_ack_configure(backgroundBlur, serial);

    if (surface.imp()->pendingVibrancyState != surface.imp()->currentVibrancyState ||
        surface.imp()->pendingVibrancyStyle != surface.imp()->currentVibrancyStyle)
    {
        surface.update(true);
        surface.imp()->currentVibrancyState = surface.imp()->pendingVibrancyState;
        surface.imp()->currentVibrancyStyle = surface.imp()->pendingVibrancyStyle;
        app()->sendEvent(AKVibrancyEvent(
            surface.imp()->currentVibrancyState,
            surface.imp()->currentVibrancyStyle),
            surface);
    }
}

void MSurface::Imp::createSurface() noexcept
{
    if (wlCallback)
    {
        wl_callback_destroy(wlCallback);
        wlCallback = nullptr;
    }

    if (wlViewport)
    {
        wp_viewport_destroy(wlViewport);
        wlViewport = nullptr;
    }

    if (eglSurface)
    {
        eglDestroySurface(app()->gl.eglDisplay, eglSurface);
        eglSurface = EGL_NO_SURFACE;
    }

    if (eglWindow)
    {
        wl_egl_window_destroy(eglWindow);
        eglWindow = nullptr;
    }

    if (backgroundBlur)
    {
        lvr_background_blur_destroy(backgroundBlur);
        backgroundBlur = nullptr;
    }

    if (invisibleRegion)
    {
        lvr_invisible_region_destroy(invisibleRegion);
        invisibleRegion = nullptr;
    }

    if (wlSurface)
    {
        wl_surface_destroy(wlSurface);
        wlSurface = nullptr;
    }

    wlSurface = wl_compositor_create_surface(app()->wayland().compositor);
    wl_surface_add_listener(wlSurface, &wlSurfaceListener, &obj);
    wlViewport = wp_viewporter_get_viewport(app()->wayland().viewporter, wlSurface);

    if (app()->wayland().invisibleRegionManager)
        invisibleRegion = lvr_invisible_region_manager_get_invisible_region(app()->wayland().invisibleRegionManager, wlSurface);

    if (app()->wayland().backgroundBlurManager)
    {
        backgroundBlur = lvr_background_blur_manager_get_background_blur(app()->wayland().backgroundBlurManager, wlSurface);
        lvr_background_blur_add_listener(backgroundBlur, &backgroundBlurListener, &obj);
    }
}

bool MSurface::Imp::createCallback() noexcept
{
    if (wlCallback)
        return false;

    callbackSendMs = AKTime::ms();
    wlCallback = wl_surface_frame(wlSurface);
    wl_callback_add_listener(wlCallback, &wlCallbackListener, &obj);
    return true;
}

void MSurface::Imp::setMapped(bool mapped) noexcept
{
    if (mapped == obj.mapped())
        return;

    if (!mapped && wlCallback)
    {
        wl_callback_destroy(wlCallback);
        wlCallback = nullptr;
    }

    std::unordered_set<MPopup*> *childPopups { nullptr };

    switch (role)
    {
    case Role::Popup:
        childPopups = &((MPopup*)&obj)->imp()->childPopups;
        break;
    case Role::Toplevel:
        childPopups = &((MToplevel*)&obj)->imp()->childPopups;
        break;
    case Role::LayerSurface:
        childPopups = &((MLayerSurface*)&obj)->imp()->childPopups;
        break;
    default:
        break;
    }

    if (childPopups)
        for (auto &child : *childPopups)
            child->update();

    flags.setFlag(Mapped, mapped);
    obj.onMappedChanged.notify();
}

bool MSurface::Imp::resizeBuffer(const SkISize &size) noexcept
{
    const SkISize bufferSize { size.width() * scale , size.height() * scale };

    if (bufferSize == this->bufferSize)
        return false;

    this->size = size;
    this->bufferSize = bufferSize;

    eglMakeCurrent(app()->graphics().eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, app()->graphics().eglContext);

    if (size.isEmpty())
    {
        skSurface.reset();

        if (eglSurface != EGL_NO_SURFACE)
        {
            eglDestroySurface(app()->gl.eglDisplay, eglSurface);
            eglSurface = EGL_NO_SURFACE;
        }

        if (eglWindow)
        {
            wl_egl_window_destroy(eglWindow);
            eglWindow = nullptr;
        }

        return true;
    }

    if (eglWindow)
        wl_egl_window_resize(eglWindow, bufferSize.width(), bufferSize.height(), 0, 0);
    else
    {
        eglWindow = wl_egl_window_create(wlSurface, bufferSize.width(), bufferSize.height());
        eglSurface = eglCreateWindowSurface(app()->gl.eglDisplay, app()->gl.eglConfig, (EGLNativeWindowType)eglWindow, NULL);
        assert("Failed to create EGLSurface for MSurface" && eglSurface != EGL_NO_SURFACE);
    }

    //AKLog::debug("Resized %d x %d", cl.bufferSize.width(), cl.bufferSize.height());

    static const SkSurfaceProps skSurfaceProps(0, kUnknown_SkPixelGeometry);

    static constexpr GrGLFramebufferInfo fbInfo
    {
        .fFBOID = 0,
        .fFormat = GL_RGBA8_OES
    };

    const GrBackendRenderTarget backendTarget = GrBackendRenderTargets::MakeGL(
        bufferSize.width(),
        bufferSize.height(),
        0, 0,
        fbInfo);

    skSurface = SkSurfaces::WrapBackendRenderTarget(
        akApp()->glContext()->skContext().get(),
        backendTarget,
        GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,
        SkColorType::kRGBA_8888_SkColorType,
        SkColorSpace::MakeSRGB(),
        &skSurfaceProps);

    return true;
}
