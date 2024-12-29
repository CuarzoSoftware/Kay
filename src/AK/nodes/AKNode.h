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

#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <vector>

class AK::AKNode : public AKObject
{
public:

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
        bool onBakeGeneratedDamage;
    };

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
        Chg_Layout,
        Chg_Parent,
        Chg_ChildrenClipping,
        Chg_Last
    };
    void addChange(Change change) noexcept;

    [[nodiscard]]
    const std::bitset<128> &changes() const noexcept;

    void setVisible(bool visible) noexcept
    {
        layout().setDisplay(visible ? YGDisplayFlex : YGDisplayNone);
    }

    bool visible() const noexcept
    {
        return layout().display() != YGDisplayNone;
    }

    /* Insert at the end, nullptr unsets */
    void setParent(AKNode *parent) noexcept;
    AKNode *parent() const noexcept
    {
        return m_parent;
    }

    AKNode *topmostParent() const noexcept;

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

    const std::vector<AKTarget*> &intersectedTargets() const noexcept
    {
        return m_intersectedTargets;
    }

    bool childrenClippingEnabled() const noexcept
    {
        return m_flags.check(ChildrenClipping);
    }

    void enableChildrenClipping(bool enable) noexcept
    {
        if (childrenClippingEnabled() == enable)
            return;

        m_flags.setFlag(ChildrenClipping, enable);
        addChange(Chg_ChildrenClipping);
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

    const AKLayout &layout() const noexcept
    {
        return m_layout;
    }

    AKLayout &layout() noexcept
    {
        return m_layout;
    }

    /* Effects */

    const std::unordered_set<AKBackgroundEffect*> &backgroundEffects() const noexcept
    {
        return m_backgroundEffects;
    }

    void addBackgroundEffect(AKBackgroundEffect *backgroundEffect) noexcept;
    void removeBackgroundEffect(AKBackgroundEffect *backgroundEffect) noexcept;

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

    TargetData *targetData(AKTarget *target) const noexcept
    {
        auto it = m_targets.find(target);
        return it == m_targets.end() ? nullptr : &it->second;
    }
private:
    friend class AKBackgroundEffect;
    friend class AKRenderable;
    friend class AKBakeable;
    friend class AKSubScene;
    friend class AKContainer;
    friend class AKTarget;
    friend class AKScene;
    friend class AKLayout;

    enum Flags : UInt64
    {
        ChildrenClipping    = 1L << 0,
    };

    AKNode(AKNode *parent = nullptr) noexcept;
    AKNode *closestClipperParent() const noexcept;

    AKWeak<TargetData> t;
    AKLayout m_layout { *this };
    AKBitset<Flags> m_flags { 0 };
    std::unordered_set<AKBackgroundEffect*> m_backgroundEffects;
    SkIRect m_rect;
    SkIRect m_globalRect;
    UInt32 m_caps { 0 };
    AKNode *m_parent { nullptr };
    std::vector<AKNode*> m_children;
    size_t m_parentLinkIndex;
    std::unique_ptr<SkRegion> m_inputRegion;
    std::vector<AKTarget*> m_intersectedTargets;
    mutable std::unordered_map<AKTarget*, TargetData> m_targets;
    bool m_insideLastTarget { false };
    bool m_renderedOnLastTarget { false };
};

#endif // AKNODE_H
