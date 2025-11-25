#ifndef AKCOLORTHEME_H
#define AKCOLORTHEME_H

#include <CZ/Core/CZColorScheme.h>
#include <CZ/AK/AK.h>
#include <CZ/AK/ThirdParty/Material/dynamiccolor/dynamic_color.h>
#include <CZ/skia/core/SkColor.h>

class CZ::AKColorTheme
{
public:
    static std::shared_ptr<AKColorTheme> MakeFromColor(SkColor color) noexcept;

    const material_color_utilities::DynamicScheme &scheme(CZColorScheme scheme) const noexcept;
    const material_color_utilities::DynamicScheme &light() const noexcept { return m_scheme[0]; }
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
