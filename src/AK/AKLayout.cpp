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
