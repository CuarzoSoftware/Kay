#ifndef AKCOLORROLE_H
#define AKCOLORROLE_H

namespace CZ
{
    enum class AKColorRole
    {
        // Palette key colors
        PrimaryPaletteKeyColor,
        SecondaryPaletteKeyColor,
        TertiaryPaletteKeyColor,
        NeutralPaletteKeyColor,
        NeutralVariantPaletteKeyColor,

        // Background & surface
        Background,
        OnBackground,
        Surface,
        SurfaceDim,
        SurfaceBright,
        SurfaceContainerLowest,
        SurfaceContainerLow,
        SurfaceContainer,
        SurfaceContainerHigh,
        SurfaceContainerHighest,
        OnSurface,
        SurfaceVariant,
        OnSurfaceVariant,
        InverseSurface,
        InverseOnSurface,
        Outline,
        OutlineVariant,
        Shadow,
        Scrim,
        SurfaceTint,

        // Primary group
        Primary,
        OnPrimary,
        PrimaryContainer,
        OnPrimaryContainer,
        InversePrimary,

        // Secondary group
        Secondary,
        OnSecondary,
        SecondaryContainer,
        OnSecondaryContainer,

        // Tertiary group
        Tertiary,
        OnTertiary,
        TertiaryContainer,
        OnTertiaryContainer,

        // Error group
        Error,
        OnError,
        ErrorContainer,
        OnErrorContainer,

        // Fixed variants
        PrimaryFixed,
        PrimaryFixedDim,
        OnPrimaryFixed,
        OnPrimaryFixedVariant,
        SecondaryFixed,
        SecondaryFixedDim,
        OnSecondaryFixed,
        OnSecondaryFixedVariant,
        TertiaryFixed,
        TertiaryFixedDim,
        OnTertiaryFixed,
        OnTertiaryFixedVariant,
    };
}

#endif // AKCOLORROLE_H
