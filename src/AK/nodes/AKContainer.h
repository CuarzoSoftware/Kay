#ifndef AKCONTAINER_H
#define AKCONTAINER_H

#include <AK/nodes/AKNode.h>

/**
 * @brief Container node that doesn't produce any output on its own.
 * @ingroup AKNodes
 */
class AK::AKContainer : public AKNode
{
public:
    explicit AKContainer(YGFlexDirection flexDirection = YGFlexDirectionColumn, bool clipsChildren = false, AKNode *parent = nullptr) noexcept : AKNode(parent)
    {
        enableChildrenClipping(clipsChildren);
        layout().setFlexDirection(flexDirection);
    }

    AKCLASS_NO_COPY(AKContainer)
};

#endif // AKCONTAINER_H
