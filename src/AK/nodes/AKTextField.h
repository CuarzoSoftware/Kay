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
    AKCLASS_NO_COPY(AKTextField)
protected:
    void layoutEvent(const AKLayoutEvent &event) override;
    void keyboardEnterEvent(const AKKeyboardEnterEvent &event) override;
    void keyboardKeyEvent(const AKKeyboardKeyEvent &event) override;
    void keyboardLeaveEvent(const AKKeyboardLeaveEvent &event) override;
    void pointerButtonEvent(const AKPointerButtonEvent &event) override;
    void pointerMoveEvent(const AKPointerMoveEvent &event) override;
    void windowStateEvent(const AKWindowStateEvent &event) override;
    void updateDimensions() noexcept;
    void updateScale() noexcept;
    void updateTextPosition() noexcept;
    void updateCaretPos() noexcept;
    void addUTF8(const char *utf8) noexcept;
    void removeUTF8() noexcept;
    void moveCaretRight() noexcept;
    void moveCaretLeft() noexcept;
    AKThreeImagePatch m_hThreePatch { AKHorizontal, this };
    AKContainer m_content { YGFlexDirectionRow, true, &m_hThreePatch };
    AKText m_text { "", &m_content };
    AKTextCaret m_caret { &m_text };
    size_t m_caretRightOffset { 0 };
    size_t m_selectionStart { 0 };
    bool m_interactiveSelection { false };
    //AKBackgroundImageShadowEffect m_focusShadow { 8.f, {0, 0}, AKTheme::FocusOutlineColor, this };
};

#endif // AKTEXTFIELD_H
