#include <CZ/AK/AKApp.h>
#include <CZ/AK/AKLayout.h>
#include <CZ/AK/AKTarget.h>
#include <CZ/AK/AKLog.h>
#include <CZ/AK/Nodes/AKSubScene.h>
#include <CZ/Events/CZLayoutEvent.h>
#include <CZ/CZCore.h>

using namespace CZ;

AKLayout::AKLayout(AKNode &akNode) noexcept : m_node(YGNodeNew()), m_akNode(akNode)
{
    m_config = YGConfigNew();
    YGConfigSetPointScaleFactor(m_config, 1.f);
    YGNodeSetConfig(m_node, m_config);

    m_anchorNode.setOnDestroyCallback([this](CZObject*){
        m_anchorNode.reset();
        setPositionType(m_posTypeBeforeAnchorNode);
        YGNodeMarkDirty(m_node);
        m_akNode.addChange(AKNode::CHLayout);
    });
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

void AKLayout::setAnchorNode(AKNode *anchor) noexcept
{
    if (m_anchorNode == anchor || anchor == &m_akNode)
        return;

    if (anchor)
    {
        if (!m_anchorNode)
            m_posTypeBeforeAnchorNode = positionType();
        setPositionType(YGPositionTypeAbsolute);
        m_anchorNode.reset(anchor);
        m_akNode.addChange(AKNode::CHLayout);
    }
    else
    {
        m_anchorNode.reset(anchor);
        setPositionType(m_posTypeBeforeAnchorNode);
        m_akNode.addChange(AKNode::CHLayout);
    }
}

void AKLayout::checkIsDirty() noexcept
{
    if (YGNodeIsDirty(m_node))
        m_akNode.addChange(AKNode::CHLayout);
}

void AKLayout::apply(bool calculate, bool updateRoot) noexcept
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

        if (updateRoot)
        {
            m_akNode.m_worldRect.setXYWH(
                calculatedLeft(), calculatedTop(),
                calculatedWidth(), calculatedHeight());
        }

        for (AKNode *child : m_akNode.children(true))
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

    const bool updateScale { node->parent()->m_flags.has(AKNode::ChildrenNeedScaleUpdate) };
    const bool updateRect { node->parent()->m_flags.has(AKNode::ChildrenNeedPosUpdate) || YGNodeGetHasNewLayout(node->layout().m_node) };

    if (!updateScale && !updateRect)
        return;

    YGNodeSetHasNewLayout(node->layout().m_node, false);

    CZBitset<CZLayoutChange> changes;

    if (updateRect)
    {
        const auto parentWorldPos { node->layout().anchorNode() ? node->layout().anchorNode()->worldRect().topLeft() : node->parent()->worldRect().topLeft() };
        const auto newWorldRect = SkIRect::MakeXYWH(
            parentWorldPos.x() + SkScalarFloorToInt(node->layout().calculatedLeft()),
            parentWorldPos.y() + SkScalarFloorToInt(node->layout().calculatedTop()),
            SkScalarRoundToInt(node->layout().calculatedWidth()),
            SkScalarRoundToInt(node->layout().calculatedHeight()));

        if (newWorldRect.topLeft() != node->m_worldRect.topLeft())
        {
            changes.add(CZLayoutChangePos);
            node->addChange(AKNode::CHLayoutPos);
            node->m_flags.add(AKNode::ChildrenNeedPosUpdate);
        }

        if (newWorldRect.size() != node->m_worldRect.size())
        {
            changes.add(CZLayoutChangeSize);
            node->addChange(AKNode::CHLayoutSize);
        }

        node->m_worldRect = newWorldRect;

        /* The sceneRect is relative to the current target viewport.
         * If it differs from the previous frame, the scene damages both.
         * worldRect is not used for this because AKSubScene children would be
         * unnecessarily damaged when the sub scene position changes. */

        //if (node->subScene())
        //{
            // AKSubScenes always set the RSurface::viewport().xy() = AKSubScene::worldRect().xy()
            // TODO: Add option to change the viewport pos?
        //    node->m_sceneRect = node->m_worldRect.makeOffset(-node->subScene()->m_worldRect.topLeft());
        //}
        //else
        //{
            // applyTree is only called by AKScene if the user provided a valid AKTarget, so this is safe
        //    node->m_sceneRect = node->m_worldRect.makeOffset(-node->scene()->currentTarget()->m_worldViewport.topLeft());
        //}
    }

    if (updateScale || updateRect)
    {
        Int32 newScale { 1 };

        if (node->subScene())
            newScale = node->subScene()->scale();
        else if (node->scene())
        {
            if (!node->childrenClippingEnabled() && node->m_worldRect.isEmpty())
                node->m_flags.add(AKNode::ChildrenNeedScaleUpdate);
            else
            {
                for (const auto &target : node->scene()->targets())
                    if (SkIRect::Intersects(node->m_worldRect, target->m_worldViewport))
                        if (target->bakedNodesScale() > newScale)
                            newScale = target->bakedNodesScale();
            }
        }

        if (newScale != node->m_scale)
        {
            node->m_scale = newScale;
            changes.add(CZLayoutChangeScale);
            node->addChange(AKNode::CHLayoutScale);
            node->m_flags.add(AKNode::ChildrenNeedScaleUpdate);
        }
    }

    if (changes.get() != 0)
        CZCore::Get()->sendEvent(CZLayoutEvent(changes), *node);

    for (AKNode *child : node->children(true))
        applyTree(child);

    node->m_flags.remove(AKNode::ChildrenNeedPosUpdate | AKNode::ChildrenNeedPosUpdate);
}
