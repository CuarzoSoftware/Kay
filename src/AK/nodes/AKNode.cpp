#include <include/core/SkColorSpace.h>
#include <include/gpu/ganesh/SkImageGanesh.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>

#include <AK/AKTarget.h>
#include <AK/nodes/AKNode.h>
#include <AK/AKSurface.h>
#include <AK/effects/AKBackgroundEffect.h>
#include <GL/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <cassert>
#include <yoga/Yoga.h>

using namespace AK;

AKNode::AKNode(AKNode *parent) noexcept
{
    setParent(parent);
}

AKNode::~AKNode()
{
    setBackgroundEffect(nullptr);

    for (auto &t : m_targets)
    {
        t.second.target->m_damage.op(t.second.prevLocalClip, SkRegion::kUnion_Op);
        t.second.target->m_nodes[t.second.targetLink] = t.second.target->m_nodes.back();
        t.second.target->m_nodes.back()->m_targets[t.first].targetLink = t.second.targetLink;
        t.second.target->m_nodes.pop_back();
    }

    setParent(nullptr);
}

void AKNode::addChange(Change change) noexcept
{
    for (auto &t : m_targets)
        t.second.changes.set(change);
}

const std::bitset<128> &AKNode::changes() const noexcept
{
    static std::bitset<128> emptyChanges;
    return currentTarget() == nullptr ? emptyChanges: m_targets[currentTarget()].changes;
}

AKNode *AKNode::closestClipperParent() const noexcept
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
        auto next = m_parent->m_children.erase(m_parent->m_children.begin() + m_parentLinkIndex);
        for (; next != m_parent->m_children.end(); next++) (*next)->m_parentLinkIndex--;
    }

    m_parent = parent;

    if (parent)
    {
        m_parentLinkIndex = YGNodeGetChildCount(parent->layout().m_node);
        YGNodeInsertChild(parent->layout().m_node, layout().m_node, m_parentLinkIndex);
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
            m_parentLinkIndex = other->m_parentLinkIndex;
            YGNodeInsertChild(m_parent->layout().m_node, layout().m_node, m_parentLinkIndex);
            auto next = m_parent->m_children.insert(m_parent->m_children.begin() + m_parentLinkIndex, this) + 1;
            for (; next != m_parent->m_children.end(); next++) (*next)->m_parentLinkIndex++;

            assert(m_parent->m_children[m_parentLinkIndex] == this);
            assert(m_parent->m_children[m_parentLinkIndex+1] == other);
            assert(m_parent->m_children[other->m_parentLinkIndex] == other);
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
            m_parentLinkIndex = other->m_parentLinkIndex + 1;
            YGNodeInsertChild(m_parent->layout().m_node, layout().m_node, m_parentLinkIndex);
            auto next = m_parent->m_children.insert(m_parent->m_children.begin() + m_parentLinkIndex, this) + 1;
            for (; next != m_parent->m_children.end(); next++) (*next)->m_parentLinkIndex++;
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

AKBackgroundEffect *AKNode::backgroundEffect() const noexcept
{
    return m_backgroundEffect;
}

void AKNode::setBackgroundEffect(AKBackgroundEffect *backgroundEffect) noexcept
{
    if (backgroundEffect == m_backgroundEffect)
        return;

    if (m_backgroundEffect)
    {
        m_backgroundEffect->m_targetNode.reset();
        m_backgroundEffect->onTargetNodeChanged();
    }

    m_backgroundEffect.reset(backgroundEffect);

    if (m_backgroundEffect)
    {
        m_backgroundEffect->m_targetNode.reset(this);
        m_backgroundEffect->onTargetNodeChanged();
    }
}

