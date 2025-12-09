#ifndef AKCOLORTHEME_H
#define AKCOLORTHEME_H

#include <CZ/Core/CZColorScheme.h>
#include <CZ/AK/AK.h>
#include <CZ/AK/ThirdParty/Material/dynamiccolor/dynamic_color.h>
#include <CZ/skia/core/SkColor.h>

/**
 * @brief A color theme.
 *
 * A color theme defines a coherent set of colors that can be used by all
 * components of an application. It provides colors based on their semantic
 * purpose and is powered by the Material 3 dynamic color system.
 *
 * For more details, see:
 * https://m3.material.io/styles/color/system/overview
 */
class CZ::AKColorTheme
{
public:

    /**
     * @brief Creates both light and dark color themes from a single base color.
     *
     * This factory method generates a pair of dynamic color schemes (light and dark)
     * derived from the provided base color, using the Material 3 color utilities.
     *
     * @param color The base color used to generate the theme.
     * @return A shared pointer to the created ::AKColorTheme instance.
     */
    static std::shared_ptr<AKColorTheme> MakeFromColor(SkColor color) noexcept;

    /**
     * @brief Returns the light or dark color scheme.
     *
     * If CZColorScheme::Unknown is provided, the light color scheme is returned.
     *
     * @param scheme The desired color scheme variant.
     * @return A reference to the corresponding Material 3 dynamic color scheme.
     */
    const material_color_utilities::DynamicScheme &scheme(CZColorScheme scheme) const noexcept;

    /**
     * @brief Returns the light color scheme.
     */
    const material_color_utilities::DynamicScheme &light() const noexcept { return m_scheme[0]; }

    /**
     * @brief Returns the dark color scheme.
     */
    const material_color_utilities::DynamicScheme &dark() const noexcept { return m_scheme[1]; }

private:
    AKColorTheme(
        material_color_utilities::DynamicScheme light,
        material_color_utilities::DynamicScheme dark) noexcept :
        m_scheme {std::move(light), std::move(dark)}
    {}

    material_color_utilities::DynamicScheme m_scheme[2];
};

#endif // AKCOLORTHEME_H
