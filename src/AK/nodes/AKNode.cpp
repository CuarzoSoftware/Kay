#include <include/core/SkColorSpace.h>
#include <include/gpu/ganesh/SkImageGanesh.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>

#include <AK/events/AKPointerEnterEvent.h>

#include <AK/AKTarget.h>
#include <AK/AKScene.h>
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
    while (!m_backgroundEffects.empty())
        removeBackgroundEffect(*m_backgroundEffects.begin());

    for (auto &t : m_targets)
        t.second.target->m_damage.op(t.second.prevLocalClip, SkRegion::kUnion_Op);

    setParent(nullptr);
}

void AKNode::addChange(Change change) noexcept
{
    for (auto &t : m_targets)
        t.second.changes.set(change);

    repaint();
}

void AKNode::repaint() noexcept
{
    for (auto *t : m_intersectedTargets)
        t->markDirty();
}

const std::bitset<128> &AKNode::changes() const noexcept
{
    static std::bitset<128> emptyChanges;

    if (t && t->target)
        return m_targets[t->target].changes;
    else if (!m_targets.empty())
        return m_targets.begin()->second.changes;

    if (!emptyChanges.test(0))
        emptyChanges.set();

    return emptyChanges;
}

void AKNode::enablePointerGrab(bool enabled) noexcept
{
    if (pointerGrabEnabled() == enabled)
        return;

    m_flags.setFlag(PointerGrab, enabled);

    if (enabled && !hasPointerFocus())
    {
        m_flags.add(HasPointerFocus);
        onEvent(AKPointerEnterEvent());
    }
}

AKNode *AKNode::topmostInvisibleParent() const noexcept
{
    AKNode *topmost { nullptr };
    const AKNode *node { this };

    while (node)
    {
        if (node->parent() && !node->parent()->visible())
            topmost = node->parent();

        node = node->parent();
    }

    return topmost;
}

AKNode *AKNode::closestClipperParent() const noexcept
{
    if (!parent())
        return nullptr;

    if (parent()->childrenClippingEnabled() || parent()->isRoot())
        return parent();

    return parent()->closestClipperParent();
}

void AKNode::setScene(AKScene *scene) noexcept
{
    if (m_scene == scene)
        return;

    if (m_scene)
        m_scene->m_treeChanged = true;

    m_scene.reset(scene);

    if (scene)
        scene->m_treeChanged = true;

    for (AKNode *child : m_children)
        child->propagateScene(scene);
}

void AKNode::propagateScene(AKScene *scene) noexcept
{
    m_scene.reset(scene);

    for (AKNode *child : m_children)
        child->propagateScene(scene);
}

void AKNode::setParentPrivate(AKNode *parent, bool handleChanges) noexcept
{
    assert(!parent || (parent != this && !parent->isSubchildOf(this)));

    if (m_parent && m_parent == parent && m_parent->children().back() == this)
        return;

    if (m_parent)
    {
        if (handleChanges && m_parent != parent)
            addChange(Chg_Parent);
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

        if (handleChanges)
        {
            parent->addChange(Chg_Layout);
            setScene(parent->scene());
        }

        if (scene())
            scene()->m_treeChanged = true;
    }
    else if (handleChanges)
        setScene(nullptr);
}

void AKNode::addFlagsAndPropagate(UInt32 flags) noexcept
{
    m_flags.add(flags);

    for (AKNode *child : m_children)
        child->addFlagsAndPropagate(flags);
}

void AKNode::removeFlagsAndPropagate(UInt32 flags) noexcept
{    
    m_flags.remove(flags);

    for (AKNode *child : m_children)
        child->removeFlagsAndPropagate(flags);
}

void AKNode::setFlagsAndPropagateToParents(UInt32 flags, bool set) noexcept
{
    AKNode *node { this };

    while (node)
    {
        node->m_flags.setFlag(flags, set);
        node = node->parent();
    }
}

void AKNode::setParent(AKNode *parent) noexcept
{
    setParentPrivate(parent, true);
}

AKNode *AKNode::topmostParent() const noexcept
{
    AKNode *par { parent() };

    while (par && par->parent())
        par = par->parent();

    return par;
}

AKNode *AKNode::bottommostChild() const noexcept
{
    const AKNode *child { this };

    while (child && !child->children().empty())
        child = child->children().back();

    return child == this ? nullptr : (AKNode*)child;
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
            if (other->parent() == parent())
            {
                // Already inserted before
                if (other->m_parentLinkIndex == m_parentLinkIndex + 1)
                    return;
            }
            else
            {
                addChange(Chg_Parent);

                if (parent())
                    parent()->addChange(Chg_Layout);
            }

            setScene(other->scene());
            other->parent()->addChange(Chg_Layout);
            setParentPrivate(nullptr, false);
            m_parent = other->parent();
            m_parentLinkIndex = other->m_parentLinkIndex;
            YGNodeInsertChild(m_parent->layout().m_node, layout().m_node, m_parentLinkIndex);
            auto next = m_parent->m_children.insert(m_parent->m_children.begin() + m_parentLinkIndex, this) + 1;
            for (; next != m_parent->m_children.end(); next++) (*next)->m_parentLinkIndex++;

            assert(m_parent->m_children[m_parentLinkIndex] == this);
            assert(m_parent->m_children[m_parentLinkIndex+1] == other);
            assert(m_parent->m_children[other->m_parentLinkIndex] == other);

            if (scene())
                scene()->m_treeChanged = true;
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
            if (other->parent() == parent())
            {
                // Already inserted after
                if (other->m_parentLinkIndex + 1 == m_parentLinkIndex)
                    return;
            }
            else
            {
                addChange(Chg_Parent);

                if (parent())
                    parent()->addChange(Chg_Layout);
            }

            if (other->parent()->children().back() == other)
            {
                setParent(other->parent());
                return;
            }

            setParentPrivate(nullptr, false);
            setScene(other->scene());
            other->parent()->addChange(Chg_Layout);
            m_parent = other->parent();
            m_parentLinkIndex = other->m_parentLinkIndex + 1;
            YGNodeInsertChild(m_parent->layout().m_node, layout().m_node, m_parentLinkIndex);
            auto next = m_parent->m_children.insert(m_parent->m_children.begin() + m_parentLinkIndex, this) + 1;
            for (; next != m_parent->m_children.end(); next++) (*next)->m_parentLinkIndex++;

            if (scene())
                scene()->m_treeChanged = true;
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

AKTarget *AKNode::currentTarget() const noexcept
{
    assert("The current target can only be accessed during onSceneBegin(), onSceneCalculatedRect(), onRender() or onBake() events" && t);
    assert("The current target can only be accessed during onSceneBegin(), onSceneCalculatedRect(), onRender() or onBake() events" && scene() != nullptr);
    assert("The current target can only be accessed during onSceneBegin(), onSceneCalculatedRect(), onRender() or onBake() events" && scene()->m_eventWithoutTarget == false);
    return t->target;
}

void AKNode::addBackgroundEffect(AKBackgroundEffect *backgroundEffect) noexcept
{
    if (!backgroundEffect || m_backgroundEffects.contains(backgroundEffect))
        return;

    if (backgroundEffect->targetNode())
        backgroundEffect->targetNode()->m_backgroundEffects.erase(backgroundEffect);

    m_backgroundEffects.insert(backgroundEffect);
    backgroundEffect->m_targetNode.reset(this);
    backgroundEffect->onTargetNodeChanged();
}

void AKNode::removeBackgroundEffect(AKBackgroundEffect *backgroundEffect) noexcept
{
    if (!backgroundEffect || !m_backgroundEffects.contains(backgroundEffect))
        return;

    m_backgroundEffects.erase(backgroundEffect);
    backgroundEffect->m_targetNode.reset(nullptr);
    backgroundEffect->onTargetNodeChanged();
}

bool AKNode::activated() const noexcept
{
    return scene() && scene()->activated();
}

void AKNode::RIterator::reset(AKNode *node) noexcept
{
    m_node = node;

    if (m_node)
    {
        m_done = false;
        m_end = m_node->topmostParent();

        if (!m_end)
            m_end = m_node;
    }
    else
    {
        m_done = true;
        m_end = node;
    }
}

void AKNode::RIterator::next() noexcept
{
    m_done = m_end == m_node;

    if (done()) return;

    AKNode *prev { m_node->prev() };

    if (!prev)
    {
        m_node = m_node->parent();
        return;
    }
    else
    {
        AKNode *bottommost { prev->bottommostChild() };

        if (bottommost)
        {
            m_node = bottommost;
            return;
        }
        else
            m_node = prev;
    }
}

void AKNode::RIterator::jumpTo(AKNode *node) noexcept
{
    if (node == m_node) return;

    if (node)
    {
        if (m_end)
        {
            m_done = false;
            m_node = node;
        }
        else
            reset(node);
    }
    else
    {
        m_done = true;
        m_node = m_end = nullptr;
    }
}
