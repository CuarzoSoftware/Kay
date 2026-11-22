#ifndef CZ_AKWIFIICON_H
#define CZ_AKWIFIICON_H

#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKFontIcon.h>
#include <CZ/Core/CZAdaptive.h>

/**
 * @brief A Wi-Fi signal strength icon: the fan of arcs, the lit ones by strength.
 * @ingroup AKNodes
 *
 * Like macOS's: the whole fan is drawn faded, and the bars the strength reaches over it.
 */
class CZ::AKWiFiIcon : public AKContainer
{
public:
    /** @brief Creates a @p size units icon showing @p strength (0..100). */
    AKWiFiIcon(UInt8 strength = 100, UInt32 size = 16, AKNode *parent = nullptr) noexcept;

    void setStrength(UInt8 strength) noexcept;
    UInt8 strength() const noexcept { return m_strength; }

    /// The bars lit (1..3) for a strength.
    static int Bars(UInt8 strength) noexcept { return strength >= 67 ? 3 : strength >= 34 ? 2 : 1; }

    /** @brief The lit bars' color (AKTheme::Label by default); the rest is drawn with AKTheme::TertiaryLabel. */
    void setColor(CZAdaptiveColor color) noexcept;

    AKFontIcon fan;   ///< All the arcs, faded
    AKFontIcon bars;  ///< The lit ones

protected:
    void colorSchemeEvent(const CZColorSchemeEvent &e) override;

private:
    void updateColors() noexcept;
    CZAdaptiveColor m_color;
    UInt8 m_strength { 100 };
};

#endif // CZ_AKWIFIICON_H
