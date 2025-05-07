#ifndef MSURFACEPRIVATE_H
#define MSURFACEPRIVATE_H

#include <Marco/protocols/lvr-background-blur-client.h>
#include <Marco/protocols/lvr-invisible-region-client.h>
#include <Marco/roles/MSurface.h>
#include <Marco/nodes/MRootSurfaceNode.h>

class AK::MSurface::Imp
{
public:

    enum Flags
    {
        PendingUpdate             = 1 << 0,

        // Commit even if pending callback
        ForceUpdate               = 1 << 1,

        // The surface is actually mapped
        Mapped                    = 1 << 2,

        // The user wants to map the surface
        UserMapped                = 1 << 3,

        // Shared between some roles
        PendingNullCommit         = 1 << 4,
        PendingFirstConfigure     = 1 << 5,
        PendingConfigureAck       = 1 << 6,
        PendingChildren           = 1 << 7,
        PendingParent             = 1 << 8,
        BuiltinDecorations        = 1 << 9,
        HasBufferAttached         = 1 << 10,
        Last                      = 1 << 11,
    };

    enum TmpFlags
    {
        ScaleChanged              = 1 << 0,
        ScreensChanged            = 1 << 1,
        PreferredScaleChanged     = 1 << 2,
    };

    Imp(MSurface &obj) noexcept;
    MSurface &obj;

    // Cleared after onUpdate
    AKBitset<TmpFlags> tmpFlags;

    // Persistent
    AKBitset<Flags> flags { BuiltinDecorations };

    // Kay
    AKScene scene;
    AKWeak<AKSceneTarget> target;
    MRootSurfaceNode root;

    // Current wl_surface scale factor
    Int32 scale { 1 };

    // If not set by the compositor the max wl_output scale is used
    Int32 preferredBufferScale { -1 };

    // Intersected screens
    std::set<MScreen*> screens;

    SkISize size { 0, 0 };
    SkISize bufferSize { 0, 0 };
    SkISize viewportSize { -1, -1 };

    // Vibrancy
    AKVibrancyState currentVibrancyState { AKVibrancyState::Disabled };
    AKVibrancyState pendingVibrancyState { AKVibrancyState::Disabled };
    AKVibrancyStyle currentVibrancyStyle { AKVibrancyStyle::Light };
    AKVibrancyStyle pendingVibrancyStyle { AKVibrancyStyle::Light };

    UInt32 callbackSendMs { 0 };
    wl_callback *wlCallback { nullptr };
    wl_surface *wlSurface { nullptr };
    wp_viewport *wlViewport { nullptr };
    lvr_background_blur *backgroundBlur { nullptr };
    lvr_invisible_region *invisibleRegion { nullptr };

    wl_egl_window *eglWindow { nullptr };
    EGLSurface eglSurface { EGL_NO_SURFACE };
    sk_sp<SkSurface> skSurface;

    std::list<MSubsurface*> subSurfaces;

    Role role;
    size_t appLink;

    // Creates a wl_surface and wp_viewporter
    // Prev surface and viewporter are destroyed
    void createSurface() noexcept;
    bool createCallback() noexcept;
    void setMapped(bool mapped) noexcept;

    /**
     * @brief Resize the wl_egl_window
     *
     * @return true if resized, false if the size is the same.
     */
    bool resizeBuffer(const SkISize &size) noexcept;

    static void wl_surface_enter(void *data, wl_surface *surface, wl_output *output);
    static void wl_surface_leave(void *data,wl_surface *surface, wl_output *output);
    static void wl_surface_preferred_buffer_scale(void *data, wl_surface *surface, Int32 factor);
    static void wl_surface_preferred_buffer_transform(void *data, wl_surface *surface, UInt32 transform);
    static void wl_callback_done(void *data, wl_callback *callback, UInt32 ms);

    static void background_blur_state(void *data, lvr_background_blur *backgroundBlur, UInt32 state);
    static void background_blur_color_hint(void *data, lvr_background_blur *backgroundBlur, UInt32 style);
    static void background_blur_configure(void *data, lvr_background_blur *backgroundBlur, UInt32 serial);
};

#endif // MSURFACEPRIVATE_H
