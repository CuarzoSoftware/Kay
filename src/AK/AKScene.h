#ifndef AKSCENE_H
#define AKSCENE_H

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
    bool activated() const noexcept { return m_activated; };
    bool event(const AKEvent &event) override;

    AKNode *pointerFocus() const noexcept { return m_pointerFocus; };
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
    std::shared_ptr<AKPainter> m_painter;
    bool m_isSubScene { false };
    bool m_treeChanged { false };
    bool m_eventWithoutTarget { false };
    bool m_activated { true };
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
    void handleStateActivatedEvent();
    void handleStateDeactivatedEvent();
};

#endif // AKSCENE_H
