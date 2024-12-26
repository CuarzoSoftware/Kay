#ifndef AKCONTAINER_H
#define AKCONTAINER_H

#include <AK/nodes/AKNode.h>

/**
 * @brief Container node that doesn't produce any output on its own.
 */
class AK::AKContainer : public AKNode
{
public:
    AKContainer(YGFlexDirection flexDirection = YGFlexDirectionColumn, bool clipsChildren = false, AKNode *parent = nullptr) noexcept : AKNode(parent)
    {
        enableChildrenClipping(clipsChildren);
        layout().setFlexDirection(flexDirection);
    }
};

#endif // AKCONTAINER_H
