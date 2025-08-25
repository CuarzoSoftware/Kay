#ifndef MWINDOWROLE_H
#define MWINDOWROLE_H

#include <CZ/Marco/protocols/lvr-invisible-region-client.h>
#include <CZ/Marco/protocols/viewporter-client.h>

#include <CZ/AK/AKVibrancy.h>
#include <CZ/CZSignal.h>
#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKSolidColor.h>
#include <CZ/AK/AKScene.h>
#include <CZ/Marco/Marco.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <set>

/**
 * @brief Base class for surface roles.
 *
 * ### UI Structure
 *
 * Each MSurface is an AKSolidColor node that represents the central node of the surface.
 * From the data perspective, the MSurface class contains the following members:
 * - `AKScene` (accessible via `scene()`)
 * - `AKTarget` (accessible via `target()`)
 * - `AKContainer` (accessible via `rootNode()`)
 *
 * From the UI perspective, the hierarchy is structured as follows:
 *
 * | - AKScene (scene())
 *   | - AKTarget (target())
 *     | - AKContainer (rootNode())
 *       |- MSurface (this class, the central node)
 *
 * ### Wayland Surface and Layout
 *
 * Most roles configure the internal Wayland surface dimensions to align with the layout of the central node.
 * Use the `layout()` functions to modify the dimensions of the Wayland surface accordingly.
 *
 * In specific cases, such as in `MToplevel`, the central node may be smaller than the actual surface size.
 * This allows room for additional features like window decorations.
 *
 * ### Adding Child Nodes
 *
 * As the central node itself, `MSurface` supports the addition of child nodes, like any other node. However,
 * its `setParent()` function is private and should not be accessed, as doing so would disrupt it's internal logic.
 */
class CZ::MSurface : public AKSolidColor
{
public:
    enum class Role
    {
        SubSurface,
        Toplevel,
        Popup,
        LayerSurface
    };

    ~MSurface();

    Role role() noexcept;

    /**
     * @brief The current scale factor.
     *
     * @return
     */
    Int32 scale() noexcept;
    const SkISize &surfaceSize() const noexcept;
    const SkISize &bufferSize() const noexcept;
    const std::set<MScreen*> &screens() const noexcept;
    void setMapped(bool mapped) noexcept;
    bool mapped() const noexcept;
    void update(bool force = false) noexcept;
    SkISize minContentSize() noexcept;
    const std::list<MSubsurface*> &subSurfaces() const noexcept;

    AKScene &scene() const noexcept;
    AKTarget *target() const noexcept;
    AKNode *rootNode() const noexcept;

    wl_surface *wlSurface() const noexcept;
    wl_callback *wlCallback() const noexcept;
    wp_viewport *wlViewport() const noexcept;
    lvr_invisible_region *wlInvisibleRegion() const noexcept;
    wl_egl_window *eglWindow() const noexcept;
    sk_sp<SkSurface> skSurface() const noexcept;
    EGLSurface eglSurface() const noexcept;

    AKVibrancyState vibrancyState() const noexcept;
    AKVibrancyStyle vibrancyStyle() const noexcept;
    virtual void vibrancyEvent(const AKVibrancyEvent &event);

    CZSignal<const AKVibrancyEvent &> onVibrancyChanged;
    CZSignal<> onMappedChanged;
    CZSignal<MScreen&> onEnteredScreen;
    CZSignal<MScreen&> onLeftScreen;
    CZSignal<UInt32> onCallbackDone;
    class Imp;
    Imp *imp() const noexcept;
protected:
    friend class MApplication;
    friend class MCSDShadow;
    MSurface(Role role) noexcept;
    virtual void onUpdate() noexcept;
    bool event(const CZEvent &event) noexcept override;
private:
    std::unique_ptr<Imp> m_imp;
    using AKNode::setParent;
    using AKNode::parent;
    using AKNode::setVisible;
};

#endif // MWINDOWROLE_H
