#include <include/core/SkColorSpace.h>
#include <include/gpu/ganesh/SkImageGanesh.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>

#include <AK/events/AKKeyboardEnterEvent.h>
#include <AK/events/AKKeyboardLeaveEvent.h>
#include <AK/events/AKPointerEnterEvent.h>
#include <AK/AKSafeEventQueue.h>
#include <AK/AKApplication.h>
#include <AK/AKTarget.h>
#include <AK/AKScene.h>
#include <AK/nodes/AKNode.h>
#include <AK/nodes/AKSubScene.h>
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

    setParent(nullptr);
}


bool AKNode::damageTargets() noexcept
{
    if (!visible())
        return false;

    if (caps() != 0)
        for (auto &t : m_targets)
        {
            if (m_intersectedTargets.contains(t.first))
            {
                t.second.target->m_damage.op(t.second.prevLocalRect, SkRegion::kUnion_Op);
                t.first->markDirty();
            }
        }

    return true;
}

void AKNode::damageTargetsAndPropagate() noexcept
{
    if (!damageTargets())
        return;

    for (AKNode *child : m_children)
        child->damageTargetsAndPropagate();
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

const AKChanges &AKNode::changes() const noexcept
{
    static AKChanges emptyChanges;

    if (t && t->target)
        return m_targets[t->target].changes;
    else if (!m_targets.empty())
        return m_targets.begin()->second.changes;

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
        akApp()->sendEvent(AKPointerEnterEvent(), *this);
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

void AKNode::updateSubScene() noexcept
{
    if (parent())
    {
        if (parent()->caps() & Caps::Scene)
            m_subScene.reset((AKSubScene*)parent());
        else
            m_subScene.reset(parent()->subScene());
    }
    else
        m_subScene.reset();

    for (AKNode *child : m_children)
        child->updateSubScene();
}

void AKNode::setParentPrivate(AKNode *parent, bool handleChanges) noexcept
{
    assert(!parent || (parent != this && !parent->isSubchildOf(this)));

    if (m_parent && m_parent == parent && m_parent->children().back() == this)
        return;

    const bool isBackgroundEffect { (m_caps & Caps::BackgroundEffect) != 0 };

    if (m_parent)
    {
        if (!isBackgroundEffect)
        {
            if (handleChanges && m_parent != parent)
                addChange(CHParent);

            YGNodeRemoveChild(m_parent->layout().m_node, layout().m_node);
        }
        auto next = m_parent->m_children.erase(m_parent->m_children.begin() + m_parentLinkIndex);
        for (; next != m_parent->m_children.end(); next++) (*next)->m_parentLinkIndex--;
    }

    m_parent = parent;

    if (parent)
    {
        m_parentLinkIndex = parent->children().size();
        parent->m_children.push_back(this);

        if (isBackgroundEffect)
        {
            m_scene.reset(parent->scene());
            updateSubScene();
            return;
        }

        YGNodeInsertChild(parent->layout().m_node, layout().m_node, m_parentLinkIndex);

        if (handleChanges)
        {
            if (!isBackgroundEffect)
                damageTargetsAndPropagate();
            parent->addChange(CHLayout);
            setScene(parent->scene());
            updateSubScene();
        }

        if (scene())
            scene()->m_treeChanged = true;
    }
    else if (handleChanges)
    {
        if (!isBackgroundEffect)
            damageTargetsAndPropagate();

        if (isBackgroundEffect)
            m_scene.reset();
        else
            setScene(nullptr);

        updateSubScene();
    }
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

AKNode *AKNode::bottommostRightChild() const noexcept
{
    const AKNode *child { this };

    while (child && !child->children().empty())
        child = child->children().back();

    return child == this ? nullptr : (AKNode*)child;
}

AKNode *AKNode::bottommostLeftChild() const noexcept
{
    const AKNode *child { this };

    while (child && !child->children().empty())
        child = child->children().front();

    return child == this ? nullptr : (AKNode*)child;
}

void AKNode::insertBefore(AKNode *other) noexcept
{
    assert(!other || !other->isSubchildOf(this));

    if (other == this)
        return;

    const bool isBackgroundEffect { (m_caps & Caps::BackgroundEffect) != 0 };

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
            else if (!isBackgroundEffect)
            {
                addChange(CHParent);

                if (parent())
                    parent()->addChange(CHLayout);
            }

            if (isBackgroundEffect)
            {
                m_scene.reset(other->scene());
            }
            else
            {
                damageTargetsAndPropagate();
                setScene(other->scene());
                other->parent()->addChange(CHLayout);
            }

            setParentPrivate(nullptr, false);
            m_parent = other->parent();
            m_parentLinkIndex = other->m_parentLinkIndex;

            if (!isBackgroundEffect)
                YGNodeInsertChild(m_parent->layout().m_node, layout().m_node, m_parentLinkIndex);
            auto next = m_parent->m_children.insert(m_parent->m_children.begin() + m_parentLinkIndex, this) + 1;
            for (; next != m_parent->m_children.end(); next++) (*next)->m_parentLinkIndex++;

            updateSubScene();

            assert(m_parent->m_children[m_parentLinkIndex] == this);
            assert(m_parent->m_children[m_parentLinkIndex+1] == other);
            assert(m_parent->m_children[other->m_parentLinkIndex] == other);

            if (!isBackgroundEffect && scene())
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

    const bool isBackgroundEffect { (m_caps & Caps::BackgroundEffect) != 0 };

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
            else if (!isBackgroundEffect)
            {
                addChange(CHParent);

                if (parent())
                    parent()->addChange(CHLayout);
            }

            if (other->parent()->children().back() == other)
            {
                setParent(other->parent());
                return;
            }

            setParentPrivate(nullptr, false);

            if (isBackgroundEffect)
            {
                m_scene.reset(other->scene());
            }
            else
            {
                damageTargetsAndPropagate();
                setScene(other->scene());
                other->parent()->addChange(CHLayout);
            }

            m_parent = other->parent();
            m_parentLinkIndex = other->m_parentLinkIndex + 1;

            if (!isBackgroundEffect)
                YGNodeInsertChild(m_parent->layout().m_node, layout().m_node, m_parentLinkIndex);

            auto next = m_parent->m_children.insert(m_parent->m_children.begin() + m_parentLinkIndex, this) + 1;
            for (; next != m_parent->m_children.end(); next++) (*next)->m_parentLinkIndex++;

            updateSubScene();

            if (!isBackgroundEffect && scene())
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

bool AKNode::isPointerOver() const noexcept
{
    AKPointer &p { akApp()->pointer() };
    return scene() && scene() == p.windowFocus() && globalRect().contains(p.pos().x(), p.pos().y());
}

void AKNode::setKeyboardFocus(bool set) noexcept
{
    if (!scene() || set == hasKeyboardFocus())
        return;

    if (set)
    {
        if (!isKeyboardFocusable())
            return;

        AKSafeEventQueue queue;

        if (scene()->keyboardFocus())
            queue.addEvent(AKKeyboardLeaveEvent(), *scene()->keyboardFocus());

        scene()->m_win->keyboardFocus.reset(this);
        queue.addEvent(AKKeyboardEnterEvent(), *this);
        queue.dispatch();
    }
    else
    {
        scene()->m_win->keyboardFocus.reset();
        akApp()->sendEvent(AKKeyboardLeaveEvent(), *this);
    }
}

bool AKNode::hasKeyboardFocus() const noexcept
{
    return scene() && scene()->keyboardFocus() == this;
}

void AKNode::setKeyboardFocusable(bool enabled) noexcept
{
    m_flags.setFlag(KeyboardFocusable, enabled);

    if (!enabled && hasKeyboardFocus())
        setKeyboardFocus(false);
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

    backgroundEffect->damageTargetsAndPropagate();
    m_backgroundEffects.erase(backgroundEffect);
    backgroundEffect->m_targetNode.reset(nullptr);
    backgroundEffect->onTargetNodeChanged();
}

AKNode *AKNode::root() const noexcept
{
    if (!m_scene)
        return nullptr;
    return m_scene->root();
}

bool AKNode::activated() const noexcept
{
    return scene() && scene()->windowState().check(AKActivated);
}

bool AKNode::event(const AKEvent &event)
{
    switch (event.type())
    {
    case AKEvent::Layout:
        layoutEvent((const AKLayoutEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    case AKEvent::WindowState:
        windowStateEvent((const AKWindowStateEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    case AKEvent::PointerEnter:
        pointerEnterEvent((const AKPointerEnterEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    case AKEvent::PointerMove:
        pointerMoveEvent((const AKPointerMoveEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    case AKEvent::PointerButton:
        pointerButtonEvent((const AKPointerButtonEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    case AKEvent::PointerScroll:
        pointerScrollEvent((const AKPointerScrollEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    case AKEvent::PointerLeave:
        pointerLeaveEvent((const AKPointerLeaveEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    case AKEvent::KeyboardEnter:
        keyboardEnterEvent((const AKKeyboardEnterEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    case AKEvent::KeyboardKey:
        keyboardKeyEvent((const AKKeyboardKeyEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    case AKEvent::KeyboardLeave:
        keyboardLeaveEvent((const AKKeyboardLeaveEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    default:
        return AKObject::event(event);
    }

    return true;
}

void AKNode::layoutEvent(const AKLayoutEvent &event)
{
    onLayoutChanged.notify(event);
    ((const AKEvent&)event).ignore();
}

void AKNode::windowStateEvent(const AKWindowStateEvent &event)
{
    ((const AKEvent&)event).ignore();
}

void AKNode::pointerEnterEvent(const AKPointerEnterEvent &event)
{
    ((const AKEvent&)event).ignore();
}

void AKNode::pointerMoveEvent(const AKPointerMoveEvent &event)
{
    ((const AKEvent&)event).ignore();
}

void AKNode::pointerButtonEvent(const AKPointerButtonEvent &event)
{
    ((const AKEvent&)event).ignore();
}

void AKNode::pointerScrollEvent(const AKPointerScrollEvent &event)
{
    ((const AKEvent&)event).ignore();
}

void AKNode::pointerLeaveEvent(const AKPointerLeaveEvent &event)
{
    ((const AKEvent&)event).ignore();
}

void AKNode::keyboardEnterEvent(const AKKeyboardEnterEvent &event)
{
    ((const AKEvent&)event).ignore();
}

void AKNode::keyboardKeyEvent(const AKKeyboardKeyEvent &event)
{
    ((const AKEvent&)event).ignore();
}

void AKNode::keyboardLeaveEvent(const AKKeyboardLeaveEvent &event)
{
    ((const AKEvent&)event).ignore();
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

void AKNode::Iterator::reset(AKNode *node) noexcept
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

void AKNode::Iterator::next() noexcept
{
    m_done = m_end == m_node;

    if (done()) return;

    AKNode *next { m_node->next() };

    if (!next)
    {
        m_node = m_node->parent();
        return;
    }
    else
    {
        AKNode *bottommost { next->bottommostLeftChild() };

        if (bottommost)
        {
            m_node = bottommost;
            return;
        }
        else
            m_node = next;
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
        AKNode *bottommost { prev->bottommostRightChild() };

        if (bottommost)
        {
            m_node = bottommost;
            return;
        }
        else
            m_node = prev;
    }
}

void AKNode::Iterator::jumpTo(AKNode *node) noexcept
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
