#ifndef CZ_AKOUTLINE_H
#define CZ_AKOUTLINE_H

#include <CZ/AK/Nodes/AKNinePatch.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/Core/CZAdaptive.h>

/**
 * @brief A rounded outline, like the blue rings marking selected or focused items on macOS.
 * @ingroup AKNodes
 *
 * A ring of thickness() drawn just inside the node's bounds, so place it around what it marks,
 * outset by thickness() (plus a gap if wanted). Its inner corners have radius() (match the marked
 * item's corners), the outer ones radius() + thickness().
 *
 * Drawn as a nine-patch from a small image generated for the node's scale and color scheme. The
 * middle is never painted (it's part of the invisible region), so what's inside isn't redrawn.
 */
class CZ::AKOutline : public AKNinePatch
{
public:
    /** @brief Property change bits consumed on the next AKScene::render(). */
    enum Changes
    {
        CHOutline = AKNinePatch::CHLast, ///< color(), radius() or thickness() changed.
        CHLast                           ///< Marker for subclass chaining.
    };

    /** @brief Creates an outline with the default color (AKTheme::ControlAccent), radius and thickness. */
    AKOutline(AKNode *parent = nullptr) noexcept;

    /** @brief Color of the ring (AKTheme::ControlAccent by default). */
    void setColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor color() const noexcept { return m_color; }

    /** @brief Corner radius inside the ring (6 by default). */
    void setRadius(Int32 radius) noexcept;
    Int32 radius() const noexcept { return m_radius; }

    /** @brief Width of the ring (3 by default). */
    void setThickness(Int32 thickness) noexcept;
    Int32 thickness() const noexcept { return m_thickness; }

protected:
    void onSceneBegin() override;

private:
    CZAdaptiveColor m_color { AKTheme::ControlAccent };
    Int32 m_radius { 6 };
    Int32 m_thickness { 3 };
};

#endif // CZ_AKOUTLINE_H
