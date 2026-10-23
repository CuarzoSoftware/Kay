#ifndef CZ_AKTEXTCARET_H
#define CZ_AKTEXTCARET_H

#include <CZ/AK/Nodes/AKThreePatch.h>
#include <CZ/Core/CZLinearAnimation.h>

/**
 * @brief Blinking caret for text fields.
 * @ingroup AKNodes
 *
 * A thin vertical three-patch tinted with AKTheme::SystemBlue, 16 logical pixels
 * tall by default. Its input region is empty, so it never takes pointer events.
 * Used internally by AKTextField.
 */
class CZ::AKTextCaret : public AKThreePatch
{
public:
    /** @brief Constructs a caret (blinking disabled). */
    AKTextCaret(AKNode *parent = nullptr) noexcept;

    /**
     * @brief Starts or stops the blink animation (a 1.2 s opacity cycle that repeats while enabled).
     *
     * Disabling it restores full opacity.
     */
    void setAnimated(bool enabled) noexcept;

    /** @brief Whether the blink animation is running. Defaults to `false`. */
    bool animated() const noexcept { return m_animated; };
protected:
    /** @brief Regenerates the themed caret image when the scale changes. */
    void layoutEvent(const CZLayoutEvent &event) override;
    void updateDimensions() noexcept;
    CZLinearAnimation m_blinkAnimation;
    bool m_animated { false };
};

#endif // CZ_AKTEXTCARET_H
