#include <AK/AKLayout.h>
#include <AK/nodes/AKNode.h>

using namespace AK;

void AKLayout::checkIsDirty() noexcept
{
    if (YGNodeIsDirty(m_node))
        m_akNode.addChange(AKNode::Chg_Layout);
}
