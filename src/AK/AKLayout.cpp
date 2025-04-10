#include "AK/AKApplication.h"
#include <AK/AKLayout.h>
#include <AK/AKSceneTarget.h>
#include <AK/AKLog.h>
#include <AK/nodes/AKSubScene.h>
#include <AK/events/AKLayoutEvent.h>

using namespace AK;

AKLayout::AKLayout(AKNode &akNode) noexcept : m_node(YGNodeNew()), m_akNode(akNode)
{
    m_config = YGConfigNew();
    YGConfigSetPointScaleFactor(m_config, 1.f);
    YGNodeSetConfig(m_node, m_config);
}

void AKLayout::setDisplay(YGDisplay display) noexcept
{
    const bool turnedVisible { this->display() == YGDisplayNone && display != YGDisplayNone };
    YGNodeStyleSetDisplay(m_node, display);

    if (turnedVisible)
    {
        for (auto &t : m_akNode.m_targets)
        {
            t.second.changes.set(AKNode::CHLayout);
            t.first->markDirty();
        }
    }
    else
        checkIsDirty();
}

void AKLayout::checkIsDirty() noexcept
{
    if (YGNodeIsDirty(m_node))
        m_akNode.addChange(AKNode::CHLayout);
}

void AKLayout::apply(bool calculate) noexcept
{
    if (m_akNode.parent())
    {
        if (calculate)
            YGNodeCalculateLayout(
                m_node,
                m_akNode.parent()->layout().calculatedWidth(),
                m_akNode.parent()->layout().calculatedHeight(),
                YGDirectionInherit);

        applyTree(&m_akNode);
    }
    else
    {
        if (calculate)
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

void AKLayout::calculate() noexcept
{
    if (m_akNode.parent())
    {
        YGNodeCalculateLayout(
            m_akNode.parent()->layout().m_node,
            m_akNode.parent()->layout().calculatedWidth(),
            m_akNode.parent()->layout().calculatedHeight(),
            YGDirectionInherit);
    }
    else
    {
        YGNodeCalculateLayout(
            m_node,
            YGUndefined,
            YGUndefined,
            YGDirectionInherit);
    }
}

void AKLayout::applyTree(AKNode *node)
{
    if (!node->visible())
        return;

    const bool updateScale { node->parent()->m_flags.check(AKNode::ChildrenNeedScaleUpdate) };
    const bool updateRect { node->parent()->m_flags.check(AKNode::ChildrenNeedPosUpdate) || YGNodeGetHasNewLayout(node->layout().m_node) };

    if (!updateScale && !updateRect)
        return;

    YGNodeSetHasNewLayout(node->layout().m_node, false);

    AKBitset<AKLayoutEvent::Changes> changes;

    if (updateRect)
    {
        SkIRect newRect = SkIRect::MakeXYWH(
            node->parent()->globalRect().x() + SkScalarFloorToInt(node->layout().calculatedLeft()),
            node->parent()->globalRect().y() + SkScalarFloorToInt(node->layout().calculatedTop()),
            SkScalarRoundToInt(node->layout().calculatedWidth()),
            SkScalarRoundToInt(node->layout().calculatedHeight()));

        if (newRect.topLeft() != node->m_globalRect.topLeft())
        {
            changes.add(AKLayoutEvent::Changes::Pos);
            node->addChange(AKNode::CHLayoutPos);
            node->m_flags.add(AKNode::ChildrenNeedPosUpdate);
        }

        if (newRect.size() != node->m_globalRect.size())
        {
            changes.add(AKLayoutEvent::Changes::Size);
            node->addChange(AKNode::CHLayoutSize);
        }

        node->m_globalRect = newRect;

        if (node->subScene())
        {
            node->m_rect = SkIRect::MakeXYWH(
                node->m_globalRect.x() - node->subScene()->m_globalRect.x(),
                node->m_globalRect.y() - node->subScene()->m_globalRect.y(),
                node->m_globalRect.width(),
                node->m_globalRect.height());
        }
        else
        {
            node->m_rect = SkIRect::MakeXYWH(
                node->m_globalRect.x() - node->root()->m_globalRect.x(),
                node->m_globalRect.y() - node->root()->m_globalRect.y(),
                node->m_globalRect.width(),
                node->m_globalRect.height());
        }
    }

    if (updateScale || updateRect)
    {
        Int32 newScale { 1 };

        if (node->subScene())
            newScale = node->subScene()->scale();
        else if (node->scene())
        {
            if (!node->childrenClippingEnabled() && node->m_globalRect.isEmpty())
                node->m_flags.add(AKNode::ChildrenNeedScaleUpdate);
            else
            {
                for (const auto &target : node->scene()->targets())
                    if (SkIRect::Intersects(node->m_globalRect, target->m_globalIViewport))
                        if (target->bakedComponentsScale() > newScale)
                            newScale = target->bakedComponentsScale();
            }
        }

        if (newScale != node->m_scale)
        {
            node->m_scale = newScale;
            changes.add(AKLayoutEvent::Changes::Scale);
            node->addChange(AKNode::CHLayoutScale);
            node->m_flags.add(AKNode::ChildrenNeedScaleUpdate);
        }
    }

    if (changes.get() != 0)
        akApp()->sendEvent(AKLayoutEvent(changes), *node);

    for (AKNode *child : node->children())
        applyTree(child);

    node->m_flags.remove(AKNode::ChildrenNeedPosUpdate | AKNode::ChildrenNeedPosUpdate);
}
