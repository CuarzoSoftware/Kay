#ifndef CZ_AKNODE_H
#define CZ_AKNODE_H

#include <CZ/AK/AKBackgroundDamageTracker.h>
#include <CZ/AK/AKCursor.h>
#include <CZ/AK/AKObject.h>
#include <CZ/AK/AKLayout.h>
#include <CZ/AK/AKChanges.h>

#include <CZ/Core/CZWeak.h>
#include <CZ/Core/CZBitset.h>

#include <CZ/skia/core/SkImage.h>
#include <CZ/skia/core/SkSurface.h>
#include <CZ/skia/core/SkRegion.h>

#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <vector>

namespace CZ
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
class CZ::AKNode : public AKObject
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

    AKNode() noexcept { theme(); }
    virtual ~AKNode();

    enum Cap
    {
        ContainerBit            = static_cast<UInt8>(1) << 0,
        RenderableBit           = static_cast<UInt8>(1) << 1,
        BakeableBit             = static_cast<UInt8>(1) << 2,
        SubSceneBit             = static_cast<UInt8>(1) << 3,
        BackgroundEffectBit     = static_cast<UInt8>(1) << 4,
    };

    CZBitset<Cap> caps() const noexcept { return m_caps; }

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
    const AKChanges &changes() const noexcept;

    enum UserCaps
    {
        UCWindowMove = 1 << 0,
    };

    CZBitset<UserCaps> userCaps;

    /**
     * @brief Marks all intersected targets as dirty.
     *
     * This function triggers the CZ::AKTarget::onMarkedDirty() signal
     * for all intersected targets, which, depending on the implementation,
     * may lead to a window or screen repaint.
     *
     * @note The addChange() function automatically calls this function internally.
     */
    void repaint() noexcept;

    void setVisible(bool visible) noexcept;
    bool visible() const noexcept { return layout().display() != YGDisplayNone; }

    const AKNode *slot() const noexcept { return m_slot ? m_slot.get() : this; }
    AKNode *slot() noexcept { return m_slot ? m_slot.get() : this; }

    /**
     * @brief Inserts the node at the end of the parent's children.
     *
     * Passing nullptr unsets the parent.
     * If the parent is equal to the current parent, the node is re-inserted at the end.
     */
    void setParent(AKNode *parent, bool ignoreSlot = false) noexcept;

    /**
     * @brief Inserts the node before (below) the other node.
     *
     * The new parent is set to other->parent().
     * If other has no parent, the parent is unset.
     */
    void insertBefore(AKNode *other) noexcept;

    /**
     * @brief Inserts the node after (above) the other node.
     *
     * The new parent is set to other->parent().
     * If other has no parent, the parent is unset.
     */
    void insertAfter(AKNode *other) noexcept;

    AKNode *parent() const noexcept { return m_parent; }
    AKNode *topmostParent() const noexcept;
    AKNode *bottommostRightChild() const noexcept;
    AKNode *bottommostLeftChild() const noexcept;

    // Sibling nodes
    AKNode *next() const noexcept { return m_parent && m_parent->m_children.back() != this ? m_parent->m_children[m_parentLink + 1] : nullptr; }
    AKNode *prev() const noexcept { return m_parent && m_parentLink > 0 ? m_parent->m_children[m_parentLink - 1] : nullptr; }

    bool isSubchildOf(AKNode *node) const noexcept { return (node && parent()) && (parent() == node || parent()->isSubchildOf(node)); }

    const std::vector<AKNode*> &children(bool ignoreSlot = false) const noexcept { return ignoreSlot ? m_children : slot()->children(true); }

    bool isRoot() const noexcept { return m_flags.has(IsRoot); }

    const std::unordered_set<AKTarget*> &intersectedTargets() const noexcept { return m_intersectedTargets; }

    bool childrenClippingEnabled() const noexcept { return m_flags.has(ChildrenClipping); }
    void enableChildrenClipping(bool enable) noexcept;

    void setInputRegion(const SkRegion *region) noexcept { m_inputRegion = region ? std::make_unique<SkRegion>(*region) : nullptr; }
    const SkRegion *inputRegion() const noexcept { return m_inputRegion.get(); }
    bool insideLastTarget() const noexcept { return m_flags.has(InsideLastTarget); }
    bool renderedOnLastTarget() const noexcept { return m_flags.has(RenderedOnLastTarget); }
    void clearRenderedOnLastTarget() noexcept { m_flags.remove(RenderedOnLastTarget); }
    AKTarget *currentTarget() const noexcept;

    /* Layout */

    const AKLayout &layout() const noexcept { return m_layout; }
    AKLayout &layout() noexcept { return m_layout; }
    SkIRect worldRect() const noexcept { return m_worldRect; }
    SkIRect sceneRect() const noexcept { return m_sceneRect; }
    Int32 scale() const noexcept { return m_scale; }

    bool isPointerOver() const noexcept;
    bool hasPointerFocus() const noexcept { return m_flags.has(HasPointerFocus); }
    void enablePointerGrab(bool enabled) noexcept;
    bool pointerGrabEnabled() const noexcept { return m_flags.has(PointerGrab);}
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
        return m_flags.has(KeyboardFocusable);
    }

    /* Effects */

    const std::unordered_set<AKBackgroundEffect*> &backgroundEffects() const noexcept { return m_backgroundEffects; }
    void addBackgroundEffect(AKBackgroundEffect *backgroundEffect) noexcept;
    void removeBackgroundEffect(AKBackgroundEffect *backgroundEffect) noexcept;

    /* Triggered before the scene starts rendering and
     * after rect() and globalRect() are calculated */
    virtual void onSceneBegin() {}

    /**
     * @brief Background damage tracker
     *
     * @see AKBackgroundDamageTracker
     */
    AKBackgroundDamageTracker bdt { *this };

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

    // Padding + non absolute nodes dimensions
    void innerBounds(SkRect *out) noexcept;

    template<class T>
    static void FindNodesWithType(AKNode *root, std::vector<T*> *out) noexcept
    {
        {
            T *found { dynamic_cast<T*>(root) };

            if (found)
                out->emplace_back(found);
        }

        for (AKNode *node : root->children(true))
            FindNodesWithType(node, out);
    }

    // Dynamic cast
    AKContainer         *asContainer() noexcept;
    AKRenderable        *asRenderable() noexcept;
    AKBackgroundEffect  *asBackgroundEffect() noexcept;
    AKBakeable          *asBakeable() noexcept;
    AKSubScene          *asSubScene() noexcept;

    CZSignal<const CZLayoutEvent &> onLayoutChanged;

protected:
    void setSlot(AKNode *slot) noexcept;
    bool event(const CZEvent &event) noexcept override;
    virtual void layoutEvent(const CZLayoutEvent &event);
    virtual void windowStateEvent(const CZWindowStateEvent &event);
    virtual void pointerEnterEvent(const CZPointerEnterEvent &event);
    virtual void pointerMoveEvent(const CZPointerMoveEvent &event);
    virtual void pointerButtonEvent(const CZPointerButtonEvent &event);
    virtual void pointerScrollEvent(const CZPointerScrollEvent &event);
    virtual void pointerLeaveEvent(const CZPointerLeaveEvent &event);
    virtual void keyboardEnterEvent(const CZKeyboardEnterEvent &event);
    virtual void keyboardKeyEvent(const CZKeyboardKeyEvent &event);
    virtual void keyboardLeaveEvent(const CZKeyboardLeaveEvent &event);
    virtual void sceneChangedEvent(const AKSceneChangedEvent &event);
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
        ChildrenNeedPosUpdate       = 1 << 9,
        ChildrenNeedScaleUpdate     = 1 << 10,
        Skip                        = 1 << 11,
        KeyboardFocusable           = 1 << 12
    };

    /* Created by AKScene when the node is presented on the target for the first time */
    struct TargetData : public AKObject
    {
        // Set all changes to true
        TargetData() noexcept { changes.set(); }

        // Set by the scene after creation
        AKTarget *target {};

        // Visible region in the last AKScene::render() call on this target
        SkRegion prevSceneClip;

        // sceneRect value in the last AKScene::render() call on this target
        SkIRect prevSceneRect {};

        SkRegion opaque, translucent, opaqueOverlay, invisble;

        // Accumulated damage since the last AKScene::render() call on this target
        SkRegion damage;

        // Accumulated changes since the last AKScene::render() call on this target
        AKChanges changes {};

        // visible state in the last AKScene::render() call on this target
        bool visible { true };
    };

    AKNode(AKNode *parent = nullptr) noexcept;
    AKNode *closestClipperParent() const noexcept;
    void setScene(AKScene *scene) noexcept;
    void propagateScene(AKScene *scene) noexcept;
    void updateSubScene() noexcept;
    void setParentPrivate(AKNode *parent, bool handleChanges, bool ignoreSlot) noexcept;
    void addFlagsAndPropagate(UInt32 flags) noexcept;
    void removeFlagsAndPropagate(UInt32 flags) noexcept;
    void setFlagsAndPropagateToParents(UInt32 flags, bool set) noexcept;
    bool damageTargets() noexcept;
    void damageTargetsAndPropagate() noexcept;
    AKNode *topmostInvisibleParent() const noexcept;

    // To keep the app alive
    std::shared_ptr<AKApp> m_app;

    CZBitset<Cap> m_caps;

    CZBitset<Flags> m_flags {};

    // Set by the parent AKScene or AKSubScene during AKScene::render() (just to prevent frequent m_targets lookups)
    CZWeak<TargetData> tData;

    // Optional. Used to redirect children to a sub node (like Vue's default slot)
    CZWeak<AKNode> m_slot;

    AKNode *m_parent {};
    size_t m_parentLink {}; // Index of parent->children()
    std::vector<AKNode*> m_children;

    // Yoga layout
    AKLayout m_layout { *this };

    // Rect in world coordinates
    SkIRect m_worldRect {};

    // Relative to the closest parent AKSubScene viewport, otherwise the AKScene viewport
    SkIRect m_sceneRect {};

    // Greatest scale factor among the intersected targets
    Int32 m_scale { 1 };

    // Current scene, nullptr if this is not a root node or descendant of one
    CZWeak<AKScene> m_scene;

    // Closest AKSubScene parent, nullptr if none
    CZWeak<AKSubScene> m_subScene;

    // Attached background effects, TODO: replace with vector to control z-axis
    std::unordered_set<AKBackgroundEffect*> m_backgroundEffects;

    // If nullptr, the entire node receives input
    std::unique_ptr<SkRegion> m_inputRegion;

    // Doesn't include AKSubScene targets
    std::unordered_set<AKTarget*> m_intersectedTargets;

    // Target-specific state, including AKSubScene targets
    mutable std::unordered_map<AKTarget*, TargetData> m_targets;
    AKCursor m_cursor { AKCursor::Default };
    std::vector<CZWeak<AKBackgroundDamageTracker>> m_overlayBdts;
};

#endif // CZ_AKNODE_H
