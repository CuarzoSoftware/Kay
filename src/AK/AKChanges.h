#ifndef AKCHANGES_H
#define AKCHANGES_H

#include <AK/AK.h>
#include <bitset>
#include <vector>

/**
 * @brief Tracks changes of a node using a bitset.
 * @ingroup AKCore
 *
 * This class enables nodes to monitor changes in their properties and apply them atomically
 * all at once, rather than each time a setter is called.
 *
 * Each node typically has a "Changes" enum with values corresponding to their properties.
 * When a property changes within a node setter, the node registers the change using AKNode::addChange().
 *
 * Calling AKNode::addChange() stores the change in an internal instance of this class, created for
 * each AKTarget the node has been presented to, and marks all currently intersected targets as dirty.
 *
 * Later, during AKNode::onSceneBegin(), AKRenderable::onRender(), and AKBakeable::onBake() events,
 * the node can use this information to update/repaint only what is necessary.
 *
 * Finally, when the node is processed by a scene and isn't completely occluded/clipped by other nodes,
 * the bitset associated with the target is cleared and remains empty until new changes are registered.
 */
class AK::AKChanges : public std::bitset<128>
{
public:
    using std::bitset<128>::bitset;

    /**
     * @brief Checks if any of the specified changes exist in the bitset.
     * @param changes List of changes.
     * @return `true` if any of the changes exist in the bitset, `false` otherwise.
     */
    template<typename... Changes>
    constexpr bool testAnyOf(Changes...changes) const noexcept
    {
        const std::vector<size_t> &vec = {changes...};
        for (const auto ch : vec)
            if (test(ch))
                return true;
        return false;
    }
};

#endif // AKCHANGES_H
