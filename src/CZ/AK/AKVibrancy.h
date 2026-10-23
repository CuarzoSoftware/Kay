#ifndef CZ_AKVIBRANCY_H
#define CZ_AKVIBRANCY_H

namespace CZ
{
    /**
     * @brief Vibrancy (background blur) availability state.
     *
     * Carried by AKVibrancyEvent to tell nodes whether their background is being
     * rendered with the vibrancy effect (e.g. compositor-side background blur),
     * so they can adjust their styling accordingly.
     *
     * @ingroup AKCore
     */
    enum class AKVibrancyState
    {
        Disabled, ///< Vibrancy is not applied; nodes should use opaque styling.
        Enabled   ///< Vibrancy is applied behind the node tree.
    };
};

#endif // CZ_AKVIBRANCY_H
