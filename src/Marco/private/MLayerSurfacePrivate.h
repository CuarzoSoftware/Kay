#ifndef MLAYERSHELLPRIVATE_H
#define MLAYERSHELLPRIVATE_H

#include <Marco/roles/MLayerSurface.h>
#include <Marco/protocols/wlr-layer-shell-unstable-v1-client.h>
#include <Marco/MScreen.h>
#include <AK/AKWeak.h>

class AK::MLayerSurface::Imp
{
public:
    Imp(MLayerSurface &obj) noexcept;
    MLayerSurface &obj;
    Layer layer;
    Int32 exclusiveZone { 0 };
    AKBitset<AKEdge> anchor { AKEdgeNone };
    AKEdge exclusiveEdge { AKEdgeNone };
    std::string scope;
    AKWeak<MScreen> screen;
    SkIRect margin { 0, 0, 0, 0 };
    KeyboardInteractivity keyboardInteractivity { KeyboardInteractivity::None };
    std::unordered_set<MPopup*> childPopups;

    SkISize suggestedSize { 0, 0 };
    UInt32 configureSerial { 0 };

    zwlr_layer_surface_v1 *layerSurface { nullptr };
    static inline zwlr_layer_surface_v1_listener layerSurfaceListener;
    static void configure(void *data, zwlr_layer_surface_v1 *layerSurface, UInt32 serial, UInt32 width, UInt32 height);
    static void closed(void *data, zwlr_layer_surface_v1 *layerSurface);
};

#endif // MLAYERSHELLPRIVATE_H
