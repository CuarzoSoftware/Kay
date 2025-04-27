#ifndef AKSCENE_H
#define AKSCENE_H

#include <AK/nodes/AKNode.h>
#include <AK/AKWindowState.h>
#include <AK/AKObject.h>
#include <AK/AKSceneTarget.h>
#include <AK/AKTimer.h>
#include <memory.h>
#include <vector>

class AK::AKScene : public AKObject
{
public:
    AKScene() noexcept;
    AKCLASS_NO_COPY(AKScene)
    ~AKScene() = default;
    AKSceneTarget *createTarget() noexcept;
    bool destroyTarget(AKSceneTarget *target);
    bool render(AKSceneTarget *target);

    /**
     * @brief Root node.
     *
     * Only the children of the root node are rendered.
     * The root node's bounds do not clip its children, but its layout properties may affect them.
     */
    void setRoot(AKNode *node) noexcept;
    AKNode *nodeAt(const SkPoint &pos) const noexcept;
    AKNode *root() const noexcept
    {
        return m_root;
    }

    const std::vector<AKSceneTarget*> &targets() const noexcept
    {
        return m_targets;
    }

    bool isSubScene() const noexcept { return m_isSubScene; };
    AKNode *pointerFocus() const noexcept { return m_win->pointerFocus; };
    AKNode *keyboardFocus() const noexcept { return m_win->keyboardFocus; };
    AKNode *nextKeyboardFocusable() const noexcept;
    AKBitset<AKWindowState> windowState() const noexcept { return m_win->windowState; }
protected:
    bool event(const AKEvent &event) override;
private:
    friend class AKSceneTarget;
    friend class AKNode;
    friend class AKSubScene;
    // For AKSubScene
    AKScene(bool) noexcept { m_isSubScene = true; }
    SkCanvas *c;
    AKSceneTarget *t;
    SkMatrix m_matrix;
    std::vector<AKSceneTarget*> m_targets;
    AKWeak<AKNode> m_root;
    std::shared_ptr<AKPainter> m_painter;

    struct Window
    {
        const AKEvent *e;
        AKWeak<AKNode> pointerFocus;
        AKWeak<AKNode> keyboardFocus;
        AKTimer keyDelayTimer, keyRepeatTimer;
        Int64 repeatedKey { -1 };
        AKBitset<AKWindowState> windowState { AKActivated };
    };

    // Not available for sub scenes
    std::unique_ptr<Window> m_win;
    bool m_isSubScene { false };
    bool m_treeChanged { false };
    bool m_eventWithoutTarget { false };
    void addNodeDamage(AKNode &node, const SkRegion &damage) noexcept;
    void validateTarget(AKSceneTarget *target) noexcept;
    void updateMatrix() noexcept;
    void createOrAssignTargetDataForNode(AKNode *node) noexcept;
    void notifyBegin(AKNode *node);
    void calculateNewDamage(AKNode *node);
    void updateDamageRing() noexcept;
    void updateDamageTrackers() noexcept;
    void renderBackground() noexcept;
    void renderBackgroundOnTarget(AKTarget &target, SkRegion &region) noexcept;
    void renderNodes(AKNode *node);
    void renderNodeTranslucentOnTarget(AKRenderable *node, AKTarget &target, SkRegion &region) noexcept;
    void renderNodeOpaqueOnTarget(AKRenderable *node, AKTarget &target, SkRegion &region) noexcept;
    void handlePointerMoveEvent();
    void handlePointerLeaveEvent();
    void handlePointerButtonEvent();
    void handlePointerScrollEvent();
    void handleKeyboardKeyEvent();
    void handleWindowStateEvent();
};

#endif // AKSCENE_H
