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
    AKScene() noexcept = default;
    AKTarget *createTarget(std::shared_ptr<AKPainter> painter = nullptr) noexcept;
    bool destroyTarget(AKTarget *target);
    void updateLayout() noexcept
    {
        if (root())
            YGNodeCalculateLayout(root()->layout().m_node,
                                  YGUndefined,
                                  YGUndefined,
                                  YGDirectionInherit);
    }
    bool render(AKTarget *target);

    /**
     * @brief Root node.
     *
     * Only the children of the root node are rendered.
     * The root node's bounds do not clip its children, but its layout properties may affect them.
     */
    void setRoot(AKNode *node) noexcept
    {
        if (node == m_root)
            return;

        if (m_root)
            m_root->m_flags.remove(AKNode::IsRoot);

        m_root.reset(node);

        if (m_root && !m_isSubScene)
            m_root->m_flags.add(AKNode::IsRoot);

        for (AKTarget *t : m_targets)
        {
            t->m_needsFullRepaint = true;
            t->markDirty();
        }
    }

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

    void postEvent(const AKEvent &event);
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
    bool m_isSubScene { false };
    bool m_treeChanged { false };
    void validateTarget(AKTarget *target) noexcept;
    void updateMatrix() noexcept;
    void notifyBegin(AKNode *node);
    void calculateNewDamage(AKNode *node);
    void updateDamageRing() noexcept;
    void renderBackground() noexcept;
    void renderNodes(AKNode *node);

    void handlePointerMoveEvent();
    void handlePointerButtonEvent();
};

#endif // AKSCENE_H
