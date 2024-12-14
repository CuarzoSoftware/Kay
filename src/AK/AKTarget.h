#ifndef AKTARGET_H
#define AKTARGET_H

#include <AK/AKObject.h>
#include <AK/AKTransform.h>
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
     * @brief The destination surface for rendering.
     *
     * This is the surface where the rendered content will be stored.
     * Providing an invalid surface will trigger an internal assertion failure in `AKScene::render()`.
     */
    sk_sp<SkSurface> surface;

    /**
     * @brief Buffer age for damage tracking.
     *
     * Used to track the age of the buffer, as specified by the
     * [EGL_EXT_buffer_age](https://registry.khronos.org/EGL/extensions/EXT/EGL_EXT_buffer_age.txt) specification.
     *
     * Passing a value greater than 4 will trigger an internal assertion failure in `AKScene::render()`.
     */
    UInt32 age { 0 };

    /**
     * @brief Root node.
     *
     * Only the children of the root node are rendered.
     * The root node's bounds do not clip its children, but its layout properties affect them.
     */
    AKNode* root { nullptr };

    /**
     * @brief Viewport in logical coordinates relative to the root node.
     *
     * This defines the visible area in the scene that will be rendered, expressed in logical coordinates.
     */
    SkRect viewport { 0.f, 0.f, 0.f, 0.f };

    /**
     * @brief Scaling factor for the rendered content.
     *
     * By default, this is set to 1.0. The scaling factor is automatically determined based on the ratio between the destination rectangle and the viewport.
     * You can adjust this value to apply additional scaling to the content along both the X and Y axes.
     */
    Float32 scale { 1.f };

    /**
     * @brief Destination rectangle on the surface to render the viewport.
     *
     * This rectangle is defined in surface coordinates (i.e., buffer coordinates) relative to the surface's top-left corner.
     * The aspect ratio between the destination rectangle and the viewport determines the scaling factor for both axes.
     * Any pixels outside this rectangle remain unchanged.
     */
    SkIRect dstRect { 0, 0, 0, 0 };

    /**
     * @brief Viewport transformation.
     *
     * For instance, if `dstRect` covers the entire surface and a rotation of 90 degrees is applied (via `AKTransform::Rotated90`),
     * the viewport will be rotated 90° counterclockwise, and the top-left corner of the viewport will be rendered in the surface's bottom-left corner.
     */
    AKTransform transform { AKTransform::Normal };

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

    const SkVector &xyScale() const noexcept
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
private:
    friend class AKScene;
    friend class AKNode;
    friend class AKSubScene;
    AKTarget(AKScene *scene, std::shared_ptr<AKPainter> painter) noexcept;
    ~AKTarget();
    SkIRect             m_globalIViewport;
    SkIRect             m_prevViewport;     // Rel to root
    SkMatrix            m_matrix;
    SkVector            m_xyScale;
    SkRegion            m_prevClip;         // Rel to root
    SkRegion            m_damage;           // Rel to root
    SkRegion            m_opaque;           // Rel to root
    SkRegion            m_translucent;      // Rel to root
    SkRegion            m_damageRing[AK_MAX_BUFFER_AGE];    // Rel to root
    UInt32              m_damageIndex { 0 };
    AKScene *           m_scene;
    size_t              m_sceneLink;
    std::vector<AKNode*>m_nodes;
    UInt32              m_fbId;
    bool                m_isSubScene { false };
    std::shared_ptr<AKPainter> m_painter;
    YGConfigRef         m_yogaConfig;
};

#endif // AKTARGET_H
