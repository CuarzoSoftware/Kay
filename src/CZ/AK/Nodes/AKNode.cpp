#include <CZ/skia/core/SkColorSpace.h>
#include <CZ/skia/gpu/ganesh/SkImageGanesh.h>
#include <CZ/skia/gpu/ganesh/SkSurfaceGanesh.h>
#include <CZ/Events/CZKeyboardEnterEvent.h>
#include <CZ/Events/CZKeyboardLeaveEvent.h>
#include <CZ/Events/CZPointerEnterEvent.h>
#include <CZ/CZSafeEventQueue.h>
#include <CZ/CZCore.h>

#include <CZ/AK/Events/AKSceneChangedEvent.h>
#include <CZ/AK/AKApp.h>
#include <CZ/AK/AKTarget.h>
#include <CZ/AK/AKScene.h>
#include <CZ/AK/Nodes/AKNode.h>
#include <CZ/AK/Nodes/AKSubScene.h>
#include <CZ/AK/Effects/AKBackgroundEffect.h>

#include <CZ/Marco/nodes/MRootSurfaceNode.h>

#include <CZ/Ream/RSurface.h>

#include <cassert>
#include <yoga/Yoga.h>

using namespace CZ;

AKNode::AKNode(AKNode *parent) noexcept
{
    m_app = AKApp::Get();
    assert(m_app && "Create an AKApp first");
    setParent(parent);
}

AKNode::~AKNode()
{
    while (!m_backgroundEffects.empty())
        removeBackgroundEffect(*m_backgroundEffects.begin());

    setParent(nullptr);

    notifyDestruction();
}

bool AKNode::damageTargets() noexcept
{
    if (!visible())
        return false;

    for (auto *bg : m_backgroundEffects)
        bg->damageTargets();

    if (!caps().has(RenderableBit))
        return true;

    for (auto &t : m_targets)
    {
        if (m_intersectedTargets.contains(t.first))
        {
            t.second.target->m_damage.op(t.second.prevSceneRect, SkRegion::kUnion_Op);
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

void AKNode::setVisible(bool visible) noexcept
{
    if (visible == this->visible())
        return;

    layout().setDisplay(visible ? YGDisplayFlex : YGDisplayNone);

    if (!visible)
        damageTargetsAndPropagate();
}

const AKChanges &AKNode::changes() const noexcept
{
    static AKChanges emptyChanges;

    if (tData && tData->target)
        return tData->changes;
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
        CZCore::Get()->sendEvent(CZPointerEnterEvent(), *this);
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

    AKSceneChangedEvent event { m_scene, scene };

    m_scene.reset(scene);

    if (scene)
        scene->m_treeChanged = true;

    CZCore::Get()->postEvent(event, *this);

    for (AKNode *child : m_children)
        child->propagateScene(scene);
}

void AKNode::propagateScene(AKScene *scene) noexcept
{
    CZCore::Get()->postEvent(AKSceneChangedEvent(m_scene, scene), *this);
    m_scene.reset(scene);

    for (AKNode *child : m_children)
        child->propagateScene(scene);
}

void AKNode::updateSubScene() noexcept
{
    if (parent())
    {
        if (auto parentSubScene = parent()->asSubScene())
            m_subScene.reset(parentSubScene);
        else
            m_subScene.reset(parent()->subScene());
    }
    else
        m_subScene.reset();

    for (AKNode *child : m_children)
        child->updateSubScene();
}

void AKNode::setParentPrivate(AKNode *parent, bool handleChanges, bool ignoreSlot) noexcept
{
    if (parent && !ignoreSlot)
        parent = parent->slot();

    assert(!parent || (parent != this && !parent->isSubchildOf(this)));

    if (m_parent && m_parent == parent && m_parent->children(true).back() == this)
        return;

    const bool isBackgroundEffect { caps().has(BackgroundEffectBit) };

    if (m_parent)
    {
        if (!isBackgroundEffect)
        {
            if (handleChanges && m_parent != parent)
                addChange(CHParent);

            YGNodeRemoveChild(m_parent->layout().m_node, layout().m_node);
        }
        auto next = m_parent->m_children.erase(m_parent->m_children.begin() + m_parentLink);
        for (; next != m_parent->m_children.end(); next++) (*next)->m_parentLink--;
    }

    m_parent = parent;

    if (parent)
    {
        m_parentLink = parent->children(true).size();
        parent->m_children.push_back(this);

        if (isBackgroundEffect)
        {
            m_scene.reset(parent->scene());
            updateSubScene();
            return;
        }

        YGNodeInsertChild(parent->layout().m_node, layout().m_node, m_parentLink);

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

void AKNode::setParent(AKNode *parent, bool ignoreSlot) noexcept
{
    setParentPrivate(parent, true, ignoreSlot);
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

    while (child && !child->children(true).empty())
        child = child->children(true).back();

    return child == this ? nullptr : (AKNode*)child;
}

AKNode *AKNode::bottommostLeftChild() const noexcept
{
    const AKNode *child { this };

    while (child && !child->children(true).empty())
        child = child->children(true).front();

    return child == this ? nullptr : (AKNode*)child;
}

void AKNode::insertBefore(AKNode *other) noexcept
{
    assert(!other || !other->isSubchildOf(this));

    if (other == this)
        return;

    const bool isBackgroundEffect { caps().has(BackgroundEffectBit) };

    if (other)
    {
        if (other->parent())
        {
            if (other->parent() == parent())
            {
                // Already inserted before
                if (other->m_parentLink == m_parentLink + 1)
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

            setParentPrivate(nullptr, false, true);
            m_parent = other->parent();
            m_parentLink = other->m_parentLink;

            if (!isBackgroundEffect)
                YGNodeInsertChild(m_parent->layout().m_node, layout().m_node, m_parentLink);
            auto next = m_parent->m_children.insert(m_parent->m_children.begin() + m_parentLink, this) + 1;
            for (; next != m_parent->m_children.end(); next++) (*next)->m_parentLink++;

            updateSubScene();

            assert(m_parent->m_children[m_parentLink] == this);
            assert(m_parent->m_children[m_parentLink+1] == other);
            assert(m_parent->m_children[other->m_parentLink] == other);

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

    const bool isBackgroundEffect { caps().has(BackgroundEffectBit) };

    if (other)
    {
        if (other->parent())
        {
            if (other->parent() == parent())
            {
                // Already inserted after
                if (other->m_parentLink + 1 == m_parentLink)
                    return;
            }
            else if (!isBackgroundEffect)
            {
                addChange(CHParent);

                if (parent())
                    parent()->addChange(CHLayout);
            }

            if (other->parent()->children(true).back() == other)
            {
                setParent(other->parent());
                return;
            }

            setParentPrivate(nullptr, false, true);

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
            m_parentLink = other->m_parentLink + 1;

            if (!isBackgroundEffect)
                YGNodeInsertChild(m_parent->layout().m_node, layout().m_node, m_parentLink);

            auto next = m_parent->m_children.insert(m_parent->m_children.begin() + m_parentLink, this) + 1;
            for (; next != m_parent->m_children.end(); next++) (*next)->m_parentLink++;

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
        insertBefore(parent()->children(true).front());
    }
}

void AKNode::enableChildrenClipping(bool enable) noexcept
{
    if (childrenClippingEnabled() == enable)
        return;

    m_flags.setFlag(ChildrenClipping, enable);
    addChange(CHChildrenClipping);
}


AKTarget *AKNode::currentTarget() const noexcept
{
    assert("The current target can only be accessed during onSceneBegin(), onSceneCalculatedRect(), onRender() or onBake() events" && tData);
    assert("The current target can only be accessed during onSceneBegin(), onSceneCalculatedRect(), onRender() or onBake() events" && scene() != nullptr);
    assert("The current target can only be accessed during onSceneBegin(), onSceneCalculatedRect(), onRender() or onBake() events" && scene()->m_eventWithoutTarget == false);
    return tData->target;
}

bool AKNode::isPointerOver() const noexcept
{
    AKPointer &p { AKApp::Get()->pointer() };
    return scene() && scene() == p.windowFocus() && worldRect().contains(p.pos().x(), p.pos().y());
}

void AKNode::setKeyboardFocus(bool set) noexcept
{
    if (!scene() || set == hasKeyboardFocus())
        return;

    if (set)
    {
        if (!isKeyboardFocusable())
            return;

        CZSafeEventQueue queue;

        if (scene()->keyboardFocus())
            queue.addEvent(CZKeyboardLeaveEvent(), *scene()->keyboardFocus());

        scene()->m_win->keyboardFocus.reset(this);
        queue.addEvent(CZKeyboardEnterEvent(), *this);
        queue.dispatch();
    }
    else
    {
        scene()->m_win->keyboardFocus.reset();
        CZCore::Get()->sendEvent(CZKeyboardLeaveEvent(), *this);
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
    return scene() && scene()->windowState().has(CZWinActivated);
}

MSurface *AKNode::window() const noexcept
{
    if (MRootSurfaceNode *node = dynamic_cast<MRootSurfaceNode*>(root()))
        return &node->surface();
    return nullptr;
}

void AKNode::innerBounds(SkRect *out) noexcept
{
    out->fLeft = std::numeric_limits<float>::max();
    out->fTop = std::numeric_limits<float>::max();
    out->fRight = std::numeric_limits<float>::min();
    out->fBottom = std::numeric_limits<float>::min();

    SkScalar r, b;
    for (AKNode *child : children(true))
    {
        if (!child->visible() || child->layout().positionType() == YGPositionTypeAbsolute)
            continue;

        if (child->layout().calculatedLeft() < out->fLeft)
            out->fLeft = child->layout().calculatedLeft();

        if (child->layout().calculatedTop() < out->fTop)
            out->fTop = child->layout().calculatedTop();

        r = child->layout().calculatedLeft() + child->layout().calculatedWidth();

        if (r > out->fRight)
            out->fRight = r;

        b = child->layout().calculatedTop() + child->layout().calculatedHeight();

        if (b > out->fBottom)
            out->fBottom = b;
    }

    if (out->fLeft > out->fRight)
        out->fLeft = out->fRight = 0.f;

    if (out->fTop > out->fBottom)
        out->fTop = out->fBottom = 0.f;

    out->fLeft -= layout().calculatedPadding(YGEdgeLeft);
    out->fTop -= layout().calculatedPadding(YGEdgeTop);
    out->fRight += layout().calculatedPadding(YGEdgeRight);
    out->fBottom += layout().calculatedPadding(YGEdgeBottom);
}

AKContainer         *AKNode::asContainer()          noexcept { return dynamic_cast<AKContainer*>(this); }
AKRenderable        *AKNode::asRenderable()         noexcept { return dynamic_cast<AKRenderable*>(this); }
AKBackgroundEffect  *AKNode::asBackgroundEffect()   noexcept { return dynamic_cast<AKBackgroundEffect*>(this); }
AKBakeable          *AKNode::asBakeable()           noexcept { return dynamic_cast<AKBakeable*>(this); }
AKSubScene          *AKNode::asSubScene()           noexcept { return dynamic_cast<AKSubScene*>(this); }

void AKNode::setSlot(AKNode *slot) noexcept
{
    if (slot == this)
        m_slot.reset();
    else
        m_slot.reset(slot);
}

bool AKNode::event(const CZEvent &event) noexcept
{
    switch (event.type())
    {
    case CZEvent::Type::Layout:
        layoutEvent((const CZLayoutEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    case CZEvent::Type::WindowState:
        windowStateEvent((const CZWindowStateEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    case CZEvent::Type::PointerEnter:
        pointerEnterEvent((const CZPointerEnterEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    case CZEvent::Type::PointerMove:
        pointerMoveEvent((const CZPointerMoveEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    case CZEvent::Type::PointerButton:
        pointerButtonEvent((const CZPointerButtonEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    case CZEvent::Type::PointerScroll:
        pointerScrollEvent((const CZPointerScrollEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    case CZEvent::Type::PointerLeave:
        pointerLeaveEvent((const CZPointerLeaveEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    case CZEvent::Type::KeyboardEnter:
        keyboardEnterEvent((const CZKeyboardEnterEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    case CZEvent::Type::KeyboardKey:
        keyboardKeyEvent((const CZKeyboardKeyEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    case CZEvent::Type::KeyboardLeave:
        keyboardLeaveEvent((const CZKeyboardLeaveEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    case CZEvent::Type::AKSceneChanged:
        sceneChangedEvent((const AKSceneChangedEvent&)event);
        if (!event.isAccepted())
            return false;
        break;
    default:
        return AKObject::event(event);
    }

    return true;
}

void AKNode::layoutEvent(const CZLayoutEvent &event)
{
    onLayoutChanged.notify(event);
    ((const CZEvent&)event).ignore();
}

void AKNode::windowStateEvent(const CZWindowStateEvent &event)
{
    ((const CZEvent&)event).ignore();
}

void AKNode::pointerEnterEvent(const CZPointerEnterEvent &event)
{
    ((const CZEvent&)event).ignore();
}

void AKNode::pointerMoveEvent(const CZPointerMoveEvent &event)
{
    ((const CZEvent&)event).ignore();
}

void AKNode::pointerButtonEvent(const CZPointerButtonEvent &event)
{
    ((const CZEvent&)event).ignore();
}

void AKNode::pointerScrollEvent(const CZPointerScrollEvent &event)
{
    ((const CZEvent&)event).ignore();
}

void AKNode::pointerLeaveEvent(const CZPointerLeaveEvent &event)
{
    ((const CZEvent&)event).ignore();
}

void AKNode::keyboardEnterEvent(const CZKeyboardEnterEvent &event)
{
    ((const CZEvent&)event).ignore();
}

void AKNode::keyboardKeyEvent(const CZKeyboardKeyEvent &event)
{
    ((const CZEvent&)event).ignore();
}

void AKNode::keyboardLeaveEvent(const CZKeyboardLeaveEvent &event)
{
    ((const CZEvent&)event).ignore();
}

void AKNode::sceneChangedEvent(const AKSceneChangedEvent &event)
{
    ((const CZEvent&)event).ignore();
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
