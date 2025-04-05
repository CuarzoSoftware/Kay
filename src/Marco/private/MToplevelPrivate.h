#ifndef MTOPLEVELPRIVATE_H
#define MTOPLEVELPRIVATE_H

#include <Marco/private/MSurfacePrivate.h>
#include <Marco/roles/MToplevel.h>
#include <Marco/protocols/xdg-shell-client.h>
#include <Marco/protocols/xdg-decoration-unstable-v1-client.h>
#include <Marco/nodes/MCSDShadow.h>
#include <AK/nodes/AKRenderableImage.h>

class AK::MToplevel::Imp
{
public:
    Imp(MToplevel &obj) noexcept;
    MToplevel &obj;

    std::string title;
    SkISize minSize, maxSize;

    // From the last xdg_surface_configure
    AKBitset<AKWindowState> currentStates;
    AKBitset<WMCapabilities> currentWMCaps;
    DecorationMode currentDecorationMode { ClientSide };
    SkISize currentSuggestedSize { 0, 0 };
    SkISize currentSuggestedBounds { 0, 0 };
    UInt32 configureSerial { 0 };

    // From xdg_toplevel_configure but not yet xdg_surface_configure(d)
    AKBitset<AKWindowState> pendingStates;
    AKBitset<WMCapabilities> pendingWMCaps;
    SkISize pendingSuggestedSize { 0, 0 };
    SkISize pendingSuggestedBounds { 0, 0 };
    DecorationMode pendingDecorationMode { ClientSide };

    // Built-in decorations
    AKRenderableImage borderRadius[4]; // Border radius masks
    SkIRect shadowMargins { 48, 30, 48, 66 }; // L, T, R, B shadow margins
    MCSDShadow shadow; // Shadow node

    // User decorations
    SkIRect userDecorationMargins { 0, 0, 0, 0 }; // L, T, R, B

    AKWeak<MToplevel> parentToplevel;
    std::unordered_set<MToplevel*> childToplevels;
    std::unordered_set<MPopup*> childPopups;

    void applyPendingParent() noexcept;
    void applyPendingChildren() noexcept;
    void setShadowMargins(const SkIRect &margins) noexcept;

    // Wayland
    xdg_surface *xdgSurface { nullptr };
    xdg_toplevel *xdgToplevel { nullptr };
    zxdg_toplevel_decoration_v1 *xdgDecoration { nullptr };
    static inline xdg_surface_listener xdgSurfaceListener;
    static inline xdg_toplevel_listener xdgToplevelListener;
    static inline zxdg_toplevel_decoration_v1_listener xdgDecorationListener;
    static void xdg_surface_configure(void *data, xdg_surface *xdgSurface, UInt32 serial);
    static void xdg_toplevel_configure(void *data, xdg_toplevel *xdgToplevel, Int32 width, Int32 height, wl_array *states);
    static void xdg_toplevel_close(void *data, xdg_toplevel *xdgToplevel);
    static void xdg_toplevel_configure_bounds(void *data, xdg_toplevel *xdgToplevel,Int32 width, Int32 height);
    static void xdg_toplevel_wm_capabilities(void *data, xdg_toplevel *xdgToplevel, wl_array *capabilities);

    static void xdg_decoration_configure(void *data, zxdg_toplevel_decoration_v1 *decoration, UInt32 mode);

    // Root node event filter
    void handleRootPointerButtonEvent(const AKPointerButtonEvent &event) noexcept;
    void handleRootPointerMoveEvent(const AKPointerMoveEvent &event) noexcept;
    void unmap() noexcept;
};

#endif // MTOPLEVELPRIVATE_H
