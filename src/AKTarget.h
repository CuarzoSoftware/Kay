#ifndef AKTARGET_H
#define AKTARGET_H

#include <AKObject.h>
#include <AKTransform.h>
#include <include/core/SkSurface.h>
#include <include/core/SkMatrix.h>
#include <include/core/SkRegion.h>
#include <vector>

class AK::AKTarget : public AKObject
{
public:
    sk_sp<SkSurface>    surface;
    SkPoint             pos       { 0.f, 0.f};
    Float32             scale     { 1 };
    AKTransform         transform { AKTransform::Normal };
    UInt32              age       { 0 };

    SkRegion            outOpaqueRegion;
    SkRegion            outDamageRegion;
private:
    friend class AKScene;
    friend class AKNode;
    AKTarget(AKScene *scene) noexcept;
    ~AKTarget();
    SkIRect             m_viewport;
    SkMatrix            m_matrix;
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
