#ifndef CZ_AKBUTTON_H
#define CZ_AKBUTTON_H

#include <CZ/AK/Nodes/AKSubScene.h>
#include <CZ/AK/Nodes/AKThreeImagePatch.h>
#include <CZ/AK/Nodes/AKText.h>
#include <CZ/AK/Nodes/AKContainer.h>

/**
 * @brief A button.
 * @ingroup AKNodes
 */
class CZ::AKButton : public AKSubScene
{
public:
    enum Changes
    {
        CHEnabled = AKSubScene::CHLast,
        CHBackgroundColor,
        CHPressed,
        CHLast
    };

    AKButton(const std::string &text, AKNode *parent = nullptr) noexcept;
    CZ_DISABLE_COPY(AKButton)

    void setText(const std::string &text) noexcept;

    void setBackgroundColor(SkColor color) noexcept;
    SkColor backgroundColor() const noexcept { return m_backgroundColor; };

    void setEnabled(bool enabled) noexcept;
    bool enabled() const noexcept { return m_enabled; }

    void setPressed(bool pressed) noexcept;
    bool pressed() const noexcept { return m_pressed; };

    CZSignal<const CZPointerButtonEvent&> onClick;

protected:
    void pointerButtonEvent(const CZPointerButtonEvent &event) override;
    void windowStateEvent(const CZWindowStateEvent &event) override;
    void layoutEvent(const CZLayoutEvent &event) override;
    void applyLayoutConstraints() noexcept;
    void updateOpaqueRegion() noexcept;
    void updateStyle() noexcept;
    AKThreeImagePatch m_hThreePatch { CZOrientation::H, this };
    AKContainer m_content { YGFlexDirectionRow, true, &m_hThreePatch };
    AKText m_text;
    SkColor m_backgroundColor { SK_ColorWHITE };
    bool m_enabled { true };
    bool m_pressed { false };
};

#endif // CZ_AKBUTTON_H
