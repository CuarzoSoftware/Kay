#ifndef CZ_AKTEXTCARET_H
#define CZ_AKTEXTCARET_H

#include <CZ/AK/Nodes/AKThreeImagePatch.h>
#include <CZ/AK/AKAnimation.h>

/**
 * @brief Blinking caret for text fields.
 * @ingroup AKNodes
 */
class CZ::AKTextCaret : public AKThreeImagePatch
{
public:
    AKTextCaret(AKNode *parent = nullptr) noexcept;
    CZ_DISABLE_COPY(AKTextCaret)
    void setAnimated(bool enabled) noexcept;
    bool animated() const noexcept { return m_animated; };
protected:
    void layoutEvent(const CZLayoutEvent &event) override;
    void updateDimensions() noexcept;
    AKAnimation m_blinkAnimation;
    bool m_animated { false };
};

#endif // CZ_AKTEXTCARET_H
