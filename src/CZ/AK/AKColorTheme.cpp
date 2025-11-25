#include <CZ/AK/AKColorTheme.h>

using namespace CZ;
using namespace material_color_utilities;

std::shared_ptr<AKColorTheme> CZ::AKColorTheme::MakeFromColor(SkColor color) noexcept
{
    const auto SourceHCT = Hct(color);
    const auto primary = TonalPalette(SourceHCT.get_hue(), 48.0);
    const auto secondary = TonalPalette(SourceHCT.get_hue(), 16.0);
    const auto tertiary = TonalPalette(SourceHCT.get_hue() + 60, 24.0);
    const auto neutral = TonalPalette(SourceHCT.get_hue(), 4.0);
    const auto neutralVar = TonalPalette(SourceHCT.get_hue(), 8.0);
    const auto error = TonalPalette(25.0, 84.0);

    return std::shared_ptr<AKColorTheme>(new AKColorTheme(
        DynamicScheme(SourceHCT, Variant::kVibrant, 0.0, false, primary, secondary, tertiary, neutral, neutralVar, error),
        DynamicScheme(SourceHCT, Variant::kVibrant, 0.0, true, primary, secondary, tertiary, neutral, neutralVar, error)));
}

const DynamicScheme &AKColorTheme::scheme(CZColorScheme scheme) const noexcept
{
    if (scheme == CZColorScheme::Dark)
        return m_scheme[1];

    return m_scheme[0];
}
