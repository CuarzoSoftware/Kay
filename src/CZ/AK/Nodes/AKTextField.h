#ifndef CZ_AKTEXTFIELD_H
#define CZ_AKTEXTFIELD_H

#include <CZ/AK/AKTheme.h>
#include <CZ/AK/Nodes/AKSubScene.h>
#include <CZ/AK/Nodes/AKThreePatch.h>
#include <CZ/AK/Nodes/AKTextCaret.h>
#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKText.h>
#include <CZ/AK/Effects/AKBackgroundImageShadowEffect.h>

/**
 * @brief A node with text-input capabilities.
 * @ingroup AKNodes
 *
 * A single-line editable field: a themed rounded three-patch background with an
 * AKText and a blinking AKTextCaret inside. It is keyboard focusable (clicking
 * focuses it, Tab moves focus to the next focusable node) and supports typing,
 * Backspace, caret movement with the arrow keys, pointer-driven selection and
 * Ctrl+A select-all. Keyboard focus is dropped when a pointer press lands
 * outside the field. The default size is 200x24.
 */
class CZ::AKTextField : public AKContainer
{
public:
    /** @brief Constructs an empty text field. */
    AKTextField(AKNode *parent = nullptr) noexcept;
protected:
    /** @brief App-level filter that clears keyboard focus when a pointer press lands outside the field. */
    bool eventFilter(const CZEvent &event, CZObject &target) noexcept override;

    /** @brief Updates the text position on resize and the themed images on scale changes. */
    void layoutEvent(const CZLayoutEvent &event) override;

    /** @brief Shows the caret and starts its blink animation. */
    void keyboardEnterEvent(const CZKeyboardEnterEvent &event) override;

    /** @brief Handles typing, Backspace, arrow keys, Tab focus navigation and Ctrl+A. */
    void keyboardKeyEvent(const CZKeyboardKeyEvent &event) override;

    /** @brief Clears the selection and hides the caret. */
    void keyboardLeaveEvent(const CZKeyboardLeaveEvent &event) override;

    /** @brief Takes keyboard focus; a left press places the caret and starts interactive selection. */
    void pointerButtonEvent(const CZPointerButtonEvent &event) override;

    /** @brief Extends the selection while interactive selection is active. */
    void pointerMoveEvent(const CZPointerMoveEvent &event) override;

    /** @brief Pauses/resumes the caret according to the window's activated state. */
    void windowStateEvent(const CZWindowStateEvent &event) override;
    void updateDimensions() noexcept;
    void updateScale() noexcept;
    void updateTextPosition() noexcept;
    void updateCaretPos() noexcept;
    void addUTF8(const char *utf8) noexcept;
    void removeUTF8() noexcept;
    void moveCaretRight() noexcept;
    void moveCaretLeft() noexcept;
    AKThreePatch m_hThreePatch { CZOrientation::H, this };
    AKContainer m_content { YGFlexDirectionRow, true, &m_hThreePatch };
    AKText m_text { "", &m_content };
    AKTextCaret m_caret { &m_text };
    size_t m_caretRightOffset { 0 };
    size_t m_selectionStart { 0 };
    bool m_interactiveSelection { false };
    //AKBackgroundImageShadowEffect m_focusShadow { 8.f, {0, 0}, AKTheme::FocusOutlineColor, this };
};

#endif // CZ_AKTEXTFIELD_H
