#ifndef CZ_AKSCENE_H
#define CZ_AKSCENE_H

#include <CZ/AK/Nodes/AKNode.h>
#include <CZ/AK/AKObject.h>
#include <CZ/AK/AKTarget.h>
#include <CZ/Core/CZWindowState.h>
#include <CZ/Core/CZTimer.h>
#include <memory.h>
#include <vector>

/**
 * @brief Renders a tree of nodes into AKTargets.
 *
 * A scene calculates the layout of its node tree, tracks damage and renders the
 * nodes back-to-front into a given target during render(). It also behaves like a
 * window from the input point of view: it keeps the pointer and keyboard focus and
 * dispatches input events to the right nodes (see @ref events_page "Event System").
 *
 * Applications typically use one scene with a single target per window, while
 * compositors can share one scene across multiple targets (e.g. one per screen).
 * See @ref core_components_page "Core Components".
 *
 * @ingroup AKCore
 */
class CZ::AKScene : public AKObject
{
public:
    /**
     * @brief Creates a scene.
     */
    [[nodiscard]] static std::shared_ptr<AKScene> Make() noexcept;
    ~AKScene();

    /**
     * @brief Whether this is the internal scene of an AKSubScene.
     */
    bool isSubScene() const noexcept { return m_isSubScene; };

    /**
     * @brief Creates a render destination bound to this scene (see AKTarget).
     */
    [[nodiscard]] std::shared_ptr<AKTarget> makeTarget() noexcept;

    /**
     * @brief Targets created by this scene.
     */
    const std::vector<AKTarget*> &targets() const noexcept { return m_targets; }

    /**
     * @brief Renders the node tree into the given target.
     *
     * Calculates the layout (unless AKTarget::layoutOnRender is disabled), dispatches
     * bake/render events, performs damage tracking and fills the target's output regions.
     *
     * @return `false` if the target is invalid (e.g. missing surface or root node).
     */
    bool render(std::shared_ptr<AKTarget> target) noexcept;

    /**
     * @brief The target passed to render(), or `nullptr` outside a render() call.
     */
    std::shared_ptr<AKTarget> currentTarget() const noexcept { return ct; }

    /**
     * @brief Root node.
     *
     * Only the children of the root node are rendered.
     * The root node's bounds do not clip its children, but its layout properties may affect them.
     */
    void setRoot(AKNode *node) noexcept;

    /**
     * @brief The topmost node whose input region contains the given position (in world coordinates), or `nullptr`.
     */
    AKNode *nodeAt(const SkPoint &pos) const noexcept;

    /**
     * @brief The root node (see setRoot()), or `nullptr`.
     */
    AKNode *root() const noexcept { return m_root; }

    /**
     * @brief Redirects all pointer events to the given node (or releases the grab with `nullptr`),
     *        e.g. during drag interactions.
     */
    void setPointerGrab(AKNode *node) noexcept;

    /**
     * @brief The node currently grabbing pointer events, or `nullptr` (see setPointerGrab()).
     */
    AKNode *pointerGrab() const noexcept;

    /**
     * @brief The node with direct pointer focus, or `nullptr` (see AKNode::hasPointerFocus()).
     */
    AKNode *pointerFocus() const noexcept;

    /**
     * @brief The node with keyboard focus, or `nullptr` (see AKNode::setKeyboardFocus()).
     */
    AKNode *keyboardFocus() const noexcept;

    /**
     * @brief The next keyboard-focusable node after the current focus, used for tab navigation
     *        (see AKNode::setKeyboardFocusable()).
     */
    AKNode *nextKeyboardFocusable() const noexcept;

    /**
     * @brief The window state flags last received through a CZWindowStateEvent (e.g. activated).
     */
    CZBitset<CZWindowState> windowState() const noexcept;

    /**
     * @brief The Marco window (MSurface) displaying this scene, or `nullptr` (e.g. outside Marco).
     */
    MSurface *window() const noexcept;

protected:
    bool event(const CZEvent &event) noexcept override;
private:
    friend class AKTarget;
    friend class AKNode;
    friend class AKSubScene;
    friend class MSurface;
    static std::shared_ptr<AKScene> MakeSubScene() noexcept;
    AKScene(bool isSubScene) noexcept;
    bool validateTarget(std::shared_ptr<AKTarget> target) noexcept;
    void layoutTree() noexcept;
    void setupInvisibleRegion() noexcept;
    void treeNotifyBegin() noexcept;
    void notifyBegin(AKNode *node);
    void calculateTreeDamage() noexcept;
    void renderBackground() noexcept;
    void renderTree() noexcept;
    void resetTarget() noexcept;
    std::weak_ptr<AKScene> m_self;
    std::shared_ptr<AKTarget> ct;
    std::shared_ptr<RPass> pass;
    std::vector<AKTarget*> m_targets;
    CZWeak<AKNode> m_root;

    struct Window
    {
        CZWeak<MSurface> window;
        CZWeak<AKNode> pointerGrab;
        CZWeak<AKNode> pointerFocus;
        CZWeak<AKNode> keyboardFocus;
        CZTimer keyDelayTimer, keyRepeatTimer;
        Int64 repeatedKey { -1 };
        CZBitset<CZWindowState> windowState { CZWinActivated };
    };

    // Not available for sub scenes
    std::unique_ptr<Window> m_win;
    bool m_isSubScene { false };
    bool m_treeChanged { false };
    bool m_eventWithoutTarget { false };
    void addNodeDamage(AKNode &node, const SkRegion &damage) noexcept;
    void createOrAssignTargetDataForNode(AKNode *node) noexcept;
    void calculateNewDamage(AKNode *node);
    void updateDamageRing() noexcept;
    void updateDamageTrackers() noexcept;
    void backgroundPass(std::shared_ptr<RPass> pass, SkRegion &region) noexcept;
    void renderNodes(AKNode *node);
    void nodeTranslucentPass(AKRenderable *node, std::shared_ptr<RPass> pass, SkRegion &region) noexcept;
    void nodeOpaquePass(AKRenderable *node, std::shared_ptr<RPass> pass, SkRegion &region) noexcept;

    void pointerEnterEvent(const CZPointerEnterEvent &e) noexcept;
    void pointerMoveEvent(const CZPointerMoveEvent &e) noexcept;
    void pointerButtonEvent(const CZPointerButtonEvent &e) noexcept;
    void pointerScrollEvent(const CZPointerScrollEvent &e) noexcept;
    void pointerLeaveEvent(const CZPointerLeaveEvent &e) noexcept;

    void keyboardEnterEvent(const CZKeyboardEnterEvent &e) noexcept;
    void keyboardKeyEvent(const CZKeyboardKeyEvent &e) noexcept;
    void keyboardLeaveEvent(const CZKeyboardLeaveEvent &e) noexcept;

    void windowStateEvent(const CZWindowStateEvent &e) noexcept;
    void colorSchemeEvent(const CZColorSchemeEvent &e) noexcept;

    static void SetPassParamsFromRenderable(std::shared_ptr<RPass> pass, AKRenderable *rend, bool opaque) noexcept;
};

#endif // CZ_AKSCENE_H
