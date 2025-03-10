#ifndef AKEDGE_H
#define AKEDGE_H

#include <AK/AKBitset.h>
#include <limits>

namespace AK
{
    /**
     * @brief Edge flags.
     */
    enum AKEdge : UInt32
    {
        AKEdgeNone   = 0, ///< No edge.
        AKEdgeTop    = static_cast<UInt32>(1) << 0, ///< The top edge.
        AKEdgeBottom = static_cast<UInt32>(1) << 1, ///< The bottom edge.
        AKEdgeLeft   = static_cast<UInt32>(1) << 2, ///< The left edge.
        AKEdgeRight  = static_cast<UInt32>(1) << 3, ///< The right edge.
    };

    /**
     * @brief Represents a disabled edge.
     */
    static inline constexpr Int32 AKEdgeDisabled = std::numeric_limits<Int32>::min();

    /**
     * @brief Checks if the given edges form a corner by being orthogonal.
     *
     * This function verifies if the provided bitset of edges corresponds to one of the four possible
     * corner configurations: (Top-Left, Top-Right, Bottom-Left, Bottom-Right).
     *
     * @param edges A bitset representing the edges.
     * @return `true` if the edges form a corner, `false` otherwise.
     */
    inline constexpr bool edgeIsCorner(AKBitset<AKEdge> edges) noexcept
    {
        return edges.get() == (AKEdgeTop | AKEdgeLeft)
               || edges.get() == (AKEdgeTop | AKEdgeRight)
               || edges.get() == (AKEdgeBottom | AKEdgeLeft)
               || edges.get() == (AKEdgeBottom | AKEdgeRight);
    }
};

#endif // AKEDGE_H
