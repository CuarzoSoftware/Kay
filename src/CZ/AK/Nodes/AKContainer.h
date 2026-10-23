#ifndef CZ_AKCONTAINER_H
#define CZ_AKCONTAINER_H

#include <CZ/AK/Nodes/AKNode.h>

/**
 * @brief Container node that doesn't produce any output on its own.
 *
 * Containers only lay out and group children, optionally clipping them to their
 * bounds. Use them (or derive from them) to build composite components out of
 * existing nodes. See @ref creating_components_page "Creating Components".
 *
 * @ingroup AKNodes
 */
class CZ::AKContainer : public AKNode
{
public:
    /**
     * @brief Constructor.
     *
     * @param flexDirection The Flexbox direction children are laid out in. Defaults to column.
     * @param clipsChildren Whether children are clipped to this node's bounds (see AKNode::enableChildrenClipping()).
     * @param parent Optional parent node.
     */
    explicit AKContainer(YGFlexDirection flexDirection = YGFlexDirectionColumn, bool clipsChildren = false, AKNode *parent = nullptr) noexcept : AKNode(parent)
    {
        m_caps |= ContainerBit;
        enableChildrenClipping(clipsChildren);
        layout().setFlexDirection(flexDirection);
    }
};

#endif // CZ_AKCONTAINER_H
