#ifndef CZ_AKEDGESHADOW_H
#define CZ_AKEDGESHADOW_H

#include <CZ/AK/Nodes/AKImage.h>
#include <CZ/Core/CZEdge.h>

/**
 * @brief A thin shadow attached to one edge of its parent.
 *
 * An AKImage that displays the theme's baked edge-shadow gradient
 * (AKTheme::edgeShadowImage()) just outside the chosen parent edge,
 * e.g. to separate a toolbar from the content below it.
 *
 * It positions itself absolutely: it spans the full parent width (top/bottom)
 * or height (left/right), is AKTheme::EdgeShadowRadius thick, and is tinted
 * with AKTheme::EdgeShadowColor. The image is re-fetched automatically when
 * the scale changes.
 *
 * @ingroup AKEffects
 */
class CZ::AKEdgeShadow : public AKImage
{
public:

    /**
     * @brief Flags for tracking property changes.
     */
    enum Changes
    {
        CHEdge = AKImage::CHLast, ///< The edge changed.
        CHLast                    ///< Marker for subclass chaining.
    };

    /**
     * @brief Constructs an edge shadow.
     *
     * @param edge The parent edge to attach the shadow to.
     * @param parent Optional parent node.
     */
    AKEdgeShadow(CZEdge edge, AKNode *parent = nullptr) noexcept;

    /** @brief Sets the parent edge the shadow is attached to, updating its layout and orientation. */
    void setEdge(CZEdge edge) noexcept;

    /** @brief Returns the parent edge the shadow is attached to. */
    CZEdge edge() const noexcept { return m_edge; };

protected:
    CZEdge m_edge;
    /** @brief Refreshes the baked image when the scale changes. */
    void layoutEvent(const CZLayoutEvent &event) override;
};

#endif // CZ_AKEDGESHADOW_H
