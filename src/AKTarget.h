#ifndef AKTARGET_H
#define AKTARGET_H

#include <AKObject.h>
#include <AKTransform.h>
#include <include/core/SkSurface.h>
#include <include/core/SkMatrix.h>
#include <include/core/SkRegion.h>
#include <yoga/Yoga.h>
#include <vector>

class AK::AKTarget : public AKObject
{
public:
    /**
     * @brief Destination surface.
     *
     * The surface where the result will be stored.
     * Providing an invalid surface will cause AKScene::render() to fail.
     */
    sk_sp<SkSurface> surface;

    /**
     * @brief Age of the buffer.
     *
     * Used for damage tracking, following the
     * https://registry.khronos.org/EGL/extensions/EXT/EGL_EXT_buffer_age.txt spec.
     *
     * Passing an age > 4 will cause AKScene::render() to fail.
     */
    UInt32 age { 0 };

    /**
     * @brief Root node.
     *
     * Children nodes are not clipped by the root boundaries but are affected
     * by its layout properties. The root node itself is not captured,
     * only its children.
     */
    AKNode* root { nullptr };

    /**
     * @brief Viewport relative to the root node to capture.
     *
     * In logical coordinates.
     */
    SkRect viewport { 0.f, 0.f, 0.f, 0.f };

    /**
     * @brief Scale factor.
     *
     * Affects the buffer size of baked components.
     */
    Float32 scale { 1.f };

    /**
     * @brief Rect within the surface to render the viewport to.
     *
     * The rect is specified in buffer coordinates, relative to the top-left
     * corner of the surface.
     *
     * Pixels outside the rect remain unchanged.
     *
     * If the rect extends beyond the surface size, AKScene::render() will fail.
     */
    SkIRect dstRect { 0, 0, 0, 0 };

    /**
     * @brief Transform applied to the dstRect.
     */
    AKTransform transform { AKTransform::Normal };

    /**
     * @brief Calculated damage.
     *
     * Region within the viewport (without the offset) that was repainted.
     *
     * Setting it to `nullptr` avoids calculation.
     */
    SkRegion* outDamageRegion { nullptr };

    /**
     * @brief Calculated opaque region.
     *
     * Region within the viewport (without the offset) that is fully opaque.
     *
     * Setting it to `nullptr` avoids calculation.
     */
    SkRegion* outOpaqueRegion { nullptr };

    /**
     * @brief Calculated invisible region.
     *
     * Region within the viewport (without the offset) that is fully transparent.
     *
     * Setting it to `nullptr` avoids calculation.
     */
    SkRegion* outInvisibleRegion { nullptr };

    /**
     * @brief Calculated input region.
     *
     * Region within the viewport (without the offset) that receives input.
     *
     * Setting it to `nullptr` avoids calculation.
     */
    SkRegion* outInputRegion { nullptr };

private:
    friend class AKScene;
    friend class AKNode;
    friend class AKSubScene;
    AKTarget(AKScene *scene) noexcept;
    ~AKTarget();
    SkIRect             m_globalIViewport;
    SkRect              m_prevViewport;
    SkMatrix            m_matrix;
    SkPoint             m_xyScale;
    SkRegion            m_damage;
    SkRegion            m_opaque;
    SkRegion            m_translucent;
    SkRegion            m_damageRing[4];
    UInt32              m_damageIndex { 0 };
    AKScene *           m_scene;
    size_t              m_sceneLink;
    std::vector<AKNode*>m_nodes;
};

#endif // AKTARGET_H
