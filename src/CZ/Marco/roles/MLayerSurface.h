#ifndef MLAYERSHELL_H
#define MLAYERSHELL_H

#include <CZ/Marco/roles/MSurface.h>
#include <CZ/Core/CZEdge.h>

/**
 * @brief WLR Layer Shell Role
 *
 * This role can be used to create many different kind of shell components such as docks, top bars, notifications, wallpapers, and more.
 *
 * For a complete understanding of its semantics, refer to the [protocol documentation](https://wayland.app/protocols/wlr-layer-shell-unstable-v1#zwlr_layer_surface_v1).
 *
 * @note Changing the screen or scope after the surface is mapped requires destroying and recreating the layer role, briefly unmapping the surface in the compositor.
 *       This also applies when changing the layer if the compositor supports a version lower than 2.
 */
class CZ::MLayerSurface : public MSurface
{
public:
    enum Layer
    {
        Background,
        Bottom,
        Top,
        Overlay
    };

    enum KeyboardInteractivity
    {
        None,
        Exclusive,
        OnDemand
    };

    MLayerSurface() noexcept;

    CZ_DISABLE_COPY(MLayerSurface)

    /**
     * @brief Destructor for `MLayerSurface`.
     */
    ~MLayerSurface() noexcept;

    // Notifies the available width later in suggestedSizeChanged
    // If the left and right anchors are not set this is a no-op
    void requestAvailableWidth() noexcept;

    // Notifies the available height later in suggestedSizeChanged
    // If the top and bottom anchors are not set this is a no-op
    void requestAvailableHeight() noexcept;

    bool setScreen(MScreen *screen) noexcept;
    MScreen *screen() const noexcept;

    bool setAnchor(CZBitset<CZEdge> edges) noexcept;
    CZBitset<CZEdge> anchor() const noexcept;

    bool setExclusiveZone(Int32 size) noexcept;
    Int32 exclusiveZone() const noexcept;

    bool setMargin(const SkIRect &margin) noexcept;
    const SkIRect &margin() const noexcept;

    bool setKeyboardInteractivity(KeyboardInteractivity mode) noexcept;
    KeyboardInteractivity keyboardInteractivity() const noexcept;

    bool setLayer(Layer layer) noexcept;
    Layer layer() const noexcept;

    bool setExclusiveEdge(CZEdge edge) noexcept;
    CZEdge exclusiveEdge() const noexcept;

    bool setScope(const std::string &scope) noexcept;
    const std::string &scope() const noexcept;

    const SkISize &suggestedSize() const noexcept;

    /**
     * @brief Signal triggered when the compositor requests to close the window.
     *
     * To ignore the request, call `event->ignore()`.
     *
     * If the request is not ignored, the toplevel window will be unmapped (see setMapped()) but not
     * destroyed.
     *
     * @note The protocol requires the client to destroy the role, so even if ignored, the surface will be briefly unmapped.
     *       This typically occurs when the assigned output is no longer available. Use this opportunity to reassign it to
     *       another output, or the compositor will pick one.
     */
    CZSignal<const AKWindowCloseEvent&> onBeforeClose;

    class Imp;
    Imp *imp() const noexcept;

protected:
    virtual void suggestedSizeChanged();
    virtual void render() noexcept;
    void onUpdate() noexcept override;

private:
    std::unique_ptr<Imp> m_imp;
};

#endif // MLAYERSHELL_H
