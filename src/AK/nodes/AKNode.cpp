#include <include/core/SkColorSpace.h>
#include <include/gpu/ganesh/SkImageGanesh.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>

#include <AK/AKTarget.h>
#include <AK/nodes/AKNode.h>
#include <AK/AKSurface.h>
#include <GL/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <cassert>
#include <yoga/Yoga.h>

using namespace AK;

AK::AKNode::AKNode(AKNode *parent) noexcept
{
    setParent(parent);
}

AKNode::~AKNode()
{
    for (auto &t : m_targets)
    {
        t.second.target->m_damage.op(t.second.prevLocalClip, SkRegion::kUnion_Op);
        t.second.target->m_nodes[t.second.targetLink] = t.second.target->m_nodes.back();
        t.second.target->m_nodes.back()->m_targets[t.first].targetLink = t.second.targetLink;
        t.second.target->m_nodes.pop_back();
    }

    setParent(nullptr);
}

AK::AKNode *AKNode::closestClipperParent() const noexcept
{
    assert(parent() != nullptr);

    if (parent()->clipsChildren() || parent() == t->target->root)
        return parent();

    return parent()->closestClipperParent();
}

void AKNode::setParent(AKNode *parent) noexcept
{
    assert(!parent || (parent != this && !parent->isSubchildOf(this)));

    if (m_parent)
    {
        YGNodeRemoveChild(m_parent->layout().m_node, layout().m_node);
        auto next = m_parent->m_children.erase(m_parent->m_children.begin() + m_parentLink);
        for (; next != m_parent->m_children.end(); next++) (*next)->m_parentLink--;
    }

    m_parent = parent;

    if (parent)
    {
        m_parentLink = YGNodeGetChildCount(parent->layout().m_node);
        YGNodeInsertChild(parent->layout().m_node, layout().m_node, m_parentLink);
        parent->m_children.push_back(this);
    }
}

void AKNode::insertBefore(AKNode *other) noexcept
{
    assert(!other || !other->isSubchildOf(this));

    if (other == this)
        return;

    if (other)
    {
        if (other->parent())
        {
            setParent(nullptr);
            m_parent = other->parent();
            m_parentLink = other->m_parentLink;
            YGNodeInsertChild(m_parent->layout().m_node, layout().m_node, m_parentLink);
            auto next = m_parent->m_children.insert(m_parent->m_children.begin() + m_parentLink, this) + 1;
            for (; next != m_parent->m_children.end(); next++) (*next)->m_parentLink++;
        }
        else
        {
            setParent(nullptr);
        }
    }
    else if (parent())
    {
        setParent(parent());
    }
}

void AKNode::insertAfter(AKNode *other) noexcept
{
    assert(!other || !other->isSubchildOf(this));

    if (other == this)
        return;

    if (other)
    {
        if (other->parent())
        {
            setParent(nullptr);

            if (other->parent()->children().back() == other)
            {
                setParent(other->parent());
                return;
            }

            m_parent = other->parent();
            m_parentLink = other->m_parentLink + 1;
            YGNodeInsertChild(m_parent->layout().m_node, layout().m_node, m_parentLink);
            auto next = m_parent->m_children.insert(m_parent->m_children.begin() + m_parentLink, this) + 1;
            for (; next != m_parent->m_children.end(); next++) (*next)->m_parentLink++;
        }
        else
        {
            setParent(nullptr);
        }
    }
    else if (parent())
    {
        insertBefore(parent()->children().front());
    }
}

