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

    class RIterator
    {
    public:
        RIterator(AKNode *node) noexcept { reset(node); }
        void reset(AKNode *node) noexcept;
        bool done() const noexcept { return m_done; }
        void next() noexcept;
        void jumpTo(AKNode *node) noexcept;
        AKNode *end() const noexcept { return m_end; }
        AKNode *node() const noexcept { return m_node; }
    private:
        AKNode *m_node, *m_end;
        bool m_done;
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
        bool onBakeGeneratedDamage;
    };

    enum Caps : UInt32
    {
        Render              = 1 << 0,
        Bake                = 1 << 1,
        Scene               = 1 << 2,
        BackgroundEffect    = 1 << 3
    };
    AKNode() noexcept { theme(); }
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

    bool hasPointerFocus() const noexcept
    {
        return m_flags.check(HasPointerFocus);
    }

    void enablePointerGrab(bool enabled) noexcept;
    bool pointerGrabEnabled() const noexcept
    {
        return m_flags.check(PointerGrab);
    }

    AKNode *topmostInvisibleParent() const noexcept;

    /* Insert at the end, nullptr unsets */
    void setParent(AKNode *parent) noexcept;
    AKNode *parent() const noexcept { return m_parent; }
    AKNode *topmostParent() const noexcept;
    AKNode *bottommostChild() const noexcept;
    void insertBefore(AKNode *other) noexcept;
    void insertAfter(AKNode *other) noexcept;
    AKNode *next() const noexcept
    {
        return m_parent && m_parent->m_children.back() != this ? m_parent->m_children[m_parentLinkIndex + 1] : nullptr;
    }

    AKNode *prev() const noexcept
    {
        return m_parent && m_parentLinkIndex > 0 ? m_parent->m_children[m_parentLinkIndex - 1] : nullptr;
    }

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

    bool isRoot() const noexcept { return m_flags.check(IsRoot); }

    AKTarget *currentTarget() const noexcept
    {
        assert(t);
        return t->target;
    }

    const std::vector<AKTarget*> &intersectedTargets() const noexcept { return m_intersectedTargets; }
    bool childrenClippingEnabled() const noexcept { return m_flags.check(ChildrenClipping); }

    void enableChildrenClipping(bool enable) noexcept
    {
        if (childrenClippingEnabled() == enable)
            return;

        m_flags.setFlag(ChildrenClipping, enable);
        addChange(Chg_ChildrenClipping);
    }

    void setInputRegion(SkRegion *region) noexcept { m_inputRegion = region ? std::make_unique<SkRegion>(*region) : nullptr; }
    SkRegion *inputRegion() const noexcept { return m_inputRegion.get(); }
    bool insideLastTarget() const noexcept { return m_flags.check(InsideLastTarget); }
    bool renderedOnLastTarget() const noexcept { return m_flags.check(RenderedOnLastTarget); }

    /* Layout */

    /* Relative to the closest subscene */
    const SkIRect rect() const noexcept { return m_rect;}

    /* Relative to the root node */
    const SkIRect globalRect() const noexcept { return m_globalRect; }
    const AKLayout &layout() const noexcept { return m_layout; }
    AKLayout &layout() noexcept { return m_layout; }

    /* Effects */

    const std::unordered_set<AKBackgroundEffect*> &backgroundEffects() const noexcept { return m_backgroundEffects; }
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

    AKScene *scene() const noexcept { return m_scene; }

protected:
    virtual void onEvent(const AKEvent &event) { (void)event; }

private:
    friend class AKBackgroundEffect;
    friend class AKRenderable;
    friend class AKBakeable;
    friend class AKSubScene;
    friend class AKContainer;
    friend class AKTarget;
    friend class AKScene;
    friend class AKLayout;

    enum Flags : UInt32
    {
        ChildrenClipping        = 1 << 0,
        IsRoot                  = 1 << 1,
        Notified                = 1 << 2,
        InsideLastTarget        = 1 << 3,
        RenderedOnLastTarget    = 1 << 4,
        HasPointerFocus         = 1 << 5,
        ChildHasPointerFocus    = 1 << 6,
        PointerGrab             = 1 << 7
    };

    AKNode(AKNode *parent = nullptr) noexcept;
    AKNode *closestClipperParent() const noexcept;
    void setScene(AKScene *scene) noexcept;
    void propagateScene(AKScene *scene) noexcept;
    void setParentPrivate(AKNode *parent, bool handleChanges) noexcept;
    void addFlagsAndPropagate(UInt32 flags) noexcept;
    void removeFlagsAndPropagate(UInt32 flags) noexcept;
    void setFlagsAndPropagateToParents(UInt32 flags, bool set) noexcept;

    AKWeak<TargetData> t;
    AKWeak<AKScene> m_scene;
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
};

#endif // AKNODE_H
