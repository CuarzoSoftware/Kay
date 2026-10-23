#ifndef CZ_AKNODE_H
#define CZ_AKNODE_H

#include <CZ/AK/AKBackgroundDamageTracker.h>
#include <CZ/AK/AKObject.h>
#include <CZ/AK/AKLayout.h>
#include <CZ/AK/AKChanges.h>

#include <CZ/Core/CZWeak.h>
#include <CZ/Core/CZBitset.h>
#include <CZ/Core/CZCursorShape.h>
#include <CZ/Core/CZColorScheme.h>

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

    /**
     * @brief Constructs a node without a parent.
     */
    AKNode() noexcept;
    virtual ~AKNode();

    /**
     * @brief Rendering roles a node can advertise.
     *
     * Set by the role base classes (AKContainer, AKRenderable, AKBakeable,
     * AKSubScene, AKBackgroundEffect) and queried with caps().
     */
    enum Cap
    {
        ContainerBit            = static_cast<UInt8>(1) << 0, ///< The node is an AKContainer.
        RenderableBit           = static_cast<UInt8>(1) << 1, ///< The node is an AKRenderable.
        BakeableBit             = static_cast<UInt8>(1) << 2, ///< The node is an AKBakeable.
        SubSceneBit             = static_cast<UInt8>(1) << 3, ///< The node is an AKSubScene.
        BackgroundEffectBit     = static_cast<UInt8>(1) << 4, ///< The node is an AKBackgroundEffect.
    };

    /**
     * @brief The rendering roles of this node.
     */
    CZBitset<Cap> caps() const noexcept { return m_caps; }

    /**
     * @brief Index of a bit within AKChanges (see Changes).
     */
    typedef UInt32 Change;

    /**
     * @brief Changes tracked by the base AKNode class.
     *
     * Subclasses extend this enum starting at CHLast. See AKChanges for how
     * changes are recorded and consumed.
     */
    enum Changes : Change
    {
        CHLayout,           ///< A layout property changed.
        CHLayoutPos,        ///< The calculated position changed.
        CHLayoutSize,       ///< The calculated size changed.
        CHLayoutScale,      ///< The scale factor changed (see scale()).
        CHParent,           ///< The parent changed.
        CHChildrenClipping, ///< Children clipping was toggled (see enableChildrenClipping()).
        CHColorScheme,      ///< The color scheme changed (see colorScheme()).
        CHLast              ///< Marker for subclass chaining.
    };

    /**
     * @brief Records a property change.
     *
     * The change is stored for each AKTarget the node has been presented on, and all
     * currently intersected targets are marked dirty (see repaint()). The accumulated
     * changes are consumed during the next AKScene::render().
     */
    void addChange(Change change) noexcept;

    /**
     * @brief Accumulated changes for the target currently being rendered.
     *
     * Only meaningful while the scene is rendering (e.g. within onSceneBegin(),
     * AKBakeable::bakeEvent() or AKRenderable::renderEvent()).
     */
    const AKChanges &changes() const noexcept;

    /**
     * @brief Interaction capabilities that embedders may honor.
     */
    enum UserCaps
    {
        UCWindowMove = 1 << 0, ///< Dragging the node should move the window (e.g. a window-drag area).
    };

    /**
     * @brief Interaction capability hints for this node.
     */
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

    /**
     * @brief Shows or hides the node (sets the layout display mode to YGDisplayNone).
     *
     * Hidden nodes and their children are skipped during layout, rendering and input.
     */
    void setVisible(bool visible) noexcept;

    /**
     * @brief Whether the node is visible (see setVisible()).
     */
    bool visible() const noexcept { return layout().display() != YGDisplayNone; }

    /**
     * @brief The node that actually receives children (like Vue's default slot).
     *
     * Composite components may redirect children to an internal sub-node via setSlot().
     * Returns `this` unless a slot was set.
     */
    const AKNode *slot() const noexcept { return m_slot ? m_slot.get() : this; }

    /** @brief The node that actually receives children (see the const overload). */
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

    /**
     * @brief Sets the color scheme (light/dark) for this node and its descendants.
     *
     * The scheme propagates to children until a child sets its own.
     * Passing CZColorScheme::Unknown is the same as passing Light.
     */
    void setColorScheme(CZColorScheme scheme) noexcept;

    /**
     * @brief The current color scheme (never Unknown).
     */
    CZColorScheme colorScheme() const noexcept { return m_colorScheme; }

    /**
     * @brief The parent node, or `nullptr`.
     */
    AKNode *parent() const noexcept { return m_parent; }

    /**
     * @brief The furthest ancestor, or `nullptr` if the node has no parent.
     */
    AKNode *topmostParent() const noexcept;

    /**
     * @brief The deepest last-child descendant (topmost node of the subtree in z-order).
     */
    AKNode *bottommostRightChild() const noexcept;

    /**
     * @brief The deepest first-child descendant (bottom-most node of the subtree in z-order).
     */
    AKNode *bottommostLeftChild() const noexcept;

    /**
     * @brief The next sibling (rendered above this node), or `nullptr` if this is the last child.
     */
    AKNode *next() const noexcept { return m_parent && m_parent->m_children.back() != this ? m_parent->m_children[m_parentLink + 1] : nullptr; }

    /**
     * @brief The previous sibling (rendered below this node), or `nullptr` if this is the first child.
     */
    AKNode *prev() const noexcept { return m_parent && m_parentLink > 0 ? m_parent->m_children[m_parentLink - 1] : nullptr; }

    /**
     * @brief Checks if this node is a descendant of the given node.
     */
    bool isSubchildOf(const AKNode *node) const noexcept { return (node && parent()) && (parent() == node || parent()->isSubchildOf(node)); }

    /**
     * @brief The child nodes, ordered bottom to top.
     *
     * @param ignoreSlot If `false` (default), returns the children of slot() (see setParent()).
     */
    const std::vector<AKNode*> &children(bool ignoreSlot = false) const noexcept { return ignoreSlot ? m_children : slot()->children(true); }

    /**
     * @brief Whether this node is the root node of an AKScene (see AKScene::setRoot()).
     */
    bool isRoot() const noexcept { return m_flags.has(IsRoot); }

    /**
     * @brief Targets whose viewport currently intersects this node (excluding AKSubScene targets).
     */
    const std::unordered_set<AKTarget*> &intersectedTargets() const noexcept { return m_intersectedTargets; }

    /**
     * @brief Whether children are clipped to this node's bounds (see enableChildrenClipping()).
     */
    bool childrenClippingEnabled() const noexcept { return m_flags.has(ChildrenClipping); }

    /**
     * @brief Toggles clipping of child nodes to this node's bounds. Disabled by default.
     */
    void enableChildrenClipping(bool enable) noexcept;

    /**
     * @brief Sets the region (in node-local coordinates) that receives pointer input.
     *
     * Passing `nullptr` (default) makes the entire node receive input.
     */
    void setInputRegion(const SkRegion *region) noexcept { m_inputRegion = region ? std::make_unique<SkRegion>(*region) : nullptr; }

    /**
     * @brief The input region, or `nullptr` if the entire node receives input.
     */
    const SkRegion *inputRegion() const noexcept { return m_inputRegion.get(); }

    /**
     * @brief Whether the node intersected the viewport during the last AKScene::render().
     */
    bool insideLastTarget() const noexcept { return m_flags.has(InsideLastTarget); }

    /**
     * @brief Whether the node was actually rendered (not fully occluded/clipped) during the last AKScene::render().
     */
    bool renderedOnLastTarget() const noexcept { return m_flags.has(RenderedOnLastTarget); }

    /**
     * @brief Clears the renderedOnLastTarget() flag.
     */
    void clearRenderedOnLastTarget() noexcept { m_flags.remove(RenderedOnLastTarget); }

    /**
     * @brief The target currently being rendered by the scene, or `nullptr` outside AKScene::render().
     */
    AKTarget *currentTarget() const noexcept;

    /* Layout */

    /**
     * @brief The Flexbox (Yoga) layout of the node.
     */
    const AKLayout &layout() const noexcept { return m_layout; }

    /** @brief The Flexbox (Yoga) layout of the node. */
    AKLayout &layout() noexcept { return m_layout; }

    /**
     * @brief The node rect in world coordinates, calculated during AKScene::render().
     */
    SkIRect worldRect() const noexcept { return m_worldRect; }

    /**
     * @brief The node rect relative to the closest AKSubScene viewport, otherwise the AKScene viewport.
     */
    SkIRect sceneRect() const noexcept { return m_sceneRect; }

    /**
     * @brief The greatest scale factor among the intersected targets (see AKTarget::bakedNodesScale()).
     */
    Int32 scale() const noexcept { return m_scale; }

    /**
     * @brief Checks whether AKPointer::pos() is currently within worldRect().
     *
     * @note This may return `true` even if the node is visually obscured by
     *       overlay nodes.
     */
    bool isPointerOver() const noexcept;

    /**
     * @brief Checks whether this node currently has pointer focus.
     *
     * Pointer focus is assigned by the parent scene when handling pointer events
     * to the first node whose input region intersects the pointer position.
     * The node with direct focus can be queried via `scene()->pointerFocus()`.
     *
     * Focus is also propagated upward through the parent hierarchy. Therefore,
     * this method may return `true` even if:
     *   - `this != scene()->pointerFocus()`
     *   - `isPointerOver()` returns `false`.
     */
    bool hasPointerFocus() const noexcept;

    /**
     * @brief Requests or releases keyboard focus within the parent scene.
     */
    void setKeyboardFocus(bool focused) noexcept;

    /**
     * @brief Whether this node currently has keyboard focus (see AKScene::keyboardFocus()).
     */
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

    /**
     * @brief Background effects currently attached to this node.
     */
    const std::unordered_set<AKBackgroundEffect*> &backgroundEffects() const noexcept { return m_backgroundEffects; }

    /**
     * @brief Attaches a background effect, rendered behind this node (see AKBackgroundEffect).
     */
    void addBackgroundEffect(AKBackgroundEffect *backgroundEffect) noexcept;

    /**
     * @brief Detaches a background effect.
     */
    void removeBackgroundEffect(AKBackgroundEffect *backgroundEffect) noexcept;

    /**
     * @brief Background damage tracker
     *
     * @see AKBackgroundDamageTracker
     */
    AKBackgroundDamageTracker bdt { *this };

    /**
     * @brief Free-use flags for the application (ignored by Kay).
     */
    UInt64 userFlags { 0 };

    /**
     * @brief Free-use pointer for the application (ignored by Kay).
     */
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

    /**
     * @brief The Marco window (MSurface) displaying the parent scene, or `nullptr` (e.g. outside Marco).
     */
    MSurface *parentWindow() const noexcept;

    /**
     * @brief Whether the parent scene's window is activated (see AKScene::windowState()).
     */
    bool activated() const noexcept;

    /**
     * @brief Returns the current cursor shape.
     *
     * Defaults to CZCursorShape::Default.
     *
     * @see setCursor()
     */
    std::optional<CZCursorShape> cursor() const noexcept { return m_cursor; }

    /**
     * @brief Sets the cursor shape for this node.
     *
     * The assigned shape will be shown while the node has pointer focus.
     *
     * @param cursor The cursor shape to use, or std::nullopt to hide the cursor.
     */
    void setCursor(std::optional<CZCursorShape> cursor) { m_cursor = cursor; }


    /**
     * @brief Calculates the bounds of the visible, non-absolute children plus this node's padding.
     */
    void innerBounds(SkRect *out) noexcept;

    /**
     * @brief Recursively collects all nodes of type `T` within the given subtree (including `root`).
     */
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

    /** @brief Casts the node to AKContainer, or `nullptr` if it lacks the role (see caps()). */
    AKContainer         *asContainer() noexcept;
    /** @brief Casts the node to AKRenderable, or `nullptr` if it lacks the role (see caps()). */
    AKRenderable        *asRenderable() noexcept;
    /** @brief Casts the node to AKBackgroundEffect, or `nullptr` if it lacks the role (see caps()). */
    AKBackgroundEffect  *asBackgroundEffect() noexcept;
    /** @brief Casts the node to AKBakeable, or `nullptr` if it lacks the role (see caps()). */
    AKBakeable          *asBakeable() noexcept;
    /** @brief Casts the node to AKSubScene, or `nullptr` if it lacks the role (see caps()). */
    AKSubScene          *asSubScene() noexcept;

    /**
     * @brief Emitted after the node's layout changed (position, size or scale), carrying the triggering event.
     */
    CZSignal<const CZLayoutEvent &> onLayoutChanged;

    /**
     * @brief Emitted on a node when a child is added to it.
     *
     * @param added The newly added child.
     * @param prevSibling The child now preceding it (nullptr if it became the first child).
     *
     * @note Not emitted while the parent itself is being destroyed. A child that is being destroyed
     *       still triggers onChildRemoved, but a dynamic_cast to its concrete type will fail (its
     *       derived part is already gone), which is the safe way to filter it.
     */
    CZSignal<AKNode* /*added*/, AKNode* /*prevSibling*/> onChildAdded;

    /**
     * @brief Emitted on a node when a child is removed from it (reparented or destroyed).
     *
     * @param removed The removed child.
     */
    CZSignal<AKNode* /*removed*/> onChildRemoved;

    /** @brief Emitted when the node's color scheme changes (see setColorScheme). Carries the new scheme. */
    CZSignal<CZColorScheme> onColorSchemeChanged;

protected:
    /**
     * @brief Redirects future children to an internal sub-node (see slot()).
     */
    void setSlot(AKNode *slot) noexcept;

    /**
     * @brief Dispatches incoming events to the specific handlers below.
     */
    bool event(const CZEvent &event) noexcept override;

    /**
     * @brief Triggered during AKScene::render(), before rendering starts and after
     *        worldRect() is calculated (after layoutEvent()).
     *
     * The usual place to react to accumulated changes() that affect assets, damage
     * or the opaque region.
     */
    virtual void onSceneBegin() {}

    /**
     * @brief The node's layout changed (position, size or scale).
     */
    virtual void layoutEvent(const CZLayoutEvent &event);

    /**
     * @brief The node's color scheme changed (see setColorScheme()).
     */
    virtual void colorSchemeEvent(const CZColorSchemeEvent &event);

    /**
     * @brief The window state of the parent scene changed (e.g. activated).
     */
    virtual void windowStateEvent(const CZWindowStateEvent &event);

    /** @brief The pointer entered the node (see hasPointerFocus()). */
    virtual void pointerEnterEvent(const CZPointerEnterEvent &event);
    /** @brief The pointer moved while the node has pointer focus. */
    virtual void pointerMoveEvent(const CZPointerMoveEvent &event);
    /** @brief A pointer button was pressed/released while the node has pointer focus. */
    virtual void pointerButtonEvent(const CZPointerButtonEvent &event);
    /** @brief A scroll event was received while the node has pointer focus. */
    virtual void pointerScrollEvent(const CZPointerScrollEvent &event);
    /** @brief The pointer left the node. */
    virtual void pointerLeaveEvent(const CZPointerLeaveEvent &event);
    /** @brief The node gained keyboard focus (see setKeyboardFocus()). */
    virtual void keyboardEnterEvent(const CZKeyboardEnterEvent &event);
    /** @brief A key was pressed/released while the node has keyboard focus. */
    virtual void keyboardKeyEvent(const CZKeyboardKeyEvent &event);
    /** @brief The node lost keyboard focus. */
    virtual void keyboardLeaveEvent(const CZKeyboardLeaveEvent &event);
    /** @brief The node was added to or removed from a scene (see AKSceneChangedEvent). */
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
    CZColorScheme m_colorScheme { CZColorScheme::Light };

    // Set by the parent AKScene or AKSubScene during AKScene::render() (just to prevent frequent m_targets lookups)
    CZWeak<TargetData> tData;

    // Optional. Used to redirect children to a sub node (like Vue's default slot)
    CZWeak<AKNode> m_slot;

    AKNode *m_parent {};
    size_t m_parentLink {}; // Index of parent->children()
    std::vector<AKNode*> m_children;
    bool m_beingDestroyed { false }; // Suppresses child signals while this node is destroyed.

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
    std::optional<CZCursorShape> m_cursor { CZCursorShape::Default };
    std::vector<CZWeak<AKBackgroundDamageTracker>> m_overlayBdts;
};

/** @} */

#endif // CZ_AKNODE_H
