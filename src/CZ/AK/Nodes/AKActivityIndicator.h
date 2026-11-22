#ifndef CZ_AKACTIVITYINDICATOR_H
#define CZ_AKACTIVITYINDICATOR_H

#include <CZ/AK/Nodes/AKBakeable.h>
#include <CZ/Core/CZAdaptive.h>
#include <CZ/Core/CZTimer.h>

/**
 * @brief A spinning activity indicator, like macOS's: a ring of spokes fading behind the
 *        brightest one, which steps around while it runs.
 * @ingroup AKNodes
 *
 * It spins while visible and running() (started by default); stop() freezes it (e.g. to hide
 * it). The spokes take the color set with setColor() (AKTheme::SecondaryLabel by default).
 */
class CZ::AKActivityIndicator : public AKBakeable
{
public:
    enum Changes
    {
        CHPhase = AKBakeable::CHLast, ///< The bright spoke moved.
        CHLast
    };

    /** @brief Creates a @p size units indicator, spinning. */
    AKActivityIndicator(Int32 size = 16, AKNode *parent = nullptr) noexcept;

    static constexpr Int32 Spokes   { 12 };
    static constexpr UInt32 StepMs  { 1000 / Spokes }; ///< A turn per second

    void setSize(Int32 size) noexcept;

    void start() noexcept;
    void stop() noexcept;
    bool running() const noexcept { return m_running; }

    /** @brief The spokes' color (adaptive). */
    void setSpokeColor(CZAdaptiveColor color) noexcept;

protected:
    void bakeEvent(const AKBakeEvent &e) override;
    void colorSchemeEvent(const CZColorSchemeEvent &e) override;

private:
    void step() noexcept;
    CZAdaptiveColor m_color;
    CZTimer m_timer;
    Int32 m_phase { 0 };
    bool m_running { false };
};

#endif // CZ_AKACTIVITYINDICATOR_H
