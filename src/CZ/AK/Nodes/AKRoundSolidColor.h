#ifndef AKROUNDSOLIDCOLOR_H
#define AKROUNDSOLIDCOLOR_H

#include <CZ/AK/Nodes/AKNinePatch.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/Core/CZBorderRadius.h>

/**
 * @brief Solid-color rounded rectangle with an optional stroke.
 * @ingroup AKNodes
 *
 * Fills the node's bounds with a solid color, rounding each corner independently
 * (see CZBorderRadius), and can draw a stroke of a given width and color inside
 * the bounds. It is rendered as a nine-patch whose image is a rounded-rect asset
 * cached by the theme (AKTheme::RRect9Patch), so nodes sharing the same radii,
 * colors and scale share one image.
 */
class CZ::AKRoundSolidColor : public AKNinePatch
{
public:
    /** @brief Property change bits consumed on the next AKScene::render(). */
    enum Changes
    {
        /// borderRadius() changed.
        CHBorderRadius = AKNinePatch::CHLast,

        /// backgroundColor() changed.
        CHBackgroundColor,

        /// strokeColor() changed.
        CHStrokeColor,

        /// strokeWidth() changed.
        CHStrokeWidth,

        CHLast ///< Marker for subclass chaining.
    };

    /** @brief Constructs the node with the given fill color and corner radii. */
    AKRoundSolidColor(SkColor color, CZBorderRadius borderRadius, AKNode *parent = nullptr) noexcept;

    /** @brief Sets the fill color. */
    void setBackgroundColor(SkColor color) noexcept;

    /** @brief The current fill color. */
    SkColor backgroundColor() const noexcept { return color(); }

    /** @brief Sets the stroke color. Defaults to black (invisible while strokeWidth() is 0). */
    void setStrokeColor(SkColor color) noexcept;

    /** @brief The current stroke color. */
    SkColor strokeColor() const noexcept { return m_strokeColor; }

    /** @brief Sets the stroke width in logical pixels, clamped to >= 0. Defaults to 0 (no stroke). */
    void setStrokeWidth(Int32 width) noexcept;

    /** @brief The current stroke width in logical pixels. */
    Int32 strokeWidth() const noexcept { return m_strokeWidth; }

    /** @brief Sets the corner radii, each clamped to >= 0. Defaults to 8 on all corners. */
    void setBorderRadius(const CZBorderRadius &borderRadius) noexcept;

    /** @brief The current corner radii. */
    const CZBorderRadius &borderRadius() const noexcept { return m_borderRadius; }

protected:
    /** @brief Fetches the themed rounded-rect asset when the style changed and recalculates the opaque region. */
    void onSceneBegin() override;
    using AKNinePatch::setCenter;
    using AKNinePatch::setImage;
    using AKRenderable::setColor;
    using AKRenderable::color;
    std::shared_ptr<AKAsset::RRect9Patch> m_asset;
    CZBorderRadius m_borderRadius { 8 };
    Int32 m_strokeWidth { 0 };
    SkColor m_strokeColor { SK_ColorBLACK };
};

#endif // AKROUNDSOLIDCOLOR_H
