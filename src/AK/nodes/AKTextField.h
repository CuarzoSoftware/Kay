#ifndef AKTEXTFIELD_H
#define AKTEXTFIELD_H

#include <AK/AKTheme.h>
#include <AK/nodes/AKSubScene.h>
#include <AK/nodes/AKThreeImagePatch.h>
#include <AK/nodes/AKTextCaret.h>
#include <AK/nodes/AKContainer.h>
#include <AK/nodes/AKSimpleText.h>
#include <AK/effects/AKBackgroundImageShadowEffect.h>

class AK::AKTextField : public AKSubScene
{
public:
    AKTextField(AKNode *parent = nullptr) noexcept;
protected:
    void onSceneBegin() override;
    void onEvent(const AKEvent &event) override;
    AKThreeImagePatch m_hThreePatch { AKThreeImagePatch::Horizontal, this };
    AKContainer m_content { YGFlexDirectionRow, true, &m_hThreePatch };
    AKSimpleText m_text { "hola", &m_content };
    AKTextCaret m_caret { &m_content };
    AKBackgroundImageShadowEffect m_focusShadow { 8.f, {0, 0}, AKTheme::FocusOutlineColor, this };
};

#endif // AKTEXTFIELD_H
