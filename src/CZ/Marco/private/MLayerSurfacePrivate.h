#ifndef MLAYERSHELLPRIVATE_H
#define MLAYERSHELLPRIVATE_H

#include <CZ/Marco/roles/MLayerSurface.h>
#include <CZ/Marco/protocols/wlr-layer-shell-unstable-v1-client.h>
#include <CZ/Marco/MScreen.h>
#include <CZ/Core/CZWeak.h>

class CZ::MLayerSurface::Imp
{
public:
    Imp(MLayerSurface &obj) noexcept;
    MLayerSurface &obj;

    Layer pendingLayer { Layer::Background };
    Layer currentLayer { Layer::Background };

    CZBitset<CZEdge> pendingAnchor { CZEdgeNone };
    CZBitset<CZEdge> currentAnchor { CZEdgeNone };

    Int32 pendingExclusiveZone { 0 };
    Int32 currentExclusiveZone { 0 };

    CZEdge pendingExclusiveEdge { CZEdgeNone };
    CZEdge currentExclusiveEdge { CZEdgeNone };

    SkIRect pendingMargin { 0, 0, 0, 0 };
    SkIRect currentMargin { 0, 0, 0, 0 };

    KeyboardInteractivity pendingKeyboardInteractivity { KeyboardInteractivity::None };
    KeyboardInteractivity currentKeyboardInteractivity { KeyboardInteractivity::None };

    bool requestAvailableWidth { false };
    bool requestAvailableHeight { false };

    std::string pendingScope, currentScope;
    CZWeak<MScreen> pendingScreen, currentScreen;

    std::unordered_set<MPopup*> childPopups;

    SkISize suggestedSize { 0, 0 };
    UInt32 configureSerial { 0 };

    zwlr_layer_surface_v1 *layerSurface { nullptr };
    static inline zwlr_layer_surface_v1_listener layerSurfaceListener;
    static void configure(void *data, zwlr_layer_surface_v1 *layerSurface, UInt32 serial, UInt32 width, UInt32 height);
    static void closed(void *data, zwlr_layer_surface_v1 *layerSurface);

    // Unmap surface, destroy the role, but keep props
    void reset() noexcept;
    void createRole() noexcept;
};

#endif // MLAYERSHELLPRIVATE_H
