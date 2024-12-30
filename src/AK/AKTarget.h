#ifndef AKTARGET_H
#define AKTARGET_H

#include <AK/AKObject.h>
#include <AK/AKTransform.h>
#include <AK/AKSignal.h>
#include <AK/AKWeak.h>
#include <include/core/SkSurface.h>
#include <include/core/SkMatrix.h>
#include <include/core/SkRegion.h>
#include <yoga/Yoga.h>
#include <vector>

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
class AK::AKTarget : public AKObject
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
    void setViewport(const SkRect &viewport) noexcept
    {
        if (m_viewport == viewport)
            return;

        m_viewport = viewport;
        m_needsFullRepaint = true;
        markDirty();
    }

    const SkRect &viewport() const noexcept
    {
        return m_viewport;
    }

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

    AKTransform transform() const noexcept
    {
        return m_transform;
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

    void setBakedComponentsScale(SkScalar scale) noexcept
    {
        if (scale == m_bakedComponentsScale)
            return;

        m_bakedComponentsScale = scale;
        markDirty();
    }

    SkScalar bakedComponentsScale() const noexcept
    {
        return m_bakedComponentsScale;
    }

    const SkVector &scale() const noexcept
    {
        return m_xyScale;
    }

    std::shared_ptr<AKPainter> painter() const noexcept
    {
        return m_painter;
    }

    UInt32 fbId() const noexcept
    {
        return m_fbId;
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
     * @brief Indicates if a node visible on the target has changed one of its properties.
     *
     * Use the on.markedDirty() signal to detect when this property changes to true.
     * Nodes modify this property when they invoke the AKNode::addChange() method, or
     * when they are added to or removed from the scene. After the target is rendered
     * by its scene, the property is reset to false.
     */
    bool isDirty() const noexcept
    {
        return m_isDirty;
    }

    void markDirty() noexcept
    {
        if (isDirty())
            return;

        m_isDirty = true;
        on.markedDirty.notify(*this);
    }

    struct
    {
        /**
         * @brief Needs Repaint Signal
         *
         * This signal is triggered whenever the AKTarget::isDirty() property changes
         * from false to true. Nodes keep track of which targets they are visible on during an AKScene::render() call,
         * meaning this signal is only triggered by nodes that have been rendered at least once by a scene.
         */
        AKSignal<AKTarget&> markedDirty;
    } on;

private:
    friend class AKScene;
    friend class AKNode;
    friend class AKSubScene;
    AKTarget(AKScene *scene, std::shared_ptr<AKPainter> painter) noexcept;
    ~AKTarget();
    sk_sp<SkSurface>    m_surface;
    sk_sp<SkImage>      m_image;
    SkRect              m_viewport { 0.f, 0.f, 0.f, 0.f };
    SkIRect             m_globalIViewport;
    SkIRect             m_prevViewport;     // Rel to root
    SkMatrix            m_matrix;
    SkVector            m_xyScale;
    SkScalar            m_bakedComponentsScale { 1.f };
    SkRegion            m_prevClip;         // Rel to root
    SkRegion            m_damage;           // Rel to root
    SkRegion            m_opaque;           // Rel to root
    SkRegion            m_translucent;      // Rel to root
    SkRegion            m_damageRing[AK_MAX_BUFFER_AGE];    // Rel to root
    UInt32              m_damageIndex { 0 };
    AKScene *           m_scene;
    size_t              m_sceneLink;
    AKTransform         m_transform { AKTransform::Normal };
    UInt32              m_fbId;
    SkIRect             m_dstRect { 0, 0, 0, 0 };
    bool                m_isDirty { false };
    bool                m_isSubScene { false };
    bool                m_needsFullRepaint { true };
    std::shared_ptr<AKPainter> m_painter;
    std::vector<SkRegion> m_reactive;
    SkColor             m_clearColor { SK_ColorTRANSPARENT };
    UInt32              m_age { 0 };
};

#endif // AKTARGET_H
