#ifndef CZ_AKTEXTINPUT_H
#define CZ_AKTEXTINPUT_H

#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Nodes/AKCoreTextEditor.h>
#include <CZ/AK/Nodes/AKText.h>
#include <CZ/AK/Nodes/AKIconButton.h>
#include <CZ/Core/CZSignal.h>

/**
 * @brief A single-line text field: a bordered box with a placeholder, like macOS's.
 * @ingroup AKNodes
 *
 * Built on AKCoreTextEditor (editor). setPasswordMode() shows bullets instead of the text, and
 * setShowsVisibilityToggle() adds an eye button at the right that shows or hides it. Clicking
 * anywhere on the box focuses it; it's outlined with the accent color while focused.
 */
class CZ::AKTextInput : public AKRoundSolidColor
{
public:
    /** @brief Creates the field with @p placeholder. */
    AKTextInput(const std::string &placeholder = "", AKNode *parent = nullptr) noexcept;

    static constexpr SkScalar Height   { 24.f };
    static constexpr SkScalar FontSize { 13.f };

    const std::string &text() const noexcept { return editor.text(); }
    void setText(const std::string &text) noexcept;
    void setPlaceholder(const std::string &text) noexcept { placeholder.setText(text); }

    /** @brief Shows each character as a bullet (see AKCoreTextEditor::setPasswordMode()). */
    void setPasswordMode(bool enabled) noexcept;
    bool passwordMode() const noexcept { return editor.passwordMode(); }

    /**
     * @brief Shows an eye button (visibilityToggle) at the right that turns setPasswordMode() off
     *        and on, so the password can be checked while typed. Hidden by default.
     */
    void setShowsVisibilityToggle(bool shows) noexcept { visibilityToggle.setVisible(shows); }
    bool showsVisibilityToggle() const noexcept { return visibilityToggle.visible(); }

    /** @brief Gives it the keyboard focus. */
    void focus() noexcept { editor.setKeyboardFocus(true); }

    CZSignal<const std::string&> onTextChanged;
    CZSignal<const std::string&> onSubmit;   ///< Enter
    CZSignal<> onCancel;                     ///< Escape

    AKCoreTextEditor editor { "", this };
    AKText placeholder { "", this };
    AKIconButton visibilityToggle { "visibility", 20, this }; ///< See setShowsVisibilityToggle()

protected:
    void pointerButtonEvent(const CZPointerButtonEvent &e) override;
    void colorSchemeEvent(const CZColorSchemeEvent &e) override;
    /// @cond INTERNAL
    void updateColors() noexcept;
    void updateVisibilityToggle() noexcept;
    /// @endcond
};

#endif // CZ_AKTEXTINPUT_H
