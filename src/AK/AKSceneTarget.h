#ifndef AKSCENETARGET_H
#define AKSCENETARGET_H

#include <AK/AKTarget.h>
#include <AK/AKSignal.h>
#include <AK/AKWeak.h>
#include <include/core/SkSurface.h>
#include <include/core/SkMatrix.h>
#include <include/core/SkRegion.h>
#include <yoga/Yoga.h>
#include <unordered_set>

/**
 * @brief A rendering destination.
 *
 * A target encapsulates all the necessary information required for an `AKScene::render()` call.
 *
 * Targets are created using `AKScene::createTarget()` and are bound to a specific scene. They cannot be shared across scenes.
 *
 * In desktop applications, nodes are typically displayed within a single window. As such, one scene with a single target is usually sufficient per window.
 * However, in compositors, where nodes are displayed across multiple screens, a single scene is shared. A separate target is created for each screen, allowing nodes
 * to maintain their internal state independently for each screen. For instance, if damage is applied to a node and rendered to one target, the damage will only be
 * cleared for that specific target when rendered, leaving other targets unaffected.
 *
 * Targets must be destroyed on the same thread or context in which they were created, using `AKScene::destroyTarget()`. They are also automatically destroyed when the scene is destroyed.
 *
 * Destroying a target also frees all associated internal state for rendered nodes.
 */
class AK::AKSceneTarget : public AKTarget
{
public:

    /**
     * @brief The destination surface.
     *
     * Providing an invalid surface will trigger an internal assertion failure in `AKScene::render()`.
     */
    void setSurface(sk_sp<SkSurface> surface) noexcept
    {
        m_surface = surface;
    }

    sk_sp<SkSurface> surface() const noexcept
    {
        return m_surface;
    }

    void setImage(sk_sp<SkImage> image) noexcept
    {
        m_image = image;
    }

    sk_sp<SkImage> image() const noexcept
    {
        return m_image;
    }

    void setClearColor(SkColor color) noexcept
    {
        if (m_clearColor == color)
            return;

        m_clearColor = color;
        m_needsFullRepaint = true;
        markDirty();
    }

    SkColor clearColor() const noexcept
    {
        return m_clearColor;
    }

    /**
     * @brief Buffer age.
     *
     * Used for damage tracking, as specified by the
     * [EGL_EXT_buffer_age](https://registry.khronos.org/EGL/extensions/EXT/EGL_EXT_buffer_age.txt) specification.
     */
    void setAge(UInt32 age) noexcept
    {
        m_age = age;
    }

    UInt32 age() const noexcept
    {
        return m_age;
    }

    /**
     * @brief Viewport in logical coordinates relative to the root node.
     *
     * This defines the visible area in the scene that will be rendered, expressed in logical coordinates.
     */
    void setViewport(const SkRect &viewport) noexcept;

    /**
     * @brief Destination rectangle on the surface to render the viewport.
     *
     * This rectangle is defined in surface coordinates (i.e., buffer coordinates) relative to the surface's top-left corner.
     * The aspect ratio between the destination rectangle and the viewport determines the scaling factor for both axes.
     * Any pixels outside this rectangle remain unchanged.
     */
    void setDstRect(const SkIRect &rect) noexcept
    {
        if (m_dstRect == rect)
            return;

        m_dstRect = rect;
        m_needsFullRepaint = true;
        markDirty();
    }

    const SkIRect &dstRect() const noexcept
    {
        return m_dstRect;
    }

    /**
     * @brief Viewport transform.
     *
     * For instance, if `dstRect` covers the entire surface and a rotation of 90 degrees is applied (via `AKTransform::Rotated90`),
     * the viewport will be rotated 90° counterclockwise, and the top-left corner of the viewport will be rendered in the surface's bottom-left corner.
     */
    void setTransform(AKTransform transform) noexcept
    {
        if (m_transform == transform)
            return;

        m_transform = transform;
        m_needsFullRepaint = true;
        markDirty();
    }

    /**
     * @brief Additional damage.
     *
     * This region, specified in logical coordinates relative to the root node, represents the area that must be repainted.
     * This is added to `outDamageRegion`.
     *
     * If set to `nullptr`, no additional damage is added.
     */
    const SkRegion* inDamageRegion { nullptr };

    /**
     * @brief Region to limit the rendering to.
     *
     * This region, defined in logical coordinates relative to the root node, restricts the area of the scene that will be rendered.
     * This also impacts `outDamageRegion`.
     *
     * If set to `nullptr`, no clipping is applied.
     */
    const SkRegion* inClipRegion { nullptr };

    /**
     * @brief Calculated damage region after rendering.
     *
     * This region, in logical coordinates relative to the root node, represents the area that was repainted.
     * Not clipped by the viewport.
     *
     * If set to `nullptr`, no damage calculation is performed.
     */
    SkRegion* outDamageRegion { nullptr };

    /**
     * @brief Calculated opaque region.
     *
     * This region, in logical coordinates relative to the root node, identifies areas that are fully opaque after rendering.
     * Not clipped by the viewport.
     *
     * If set to `nullptr`, no opacity calculation is performed.
     */
    SkRegion* outOpaqueRegion { nullptr };

    /**
     * @brief Calculated input region.
     *
     * This region, in logical coordinates relative to the root node, represents the area that receives input events.
     * It is the union of all input regions from the nodes, clipped by their respective boundaries and their parents' boundaries.
     *
     * If set to `nullptr`, no input region calculation is performed.
     */
    SkRegion* outInputRegion { nullptr };

    void setBakedComponentsScale(Int32 scale) noexcept;

    Int32 bakedComponentsScale() const noexcept
    {
        return m_bakedComponentsScale;
    }

    const SkRegion &accumulatedDamage() const noexcept
    {
        return m_damage;
    }

    AKScene &scene() const noexcept
    {
        return *m_scene;
    }

    /**
     * @brief Checks if the target needs to be repainted.
     *
     * This property is automatically set to `true` when:
     * - A node intercepted by the target invokes the `AKNode::addChange()` method.
     * - A node intercepted by the target has the `AKNode::animated()` property set to `true`.
     * - A node is added to or removed from the scene.
     *
     * After the target is rendered by its scene, this property is reset to `false`. However,
     * an exception occurs for nodes with the `AKNode::animated()` property set to `true`. If such a node
     * is visible and intercepted by the target during `AKScene::render()`, the property remains `true`
     * to ensure continuous updates.
     *
     * Use `signalMarkedDirty()` to detect when this property changes to `true`.
     *
     * @return `true` if the node has changed and is marked as dirty, `false` otherwise.
     */
    bool isDirty() const noexcept
    {
        return m_isDirty;
    }

    /**
     * @brief Marks the target as dirty, indicating that it needs to be repainted.
     *
     * For more details check `isDirty()`.
     */
    void markDirty() noexcept;

    /**
     * @brief Enables or disables layout calculation during rendering.
     *
     * By default, scenes call `root()->layout()->calculate()` during `LScene::render()`.
     * However, in some cases (e.g., desktop applications), you may need to calculate
     * the layout beforehand to set window dimensions. Use this function to disable
     * layout calculation during rendering to avoid redundant calculations.
     *
     * @note If disabled, ensure the root layout is updated before rendering
     *       to avoid undesired results.
     *
     * @param enabled If true, layout is calculated during rendering.
     */
    void setRenderCalculatesLayout(bool enabled) noexcept
    {
        m_renderCalculatesLayout = enabled;
    }

    /**
     * @brief Returns whether layout calculation is enabled during rendering.
     *
     * @see setRenderCalculatesLayout()
     */
    bool renderCalculatesLayout() const noexcept
    {
        return m_renderCalculatesLayout;
    }

    const SkRect &viewport() const noexcept override { return m_viewport; };
    AKTransform transform() const noexcept override { return m_transform; };
    const SkVector &xyScale() const noexcept override { return m_xyScale; };
    UInt32 fbId() const noexcept override { return m_fbId; };

    struct
    {
        /**
         * @brief Needs Repaint Signal
         *
         * This signal is triggered whenever the AKSceneTarget::isDirty() property changes
         * from false to true. Nodes keep track of which targets they are visible on during an AKScene::render() call,
         * meaning this signal is only triggered by nodes that have been rendered at least once by a scene.
         */
        AKSignal<AKSceneTarget&> markedDirty;
    } on;

private:
    friend class AKScene;
    friend class AKNode;
    friend class AKSubScene;
    friend class AKLayout;
    AKSceneTarget(AKScene *scene) noexcept;
    AKCLASS_NO_COPY(AKSceneTarget)
    ~AKSceneTarget();
    SkRect              m_viewport { 0.f, 0.f, 0.f, 0.f };
    SkVector            m_xyScale;
    UInt32              m_fbId;
    AKTransform         m_transform { AKTransform::Normal };
    sk_sp<SkSurface>    m_surface;
    sk_sp<SkImage>      m_image;
    SkIRect             m_globalIViewport;
    SkIRect             m_prevViewport;     // Rel to root
    SkMatrix            m_matrix;
    Int32               m_bakedComponentsScale { 1 };
    Int32               m_prevBakedComponentsScale { 1 };
    SkRegion            m_prevClip;         // Rel to root
    SkRegion            m_damage;           // Rel to root
    SkRegion            m_opaque;           // Rel to root
    SkRegion            m_translucent;      // Rel to root
    SkRegion            m_damageRing[AK_MAX_BUFFER_AGE];    // Rel to root
    UInt32              m_damageIndex { 0 };
    AKScene *           m_scene;
    size_t              m_sceneLink;
    SkIRect             m_dstRect { 0, 0, 0, 0 };
    bool                m_isDirty { false };
    bool                m_needsFullRepaint { true };
    bool                m_renderCalculatesLayout { true };
    std::unordered_set<BackgroundDamageTracker*> m_bdts;
    SkColor             m_clearColor { SK_ColorTRANSPARENT };
    UInt32              m_age { 0 };
};

#endif // AKSCENETARGET_H
