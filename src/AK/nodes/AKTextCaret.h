#ifndef AKTEXTCARET_H
#define AKTEXTCARET_H

#include <AK/nodes/AKThreeImagePatch.h>

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
protected:
    void layoutEvent(const AKLayoutEvent &event) override;
    void updateDimensions() noexcept;
    void onSceneBegin() override;
    UInt32 m_animStartMs { 0 };
};

#endif // AKTEXTCARET_H
