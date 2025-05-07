#ifndef MAPPLICATION_H
#define MAPPLICATION_H

#include <Marco/MProxy.h>
#include <Marco/MScreen.h>
#include <Marco/input/MPointer.h>
#include <Marco/input/MKeyboard.h>
#include <Marco/protocols/xdg-shell-client.h>
#include <Marco/protocols/xdg-decoration-unstable-v1-client.h>
#include <Marco/protocols/wlr-layer-shell-unstable-v1-client.h>
#include <Marco/protocols/viewporter-client.h>
#include <Marco/protocols/lvr-background-blur-client.h>
#include <Marco/protocols/lvr-svg-path-client.h>
#include <Marco/protocols/lvr-invisible-region-client.h>
#include <AK/AKApplication.h>
#include <AK/AKWeak.h>
#include <AK/events/AKPointerEnterEvent.h>
#include <AK/events/AKPointerMoveEvent.h>
#include <AK/events/AKPointerLeaveEvent.h>
#include <AK/events/AKPointerButtonEvent.h>
#include <wayland-client.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <sys/eventfd.h>
#include <sys/poll.h>
#include <fcntl.h>

class AK::MApplication : public AKApplication
{
public:
    struct Wayland
    {
        wl_display *display { nullptr };
        MProxy<wl_registry> registry;
        MProxy<wl_shm> shm;
        MProxy<wl_compositor> compositor;
        MProxy<wl_subcompositor> subCompositor;
        MProxy<xdg_wm_base> xdgWmBase;
        MProxy<zxdg_decoration_manager_v1> xdgDecorationManager;
        MProxy<wl_seat> seat;
        MProxy<wl_pointer> pointer;
        MProxy<wl_keyboard> keyboard;
        MProxy<wp_viewporter> viewporter;
        MProxy<zwlr_layer_shell_v1> layerShell;
        MProxy<lvr_background_blur_manager> backgroundBlurManager;
        MProxy<lvr_svg_path_manager> svgPathManager;
        MProxy<lvr_invisible_region_manager> invisibleRegionManager;
    };

    struct Graphics
    {
        EGLDisplay eglDisplay;
        EGLConfig eglConfig;
        EGLContext eglContext;
        PFNEGLSWAPBUFFERSWITHDAMAGEKHRPROC eglSwapBuffersWithDamageKHR;
    };

    enum MaskingCapabilities : UInt32
    {
        NoMaskCap       = 0U,
        RoundRectCap    = 1U,
        SVGPathCap      = 2U
    };

    MApplication() noexcept;

    static MTheme *theme() noexcept
    {
        return (MTheme*)AK::theme();
    }

    const std::string &appId() const noexcept
    {
        return m_appId;
    }

    void setAppId(const std::string &appId)
    {
        if (m_appId == appId)
            return;

        m_appId = appId;
        onAppIdChanged.notify();
    }

    Wayland &wayland() noexcept
    {
        return wl;
    }

    Graphics &graphics() noexcept
    {
        return gl;
    }

    const std::vector<MSurface*> &surfaces() const noexcept
    {
        return m_surfaces;
    }

    const std::vector<MScreen*> &screens() const noexcept
    {
        return m_screens;
    }


    void update() noexcept;
    MPointer &pointer() noexcept { return (MPointer&)AKApplication::pointer(); }
    MKeyboard &keyboard() noexcept { return(MKeyboard&)AKApplication::keyboard(); }

    AKBitset<MaskingCapabilities> maskingCapabilities() const noexcept { return m_maskingcaps; }

    AKSignal<MScreen&> onScreenPlugged;
    AKSignal<MScreen&> onScreenUnplugged;
    AKSignal<> onAppIdChanged;
private:
    friend class MSurface;
    friend class MScreen;
    static void wl_registry_global(void *data, wl_registry *registry, UInt32 name, const char *interface, UInt32 version);
    static void wl_registry_global_remove(void *data, wl_registry *registry, UInt32 name);
    static void wl_output_geometry(void *data, wl_output *output, Int32 x, Int32 y, Int32 physicalWidth, Int32 physicalHeight, Int32 subpixel, const char *make, const char *model, Int32 transform);
    static void wl_output_mode(void *data, wl_output *output, UInt32 flags, Int32 width, Int32 height, Int32 refresh);
    static void wl_output_done(void *data, wl_output *output);
    static void wl_output_scale(void *data, wl_output *output, Int32 factor);
    static void wl_output_name(void *data, wl_output *output, const char *name);
    static void wl_output_description(void *data, wl_output *output, const char *description);
    static void wl_seat_capabilities(void *data, wl_seat *seat, UInt32 capabilities);
    static void wl_seat_name(void *data, wl_seat *seat, const char *name);
    static void wl_pointer_enter(void *data, wl_pointer *pointer, UInt32 serial, wl_surface *surface, wl_fixed_t x, wl_fixed_t y);
    static void wl_pointer_leave(void *data, wl_pointer *pointer, UInt32 serial, wl_surface *surface);
    static void wl_pointer_motion(void *data,wl_pointer *pointer, UInt32 time, wl_fixed_t x, wl_fixed_t y);
    static void wl_pointer_button(void *data, wl_pointer *pointer, UInt32 serial, UInt32 time, UInt32 button, UInt32 state);
    static void wl_pointer_axis(void *data, wl_pointer *pointer, UInt32 time, UInt32 axis, wl_fixed_t value);
    static void wl_pointer_frame(void *data, wl_pointer *pointer);
    static void wl_pointer_axis_source(void *data, wl_pointer *pointer, UInt32 axis_source);
    static void wl_pointer_axis_stop(void *data, wl_pointer *pointer, UInt32 time, UInt32 axis);
    static void wl_pointer_axis_discrete(void *data, wl_pointer *pointer, UInt32 axis, Int32 discrete);
    static void wl_pointer_axis_value120(void *data, wl_pointer *pointer, UInt32 axis, Int32 value120);
    static void wl_pointer_axis_relative_direction(void *data, wl_pointer *pointer, UInt32 axis, UInt32 direction);
    static void wl_keyboard_keymap(void *data, wl_keyboard *keyboard, UInt32 format, Int32 fd, UInt32 size);
    static void wl_keyboard_enter(void *data, wl_keyboard *keyboard, UInt32 serial, wl_surface *surface, wl_array *keys);
    static void wl_keyboard_leave(void *data, wl_keyboard *keyboard, UInt32 serial, wl_surface *surface);
    static void wl_keyboard_key(void *data, wl_keyboard *keyboard, UInt32 serial, UInt32 time, UInt32 key, UInt32 state);
    static void wl_keyboard_modifiers(void *data, wl_keyboard *keyboard, UInt32 serial, UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group);
    static void wl_keyboard_repeat_info(void *data, wl_keyboard *keyboard, Int32 rate, Int32 delay);
    static void xdg_wm_base_ping(void *data, xdg_wm_base *xdgWmBase, UInt32 serial);
    static void lvr_background_blur_manager_masking_capabilities(void *data, lvr_background_blur_manager *blurManager, UInt32 caps);
    void initWayland() noexcept;
    void initGraphics() noexcept;
    void updateSurfaces();
    void updateSurface(MSurface *surface);
    bool m_running { false };

    Wayland wl;
    Graphics gl;
    std::string m_appId;

    std::vector<MSurface*> m_surfaces;
    std::vector<MScreen*> m_screens, m_pendingScreens;
    std::shared_ptr<AKBooleanEventSource> m_marcoSource;
    AKEventSource *m_waylandEventSource { nullptr };
    AKBitset<MaskingCapabilities> m_maskingcaps;
};

#endif // MAPPLICATION_H
