#ifndef CZ_AKTARGET_H
#define CZ_AKTARGET_H

#include <CZ/AK/AKBackgroundDamageTracker.h>
#include <CZ/Ream/Ream.h>

#include <CZ/Core/CZSignal.h>
#include <CZ/Core/CZWeak.h>

#include <CZ/skia/core/SkSurface.h>
#include <CZ/skia/core/SkMatrix.h>
#include <CZ/skia/core/SkRegion.h>
#include <yoga/Yoga.h>

/**
 * @brief A scene render destination.
 *
 * A target describes where a scene stores the result of a `AKScene::render()` call.
 *
 * Targets are created using `AKScene::makeTarget()` and are bound to a specific scene. They cannot be shared across scenes.
 *
 * In desktop applications, nodes are typically displayed within a single window at a time. As such, one scene with a single target is usually sufficient per window.
 * However, in compositors, where nodes can appear in multiple screens at a time, a single scene can be created with separate target for each screen, allowing nodes
 * to maintain their internal state independently for each screen. For instance, if a node is damaged and rendered into a target, the damage will only be cleared for
 * that specific target but remain damaged for others.
 */
class CZ::AKTarget : public AKObject
{
public:
    ~AKTarget() noexcept;

    /**
     * @brief The scene that created this target
     */
    std::shared_ptr<AKScene> scene() const noexcept { return m_scene; }

    /**
     * @brief The destination surface (mandatory).
     *
     * The internal image must have the RImageCap_Src and RImageCap_SkSurface capabilities.
     *
     * Note: Nodes are automatically damaged if their position relative to RSurface::viewport().topLeft() change.
     *       If you only modify RSurface::dst() set the age to 0 to force a full repaint.
     */
    std::shared_ptr<RSurface> surface;

    /**
     * @brief Buffer age
     *
     * @see https://registry.khronos.org/EGL/extensions/EXT/EGL_EXT_buffer_age.txt.
     */
    UInt32 age { 0 };

    /**
     * @brief Toggles layout calculation during rendering.
     *
     * If true, AKScene::render() internally calls root()->layout()->calculate().
     *
     * There are cases where you may need to calculate the root node layout
     * beforehand e.g. to set window dimensions in a desktop application. In such cases,
     * set this variable to false to avoid redundant calculations.
     */
    bool layoutOnRender { true };

    /* All regions below are in virtual coords relative to RSurface::viewport().topLeft() */

    /**
     * @brief Optional additional damage
     */
    const SkRegion *inDamage { nullptr };

    /**
     * @brief Optional clip in world coordinates
     */
    const SkRegion *inClip { nullptr };

    /**
     * @brief Filled with the generated damage (if set)
     */
    SkRegion *outDamage { nullptr };

    /**
     * @brief Filled with the calculated opaque region (if set)
     */
    SkRegion *outOpaque { nullptr };

    /**
     * @brief Filled with the calculated invisible region (if set)
     */
    SkRegion *outInvisible { nullptr };

    /**
     * @brief Filled with the calculated input region (if set)
     */
    SkRegion *outInput { nullptr };

    /**
     * @brief Clear color (unpremult alpha)
     *
     * If changed, the entire target is damaged.
     */
    void setClearColor(SkColor color) noexcept;
    SkColor clearColor() const noexcept { return m_clearColor; }

    /**
     * @brief Scale factor [1, 4] used by AKBakeables
     *
     * @note Nodes use the greatest scale from the targets they intersect
     */
    void setBakedNodesScale(Int32 scale) noexcept;
    Int32 bakedNodesScale() const noexcept { return m_bakedNodesScale; }

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
     * Use onMarkedDirty to detect when this property changes to `true`.
     *
     * @return `true` if the node has changed and is marked as dirty, `false` otherwise.
     */
    bool isDirty() const noexcept { return m_isDirty; }
    void markDirty() noexcept;

    /**
     * @brief Marked Dirty Signal
     *
     * This signal is triggered whenever the AKTarget::isDirty() property changes
     * from false to true. Nodes keep track of which targets they are visible on during an AKScene::render() call,
     * meaning this signal is only triggered by nodes that have been rendered at least once by a scene.
     */
    CZSignal<AKTarget&> onMarkedDirty;
private:
    friend class AKScene;
    friend class AKNode;
    friend class AKSubScene;
    friend class AKLayout;
    AKTarget(std::shared_ptr<AKScene> scene) noexcept;
    std::shared_ptr<AKScene>  m_scene;
    Int32               m_bakedNodesScale { 1 };
    Int32               m_prevBakedNodesScale { 1 };

    // Rounded RSurface::viewport()
    SkIRect             m_worldViewport {};

    // m_worldViewport moved to (0,0)
    SkIRect             m_sceneViewport {};
    SkRegion            m_clip;
    SkRegion            m_damage;

    // Accumulates opaque regions to clip background nodes during AKScene::render()
    SkRegion            m_opaque;
    SkRegion            m_translucent;
    SkRegion            m_damageRing[AK_MAX_BUFFER_AGE];
    UInt32              m_damageIndex { 0 };
    bool                m_isDirty { false };
    bool                m_needsFullRepaint { true };

    std::vector<CZWeak<AKBackgroundDamageTracker>>    m_bdts;
    std::vector<CZWeak<AKBackgroundDamageTracker>>    m_bdtsPrev;
    std::vector<SkIRect>            m_bdtPrevRectsTranslated;
    SkColor             m_clearColor { SK_ColorTRANSPARENT };
};


#endif // CZ_AKTARGET_H
