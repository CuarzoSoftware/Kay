#ifndef AKTEXTFIELD_H
#define AKTEXTFIELD_H

#include <AK/AKTheme.h>
#include <AK/nodes/AKSubScene.h>
#include <AK/nodes/AKThreeImagePatch.h>
#include <AK/nodes/AKTextCaret.h>
#include <AK/nodes/AKContainer.h>
#include <AK/nodes/AKText.h>
#include <AK/effects/AKBackgroundImageShadowEffect.h>

/**
 * @brief A node with text-input capabilities.
 * @ingroup AKNodes
 */
class AK::AKTextField : public AKContainer
{
public:
    AKTextField(AKNode *parent = nullptr) noexcept;
protected:
    void layoutEvent(const AKLayoutEvent &event) override;
    void onEvent(const AKEvent &event) override;
    void updateDimensions() noexcept;
    void updateScale() noexcept;
    void updateTextPosition() noexcept;
    AKThreeImagePatch m_hThreePatch { AKThreeImagePatch::Horizontal, this };
    AKContainer m_content { YGFlexDirectionRow, true, &m_hThreePatch };
    AKText m_text { "😊😃😆", &m_content };
    AKTextCaret m_caret { &m_content };
    //AKBackgroundImageShadowEffect m_focusShadow { 8.f, {0, 0}, AKTheme::FocusOutlineColor, this };
};

#endif // AKTEXTFIELD_H
