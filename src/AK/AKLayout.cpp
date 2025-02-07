#include <AK/AKLayout.h>
#include <AK/AKTarget.h>
#include <AK/nodes/AKNode.h>

using namespace AK;

void AKLayout::setDisplay(YGDisplay display) noexcept
{
    const bool turnedVisible { this->display() == YGDisplayNone && display != YGDisplayNone };
    YGNodeStyleSetDisplay(m_node, display);

    if (turnedVisible)
    {
        for (auto &t : m_akNode.m_targets)
        {
            t.second.changes.set(AKNode::Chg_Layout);
            t.first->markDirty();
        }
    }
    else
        checkIsDirty();
}

void AKLayout::checkIsDirty() noexcept
{
    if (YGNodeIsDirty(m_node))
        m_akNode.addChange(AKNode::Chg_Layout);
}

void AKLayout::apply() noexcept
{
    if (m_akNode.parent())
    {
        YGNodeCalculateLayout(
            m_node,
            m_akNode.parent()->globalRect().width(),
            m_akNode.parent()->globalRect().height(),
            YGDirectionInherit);

        applyTree(&m_akNode);
    }
    else
    {
        YGNodeCalculateLayout(
            m_node,
            YGUndefined,
            YGUndefined,
            YGDirectionInherit);

        YGNodeSetHasNewLayout(m_node, false);

        for (AKNode *child : m_akNode.children())
            applyTree(child);
    }
}

void AKLayout::applyTree(AKNode *node)
{
    if (!node->parent()->m_flags.check(AKNode::ChildrenNeedPosUpdate) && !YGNodeGetHasNewLayout(node->layout().m_node))
        return;

    YGNodeSetHasNewLayout(node->layout().m_node, false);

    SkIRect newRect;
    newRect.fLeft = node->parent()->globalRect().x() + SkScalarFloorToInt(node->layout().calculatedLeft());
    newRect.fTop = node->parent()->globalRect().y() + SkScalarFloorToInt(node->layout().calculatedTop());
    newRect.fRight = newRect.fLeft + SkScalarFloorToInt(node->layout().calculatedWidth());
    newRect.fBottom = newRect.fTop + SkScalarFloorToInt(node->layout().calculatedHeight());

    AKBitset<AKNode::LayoutChanges> changes;

    if (newRect.topLeft() != node->m_globalRect.topLeft())
    {
        changes.add(AKNode::LayoutChanges::Pos);
        node->addChange(AKNode::Chg_LayoutPos);
        node->m_flags.add(AKNode::ChildrenNeedPosUpdate);
    }

    if (newRect.size() != node->m_globalRect.size())
    {
        changes.add(AKNode::LayoutChanges::Size);
        node->addChange(AKNode::Chg_LayoutSize);
    }

    if (changes.get() != 0)
    {
        node->m_globalRect = newRect;
        node->signalLayoutChanged.notify(changes);
    }

    for (AKNode *child : node->children())
        applyTree(child);

    node->m_flags.remove(AKNode::ChildrenNeedPosUpdate);
}
