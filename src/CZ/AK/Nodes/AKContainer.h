#ifndef CZ_AKCONTAINER_H
#define CZ_AKCONTAINER_H

#include <CZ/AK/Nodes/AKNode.h>

/**
 * @brief Container node that doesn't produce any output on its own.
 * @ingroup AKNodes
 */
class CZ::AKContainer : public AKNode
{
public:
    explicit AKContainer(YGFlexDirection flexDirection = YGFlexDirectionColumn, bool clipsChildren = false, AKNode *parent = nullptr) noexcept : AKNode(parent)
    {
        m_caps |= ContainerBit;
        enableChildrenClipping(clipsChildren);
        layout().setFlexDirection(flexDirection);
    }
};

#endif // CZ_AKCONTAINER_H
