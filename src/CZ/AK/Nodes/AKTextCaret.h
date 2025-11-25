#ifndef CZ_AKTEXTCARET_H
#define CZ_AKTEXTCARET_H

#include <CZ/AK/Nodes/AKThreePatch.h>
#include <CZ/Core/CZLinearAnimation.h>

/**
 * @brief Blinking caret for text fields.
 */
class CZ::AKTextCaret : public AKThreePatch
{
public:
    AKTextCaret(AKNode *parent = nullptr) noexcept;
    void setAnimated(bool enabled) noexcept;
    bool animated() const noexcept { return m_animated; };
protected:
    void layoutEvent(const CZLayoutEvent &event) override;
    void updateDimensions() noexcept;
    CZLinearAnimation m_blinkAnimation;
    bool m_animated { false };
};

#endif // CZ_AKTEXTCARET_H
