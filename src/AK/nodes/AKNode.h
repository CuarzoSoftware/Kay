#ifndef AKNODE_H
#define AKNODE_H

#include <AK/AKCursor.h>
#include <AK/AKObject.h>
#include <AK/AKWeak.h>
#include <AK/AKBitset.h>
#include <AK/AKLayout.h>

#include <bitset>
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

    enum LayoutChanges
    {
        Pos = 1 << 0,
        Size = 1 << 1,
        Scale = 1 << 2
    };

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
        SkIRect prevLocalRect;
        SkRegion clientDamage,
            opaque, translucent,
            opaqueOverlay;
        std::bitset<128> changes;
        bool visible { true };
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
        Chg_LayoutPos,
        Chg_LayoutSize,
        Chg_LayoutScale,
        Chg_Parent,
        Chg_ChildrenClipping,
        Chg_Last
    };
    void addChange(Change change) noexcept;

    /**
     * @brief Marks all intersected targets as dirty.
     *
     * This function triggers the AK::AKTarget::onMarkedDirty() signal
     * for all intersected targets, which, depending on the implementation,
     * may lead to a window or screen repaint.
     *
     * @note The addChange() function automatically calls this function internally.
     */
    void repaint() noexcept;

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

    AKTarget *currentTarget() const noexcept;

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
    const SkIRect sceneRect() const noexcept { return m_rect;}

    /* Relative to the root node */
    const SkIRect globalRect() const noexcept { return m_globalRect; }
    const AKLayout &layout() const noexcept { return m_layout; }
    Int32 scale() const noexcept { return m_scale; }
    AKLayout &layout() noexcept { return m_layout; }

    /* Effects */

    const std::unordered_set<AKBackgroundEffect*> &backgroundEffects() const noexcept { return m_backgroundEffects; }
    void addBackgroundEffect(AKBackgroundEffect *backgroundEffect) noexcept;
    void removeBackgroundEffect(AKBackgroundEffect *backgroundEffect) noexcept;

    /* Triggered before the scene starts rendering and
     * after rect() and globalRect() are calculated */
    virtual void onSceneBegin() {}

    /* Triggered before the scene starts rendering but
     * after rect() and globalRect() are calculated */
    virtual void onSceneCalculatedRect() {}

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
    AKSubScene *subScene() const noexcept { return m_subScene; }
    AKNode *root() const noexcept;

    bool activated() const noexcept;

    AKCursor cursor() const noexcept { return m_cursor; }
    void setCursor(AKCursor cursor) { m_cursor = cursor; }

    /**
     * @brief Sets whether the node is being animated.
     *
     * When set to `true`, all intersected targets (retrieved via `intersectedTargets()`) will
     * continuously be marked as dirty, causing the scene to repaint them continuously. This is useful
     * for nodes that are actively being animated and need to trigger updates in the rendering loop.
     *
     * @param enabled `true` to mark the node as animated, `false` otherwise. The default value is `false`.
     */
    void setAnimated(bool enabled) noexcept;

    /**
     * @brief Returns whether the node is currently being animated.
     *
     * This function checks the internal flags to determine if the node is in an animated state.
     * An animated node will continuously mark its intersected targets as dirty, causing scenes
     * to repaint them as long as the animation is active.
     *
     * @return `true` if the node is being animated, `false` otherwise.
     */
    bool animated() const noexcept
    {
        return m_flags.check(Animated);
    }

    struct {
        AKSignal<const AKEvent&> event;
    } on;

    AKSignal<AKBitset<LayoutChanges>> signalLayoutChanged;

protected:
    virtual void onEvent(const AKEvent &event) { on.event.notify(event); }

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
        ChildrenClipping            = 1 << 0,
        IsRoot                      = 1 << 1,
        Notified                    = 1 << 2,
        InsideLastTarget            = 1 << 3,
        RenderedOnLastTarget        = 1 << 4,
        HasPointerFocus             = 1 << 5,
        ChildHasPointerFocus        = 1 << 6,
        PointerGrab                 = 1 << 7,
        DiminishOpacityOnInactive   = 1 << 8,
        Animated                    = 1 << 9,
        ChildrenNeedPosUpdate       = 1 << 10,
        ChildrenNeedScaleUpdate     = 1 << 11
    };

    AKNode(AKNode *parent = nullptr) noexcept;
    AKNode *closestClipperParent() const noexcept;
    void setScene(AKScene *scene) noexcept;
    void propagateScene(AKScene *scene) noexcept;
    void updateSubScene() noexcept;
    void setParentPrivate(AKNode *parent, bool handleChanges) noexcept;
    void addFlagsAndPropagate(UInt32 flags) noexcept;
    void removeFlagsAndPropagate(UInt32 flags) noexcept;
    void setFlagsAndPropagateToParents(UInt32 flags, bool set) noexcept;

    bool m_skip { false };
    AKWeak<TargetData> t;
    AKWeak<AKScene> m_scene;
    AKWeak<AKSubScene> m_subScene;
    AKLayout m_layout { *this };
    AKBitset<Flags> m_flags { 0 };
    std::unordered_set<AKBackgroundEffect*> m_backgroundEffects;
    SkIRect m_rect;
    SkIRect m_globalRect { 0, 0, 0, 0 };
    Int32 m_scale { 1 };
    UInt32 m_caps { 0 };
    AKNode *m_parent { nullptr };
    std::vector<AKNode*> m_children;
    size_t m_parentLinkIndex;
    std::unique_ptr<SkRegion> m_inputRegion;
    std::vector<AKTarget*> m_intersectedTargets;
    mutable std::unordered_map<AKTarget*, TargetData> m_targets;
    AKCursor m_cursor { AKCursor::Default };
};

#endif // AKNODE_H
