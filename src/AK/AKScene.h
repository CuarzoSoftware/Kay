#ifndef AKSCENE_H
#define AKSCENE_H

#include <AK/AKWindowState.h>
#include <AK/AKObject.h>
#include <AK/AKTarget.h>
#include <AK/nodes/AKNode.h>
#include <memory.h>
#include <vector>

class AK::AKScene : public AKObject
{
public:
    AKScene() noexcept;
    ~AKScene();
    AKTarget *createTarget() noexcept;
    bool destroyTarget(AKTarget *target);
    bool render(AKTarget *target);

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

    const std::vector<AKTarget*> &targets() const noexcept
    {
        return m_targets;
    }

    bool isSubScene() const noexcept { return m_isSubScene; };
    AKNode *pointerFocus() const noexcept { return m_pointerFocus; };
    AKNode *keyboardFocus() const noexcept { return m_keyboardFocus; };

    AKBitset<AKWindowState> windowState() const noexcept { return m_windowState; }
protected:
    bool event(const AKEvent &event) override;
private:
    friend class AKTarget;
    friend class AKNode;
    friend class AKSubScene;
    const AKEvent *e;
    SkCanvas *c;
    AKTarget *t;
    SkMatrix m_matrix;
    std::vector<AKTarget*> m_targets;
    AKWeak<AKNode> m_root;
    AKWeak<AKNode> m_pointerFocus;
    AKWeak<AKNode> m_keyboardFocus;
    std::shared_ptr<AKPainter> m_painter;
    bool m_isSubScene { false };
    bool m_treeChanged { false };
    bool m_eventWithoutTarget { false };
    AKBitset<AKWindowState> m_windowState { AKActivated };
    void validateTarget(AKTarget *target) noexcept;
    void updateMatrix() noexcept;
    void createOrAssignTargetDataForNode(AKNode *node) noexcept;
    void notifyBegin(AKNode *node);
    void calculateNewDamage(AKNode *node);
    void updateDamageRing() noexcept;
    void renderBackground() noexcept;
    void renderNodes(AKNode *node);
    void handlePointerMoveEvent();
    void handlePointerButtonEvent();
    void handleKeyboardKeyEvent();
    void handleWindowStateEvent();
};

#endif // AKSCENE_H
