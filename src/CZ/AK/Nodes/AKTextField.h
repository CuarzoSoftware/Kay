#ifndef CZ_AKTEXTFIELD_H
#define CZ_AKTEXTFIELD_H

#include <CZ/AK/AKTheme.h>
#include <CZ/AK/Nodes/AKSubScene.h>
#include <CZ/AK/Nodes/AKThreeImagePatch.h>
#include <CZ/AK/Nodes/AKTextCaret.h>
#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKText.h>
#include <CZ/AK/Effects/AKBackgroundImageShadowEffect.h>

/**
 * @brief A node with text-input capabilities.
 * @ingroup AKNodes
 */
class CZ::AKTextField : public AKContainer
{
public:
    AKTextField(AKNode *parent = nullptr) noexcept;
    CZ_DISABLE_COPY(AKTextField)
protected:
    void layoutEvent(const CZLayoutEvent &event) override;
    void keyboardEnterEvent(const CZKeyboardEnterEvent &event) override;
    void keyboardKeyEvent(const CZKeyboardKeyEvent &event) override;
    void keyboardLeaveEvent(const CZKeyboardLeaveEvent &event) override;
    void pointerButtonEvent(const CZPointerButtonEvent &event) override;
    void pointerMoveEvent(const CZPointerMoveEvent &event) override;
    void windowStateEvent(const CZWindowStateEvent &event) override;
    void updateDimensions() noexcept;
    void updateScale() noexcept;
    void updateTextPosition() noexcept;
    void updateCaretPos() noexcept;
    void addUTF8(const char *utf8) noexcept;
    void removeUTF8() noexcept;
    void moveCaretRight() noexcept;
    void moveCaretLeft() noexcept;
    AKThreeImagePatch m_hThreePatch { CZOrientation::H, this };
    AKContainer m_content { YGFlexDirectionRow, true, &m_hThreePatch };
    AKText m_text { "", &m_content };
    AKTextCaret m_caret { &m_text };
    size_t m_caretRightOffset { 0 };
    size_t m_selectionStart { 0 };
    bool m_interactiveSelection { false };
    //AKBackgroundImageShadowEffect m_focusShadow { 8.f, {0, 0}, AKTheme::FocusOutlineColor, this };
};

#endif // CZ_AKTEXTFIELD_H
