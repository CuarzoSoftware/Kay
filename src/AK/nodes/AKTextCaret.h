#ifndef AKTEXTCARET_H
#define AKTEXTCARET_H

#include <AK/nodes/AKThreeImagePatch.h>
#include <AK/AKAnimation.h>

/**
 * @brief Blinking caret for text fields.
 * @ingroup AKNodes
 */
class AK::AKTextCaret : public AKThreeImagePatch
{
public:
    AKTextCaret(AKNode *parent = nullptr) noexcept;
    AKCLASS_NO_COPY(AKTextCaret)
    void setAnimated(bool enabled) noexcept;
    bool animated() const noexcept { return m_animated; };
protected:
    void layoutEvent(const AKLayoutEvent &event) override;
    void updateDimensions() noexcept;
    AKAnimation m_blinkAnimation;
    bool m_animated { false };
};

#endif // AKTEXTCARET_H
