#ifndef AKNODE_H
#define AKNODE_H

#include <AK/AKBackgroundDamageTracker.h>
#include <AK/AKCursor.h>
#include <AK/AKObject.h>
#include <AK/AKWeak.h>
#include <AK/AKBitset.h>
#include <AK/AKLayout.h>
#include <AK/AKChanges.h>

#include <include/core/SkImage.h>
#include <include/core/SkSurface.h>
#include <include/core/SkRegion.h>

#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <vector>

namespace AK
{
    class MSurface;
};

/**
 * @defgroup AKNodes Components
 * @brief List of all components.
 * @{
 */

/**
 * @brief Base class for components.
 */
class AK::AKNode : public AKObject
{
public:

    /**
     * @brief Iterator for traversing a tree of nodes in order.
     *
     * This class allows iterating through a tree of nodes in order, that is,
     * from the first child to the first child and from the bottom-most node to the root.
     */
    class Iterator
    {
    public:
        /**
         * @brief Constructs an iterator.
         *
         * Initializes the iterator with the given starting node.
         *
         * @param node The starting point of the iteration.
         */
        Iterator(AKNode *node) noexcept { reset(node); }

        /**
         * @brief Resets the starting point of the iterator.
         *
         * Sets the starting node for the iteration.
         *
         * @param node The new starting point for the iteration.
         */
        void reset(AKNode *node) noexcept;

        /**
         * @brief Checks if the end of the iteration has been reached.
         *
         * Determines whether the reverse iteration has reached the end.
         *
         * @return `true` if the end has been reached, `false` otherwise.
         */
        bool done() const noexcept { return m_done; }

        /**
         * @brief Advances the iterator to the next node.
         *
         * Moves the iterator to the next node.
         */
        void next() noexcept;

        /**
         * @brief Jumps to a specified node.
         *
         * More efficient than reset, but assumes the node has the same root as the current node.
         * If not, a segmentation fault may occur.
         *
         * @warning Use with caution, as it assumes the node shares the same root as the current node.
         *
         * @param node The node to jump to.
         */
        void jumpTo(AKNode *node) noexcept;

        /**
         * @brief Gets the root node.
         *
         * Always returns the root node of the tree.
         *
         * @return A pointer to the root node.
         */
        AKNode *end() const noexcept { return m_end; }

        /**
         * @brief Gets the current node.
         *
         * Returns the current node being pointed to by the iterator.
         *
         * @return A pointer to the current node.
         */
        AKNode *node() const noexcept { return m_node; }
    private:
        AKNode *m_node, *m_end;
        bool m_done;
    };

    /**
     * @brief Reverse iterator for traversing a tree of nodes in reverse order.
     *
     * This class allows iterating through a tree of nodes in reverse order, that is,
     * from the last child to the first child and from the bottom-most node to the root.
     */
    class RIterator
    {
    public:
        /**
         * @brief Constructs a reverse iterator.
         *
         * Initializes the reverse iterator with the given starting node.
         *
         * @param node The starting point of the reverse iteration.
         */
        RIterator(AKNode *node) noexcept { reset(node); }

        /**
         * @brief Resets the starting point of the reverse iterator.
         *
         * Sets the starting node for the reverse iteration.
         *
         * @param node The new starting point for the reverse iteration.
         */
        void reset(AKNode *node) noexcept;

        /**
         * @brief Checks if the end of the iteration has been reached.
         *
         * Determines whether the reverse iteration has reached the end.
         *
         * @return True if the end has been reached, false otherwise.
         */
        bool done() const noexcept { return m_done; }

        /**
         * @brief Advances the iterator to the next node.
         *
         * Moves the iterator to the next node in the reverse order.
         */
        void next() noexcept;

        /**
         * @brief Jumps to a specified node.
         *
         * More efficient than reset, but assumes the node has the same root as the current node.
         * If not, a segmentation fault may occur.
         *
         * @warning Use with caution, as it assumes the node shares the same root as the current node.
         *
         * @param node The node to jump to.
         */
        void jumpTo(AKNode *node) noexcept;

        /**
         * @brief Gets the root node.
         *
         * Always returns the root node of the tree.
         *
         * @return A pointer to the root node.
         */
        AKNode *end() const noexcept { return m_end; }

        /**
         * @brief Gets the current node.
         *
         * Returns the current node being pointed to by the iterator.
         *
         * @return A pointer to the current node.
         */
        AKNode *node() const noexcept { return m_node; }
    private:
        AKNode *m_node, *m_end;
        bool m_done;
    };

    enum Caps : UInt32
    {
        Render              = 1 << 0,
        Bake                = 1 << 1,
        Scene               = 1 << 2,
        BackgroundEffect    = 1 << 3,
    };
    AKNode() noexcept { theme(); }
    AKCLASS_NO_COPY(AKNode)
    virtual ~AKNode();

    typedef UInt32 Change;

    enum Changes : Change
    {
        CHLayout,
        CHLayoutPos,
        CHLayoutSize,
        CHLayoutScale,
        CHParent,
        CHChildrenClipping,
        CHLast
    };
    void addChange(Change change) noexcept;

    /**
     * @brief Marks all intersected targets as dirty.
     *
     * This function triggers the AK::AKSceneTarget::onMarkedDirty() signal
     * for all intersected targets, which, depending on the implementation,
     * may lead to a window or screen repaint.
     *
     * @note The addChange() function automatically calls this function internally.
     */
    void repaint() noexcept;

    const AKChanges &changes() const noexcept;

    void setVisible(bool visible) noexcept
    {
        if (visible == this->visible())
            return;

        layout().setDisplay(visible ? YGDisplayFlex : YGDisplayNone);

        if (!visible)
            damageTargetsAndPropagate();
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

    /* Insert at the end, nullptr unsets */
    void setParent(AKNode *parent) noexcept;
    AKNode *parent() const noexcept { return m_parent; }
    AKNode *topmostParent() const noexcept;
    AKNode *bottommostRightChild() const noexcept;
    AKNode *bottommostLeftChild() const noexcept;
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

    //AKSceneTarget *currentTarget() const noexcept;

    const std::unordered_set<AKSceneTarget*> &intersectedTargets() const noexcept { return m_intersectedTargets; }
    bool childrenClippingEnabled() const noexcept { return m_flags.check(ChildrenClipping); }

    void enableChildrenClipping(bool enable) noexcept
    {
        if (childrenClippingEnabled() == enable)
            return;

        m_flags.setFlag(ChildrenClipping, enable);
        addChange(CHChildrenClipping);
    }

    void setInputRegion(SkRegion *region) noexcept { m_inputRegion = region ? std::make_unique<SkRegion>(*region) : nullptr; }
    SkRegion *inputRegion() const noexcept { return m_inputRegion.get(); }
    bool insideLastTarget() const noexcept { return m_flags.check(InsideLastTarget); }
    bool renderedOnLastTarget() const noexcept { return m_flags.check(RenderedOnLastTarget); }
    AKSceneTarget *currentTarget() const noexcept;
    /* Layout */

    /* Relative to the closest subscene */
    const SkIRect sceneRect() const noexcept { return m_rect;}

    /* Relative to the root node */
    const SkIRect globalRect() const noexcept { return m_globalRect; }
    const AKLayout &layout() const noexcept { return m_layout; }
    Int32 scale() const noexcept { return m_scale; }
    AKLayout &layout() noexcept { return m_layout; }

    bool isPointerOver() const noexcept;
    void setKeyboardFocus(bool focused) noexcept;
    bool hasKeyboardFocus() const noexcept;

    /**
     * @brief Sets the keyboard focusable state for the object.
     *
     * By default, nodes are not keyboard focusable. This method enables or disables the ability for the object to gain keyboard focus.
     *
     * @param enabled `true` to make focusable, `false` otherwise.
     */
    void setKeyboardFocusable(bool enabled) noexcept;

    /**
     * @brief Checks if the object is keyboard focusable.
     *
     * Determines if keyboard focus can be assigned to the object.
     * This hint is used by functionalities like AKKeyboard::nextFocusableNode() and implementations such
     * as passing focus to the next text field on tab press.
     *
     * @see setKeyboardFocusable()
     *
     * @return `true` if the object is keyboard focusable, `false` otherwise.
     */
    bool isKeyboardFocusable() const noexcept
    {
        return m_flags.check(KeyboardFocusable);
    }

    /* Effects */

    const std::unordered_set<AKBackgroundEffect*> &backgroundEffects() const noexcept { return m_backgroundEffects; }
    void addBackgroundEffect(AKBackgroundEffect *backgroundEffect) noexcept;
    void removeBackgroundEffect(AKBackgroundEffect *backgroundEffect) noexcept;

    /* Triggered before the scene starts rendering and
     * after rect() and globalRect() are calculated */
    virtual void onSceneBegin() {}

    /**
     * @brief Reactive Region.
     *
     * This is a special region that causes damage to the scene
     * only when overlay or background damage from other nodes intersects with it.
     *
     * Relative to the node.
     */
    AKBackgroundDamageTracker bdt;

    UInt64 userFlags { 0 };
    void *userData { nullptr };

    /**
     * @brief Retrieves the topmost scene.
     *
     * Represents the topmost scene (not subscene).
     *
     * @returns `nullptr` if the node is not the root node of a scene or is not a
     * descendant of a root node.
     */
    AKScene *scene() const noexcept { return m_scene; }

    /**
     * @brief Retrieves the closest parent subscene.
     *
     * @return `nullptr` if the node is not the root node or child of any subscene.
     */
    AKSubScene *subScene() const noexcept { return m_subScene; }

    /**
     * @brief Retrieves the root node of the current scene.
     *
     * @note Returns a pointer to itself if the node is the root node of a scene
     *       (not a subscene).
     *
     * @return `nullptr` if the node is not a child of any scene.
     */
    AKNode *root() const noexcept;

    bool activated() const noexcept;

    // TODO: Add if macro for when Marco is disabled
    MSurface *window() const noexcept;

    AKCursor cursor() const noexcept { return m_cursor; }
    void setCursor(AKCursor cursor) { m_cursor = cursor; }

    AKSignal<const AKLayoutEvent &> onLayoutChanged;

protected:
    bool event(const AKEvent &event) override;
    virtual void layoutEvent(const AKLayoutEvent &event);
    virtual void windowStateEvent(const AKWindowStateEvent &event);
    virtual void pointerEnterEvent(const AKPointerEnterEvent &event);
    virtual void pointerMoveEvent(const AKPointerMoveEvent &event);
    virtual void pointerButtonEvent(const AKPointerButtonEvent &event);
    virtual void pointerScrollEvent(const AKPointerScrollEvent &event);
    virtual void pointerLeaveEvent(const AKPointerLeaveEvent &event);
    virtual void keyboardEnterEvent(const AKKeyboardEnterEvent &event);
    virtual void keyboardKeyEvent(const AKKeyboardKeyEvent &event);
    virtual void keyboardLeaveEvent(const AKKeyboardLeaveEvent &event);
private:
    friend class AKBackgroundEffect;
    friend class AKRenderable;
    friend class AKBakeable;
    friend class AKSubScene;
    friend class AKContainer;
    friend class AKSceneTarget;
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
        ChildrenNeedPosUpdate       = 1 << 9,
        ChildrenNeedScaleUpdate     = 1 << 10,
        Skip                        = 1 << 11,
        KeyboardFocusable           = 1 << 12
    };

    struct TargetData : public AKObject
    {
        TargetData() noexcept { changes.set(); }
        AKSceneTarget *target { nullptr };
        size_t targetLink;
        SkRegion prevLocalClip; // Rel to root
        SkIRect prevLocalRect;
        SkRegion clientDamage,
            opaque, translucent,
            opaqueOverlay, invisble;
        AKChanges changes;
        bool visible { true };
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
    bool damageTargets() noexcept;
    void damageTargetsAndPropagate() noexcept;
    AKNode *topmostInvisibleParent() const noexcept;

    AKBitset<Flags> m_flags { 0 };
    SkIRect m_globalRect { 0, 0, 0, 0 };
    SkIRect m_rect;
    UInt32 m_caps { 0 };
    AKLayout m_layout { *this };
    AKWeak<TargetData> t;
    Int32 m_scale { 1 };
    AKWeak<AKScene> m_scene;
    AKWeak<AKSubScene> m_subScene;
    std::unordered_set<AKBackgroundEffect*> m_backgroundEffects;
    AKNode *m_parent { nullptr };
    std::vector<AKNode*> m_children;
    size_t m_parentLinkIndex;
    std::unique_ptr<SkRegion> m_inputRegion;
    std::unordered_set<AKSceneTarget*> m_intersectedTargets;
    mutable std::unordered_map<AKSceneTarget*, TargetData> m_targets;
    AKCursor m_cursor { AKCursor::Default };
    std::vector<AKWeak<AKBackgroundDamageTracker>> m_overlayBdts;
};

/**
 * @}
 */

#endif // AKNODE_H
