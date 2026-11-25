#ifndef CZ_AKEDGESHADOW_H
#define CZ_AKEDGESHADOW_H

#include <CZ/AK/Nodes/AKImage.h>
#include <CZ/Core/CZEdge.h>
#include <CZ/Core/CZWeak.h>
#include <CZ/Core/CZLinearAnimation.h>

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
 * It can be shown only while some content is scrolled (showWhileScrolled()), like the shadow under
 * the toolbar of macOS windows, which appears once the content goes under it.
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
    ~AKEdgeShadow() noexcept;

    /** @brief Sets the parent edge the shadow is attached to, updating its layout and orientation. */
    void setEdge(CZEdge edge) noexcept;

    /** @brief Returns the parent edge the shadow is attached to. */
    CZEdge edge() const noexcept { return m_edge; };

    /**
     * @brief Shows the shadow only while @p scroll is scrolled down (AKScroll::offsetY() < 0, its
     *        content's top hidden), fading in and out (FadeMs).
     *
     * nullptr: always shown (the default).
     */
    void showWhileScrolled(AKScroll *scroll) noexcept;

    /** @brief The scroll it follows (see showWhileScrolled()), or nullptr. */
    AKScroll *scroll() const noexcept { return m_scroll.get(); }

    /**
     * @brief How opaque the shadow is when shown, from 0 to 1 (1 by default: AKTheme::EdgeShadowColor
     *        as is). Lighter shadows suit subtle separators (e.g. a toolbar's over scrolled content).
     */
    void setStrength(SkScalar strength) noexcept;
    SkScalar strength() const noexcept { return m_strength; }

    /** @brief Duration of the fade in/out of showWhileScrolled(). */
    static constexpr UInt32 FadeMs { 90 };

protected:
    CZEdge m_edge;
    /// @cond INTERNAL
    void updateScrolled(bool animated) noexcept;
    CZWeak<AKScroll> m_scroll;
    CZListener *m_scrollListener { nullptr };
    CZLinearAnimation m_fade;
    void setLevel(SkScalar level) noexcept; // Shown fraction (fade), times strength() is the opacity
    SkScalar m_fadeFrom { 0.f }, m_fadeTo { 1.f };
    SkScalar m_level { 1.f }, m_strength { 1.f };
    /// @endcond
    /** @brief Refreshes the baked image when the scale changes. */
    void layoutEvent(const CZLayoutEvent &event) override;
};

#endif // CZ_AKEDGESHADOW_H
