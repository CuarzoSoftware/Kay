#include <AKTarget.h>
#include <AKNode.h>
#include <cassert>
#include <yoga/Yoga.h>

using namespace AK;

AK::AKNode::AKNode(AKNode *parent) noexcept :
    m_node(YGNodeNew())
{
    allNodes().push_back(this);
    m_allNodesIndex = allNodes().size() - 1;
    setParent(parent);
}

AKNode::~AKNode()
{
    allNodes()[m_allNodesIndex] = allNodes().back();
    allNodes()[m_allNodesIndex]->m_allNodesIndex = m_allNodesIndex;
    allNodes().pop_back();

    for (auto &t : m_targets)
    {
        t.first->m_damage.op(t.second.prevClip, SkRegion::kUnion_Op);
        t.first->m_nodes[t.second.targetLink] = t.first->m_nodes.back();
        t.first->m_nodes.back()->m_targets[t.first].targetLink = t.second.targetLink;
        t.first->m_nodes.pop_back();
    }

    YGNodeFree(m_node);
    setParent(nullptr);
}

void AKNode::setParent(AKNode *parent) noexcept
{
    assert(!parent || (parent != this && !parent->isSubchildOf(this)));

    if (m_parent)
    {
        YGNodeRemoveChild(m_parent->m_node, m_node);
        m_parent->m_children.erase(m_parentLink);
    }

    m_parent = parent;

    if (parent)
    {
        m_nodeIndex = YGNodeGetChildCount(parent->m_node);
        YGNodeInsertChild(parent->m_node, m_node, m_nodeIndex);
        parent->m_children.push_back(this);
        m_parentLink = std::prev(parent->m_children.end());
    }
}

