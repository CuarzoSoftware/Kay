#ifndef AKNODE_H
#define AKNODE_H

#include <AK/AKObject.h>
#include <AK/AKWeak.h>
#include <AK/AKBitset.h>
#include <AK/AKLayout.h>

#include <bitset>
#include <cassert>
#include <include/core/SkImage.h>
#include <include/core/SkSurface.h>
#include <include/gpu/GrBackendSurface.h>
#include <include/gpu/gl/GrGLTypes.h>
#include <include/core/SkRegion.h>

#include <unordered_map>
#include <memory>
#include <vector>

class AK::AKNode : public AKObject
{
public:

    enum Caps : UInt32
    {
        Render              = 1 << 0,
        Bake                = 1 << 1,
        Scene               = 1 << 2,
        BackgroundEffect    = 1 << 3
    };

    virtual ~AKNode();

    typedef UInt32 Change;

    enum Changes : Change
    {
        Chg_Last
    };

    void addChange(Change change) noexcept;

    [[nodiscard]]
    const std::bitset<128> &changes() const noexcept;

    /* Insert at the end, nullptr unsets */
    void setParent(AKNode *parent) noexcept;
    AKNode *parent() const noexcept
    {
        return m_parent;
    }

    void insertBefore(AKNode *other) noexcept;
    void insertAfter(AKNode *other) noexcept;
    bool isSubchildOf(AKNode *node) const noexcept
    {
        if (!node || !parent()) return false;

        return parent() == node || parent()->isSubchildOf(node);
    }
    const std::vector<AKNode*> &children() const noexcept
    {
        return m_children;
    }

    UInt32 caps() const noexcept
    {
        return m_caps;
    }

    AKTarget *currentTarget() const noexcept
    {
        assert(t);
        return t->target;
    }

    bool visible() const noexcept
    {
        return m_flags.check(Visible);
    }

    void setVisible(bool visible) noexcept
    {
        m_flags.setFlag(Visible, visible);
    }

    bool clipsChildren() const noexcept
    {
        return m_flags.check(ClipsChildren);
    }

    void setClipsChildren(bool clip) noexcept
    {
        m_flags.setFlag(ClipsChildren, clip);
    }

    void setInputRegion(SkRegion *region) noexcept
    {
        m_inputRegion = region ? std::make_unique<SkRegion>(*region) : nullptr;
    }

    SkRegion *inputRegion() const noexcept
    {
        return m_inputRegion.get();
    }

    bool insideLastTarget() const noexcept
    {
        return m_insideLastTarget;
    }

    bool renderedOnLastTarget() const noexcept
    {
        return m_renderedOnLastTarget;
    }

    /* Layout */

    /* Relative to the closest subscene */
    const SkIRect rect() const noexcept
    {
        return m_rect;
    }

    /* Relative to the root node */
    const SkIRect globalRect() const noexcept
    {
        return m_globalRect;
    }

    AKLayout &layout() noexcept
    {
        return m_layout;
    }

    /* Effects */

    AKBackgroundEffect *backgroundEffect() const noexcept;
    void setBackgroundEffect(AKBackgroundEffect *backgroundEffect) noexcept;

    /* Triggered before the scene starts */
    virtual void onSceneBegin() {}

    /* Triggered before the scene starts rendering but
     * after the Yoga layout is updated */
    virtual void onLayoutUpdate() {}

    /**
     * @brief Reactive Region.
     *
     * This is a special region that causes damage to the scene
     * only when overlay or background damage from other nodes intersects with it.
     */
    SkRegion reactiveRegion;

    UInt64 userFlags { 0 };
    void *userData { nullptr };
private:
    friend class AKBackgroundEffect;
    friend class AKRenderable;
    friend class AKBakeable;
    friend class AKSubScene;
    friend class AKContainer;
    friend class AKTarget;
    friend class AKScene;

    enum Flags : UInt64
    {
        Visible             = 1L << 0,
        ClipsChildren       = 1L << 1,
    };

    struct TargetData : public AKObject
    {
        TargetData() noexcept { changes.set(); }
        AKTarget *target { nullptr };
        size_t targetLink;
        SkRegion prevLocalClip; // Rel to root
        SkIRect prevLocalRect,
            prevRect;
        SkRegion clientDamage,
            opaque, translucent,
            opaqueOverlay;
        std::shared_ptr<AKSurface> bake;
        std::bitset<128> changes;
        bool visible;
    };

    AKNode(AKNode *parent = nullptr) noexcept;
    AKNode *closestClipperParent() const noexcept;

    AKLayout m_layout;
    AKBitset<Flags> m_flags { Visible };
    AKWeak<AKBackgroundEffect> m_backgroundEffect;
    SkIRect m_rect;
    SkIRect m_globalRect;
    UInt32 m_caps { 0 };
    AKNode *m_parent { nullptr };
    std::vector<AKNode*> m_children;
    size_t m_parentLinkIndex;
    std::unique_ptr<SkRegion> m_inputRegion;
    mutable std::unordered_map<AKTarget*, TargetData> m_targets;
    AKWeak<TargetData> t;
    bool m_insideLastTarget { false };
    bool m_renderedOnLastTarget { false };
};

#endif // AKNODE_H
