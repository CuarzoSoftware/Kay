#ifndef AKBUTTON_H
#define AKBUTTON_H

#include <AK/nodes/AKSubScene.h>
#include <AK/nodes/AKThreeImagePatch.h>
#include <AK/nodes/AKText.h>
#include <AK/nodes/AKContainer.h>

/**
 * @brief A button.
 * @ingroup AKNodes
 */
class AK::AKButton : public AKSubScene
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
    AKCLASS_NO_COPY(AKButton)

    void setText(const std::string &text) noexcept;

    void setBackgroundColor(SkColor color) noexcept;
    SkColor backgroundColor() const noexcept { return m_backgroundColor; };

    void setEnabled(bool enabled) noexcept;
    bool enabled() const noexcept { return m_enabled; }

    void setPressed(bool pressed) noexcept;
    bool pressed() const noexcept { return m_pressed; };

    AKSignal<const AKPointerButtonEvent&> onClick;

protected:
    void pointerButtonEvent(const AKPointerButtonEvent &event) override;
    void windowStateEvent(const AKWindowStateEvent &event) override;
    void layoutEvent(const AKLayoutEvent &event) override;
    void applyLayoutConstraints() noexcept;
    void updateOpaqueRegion() noexcept;
    void updateStyle() noexcept;
    AKThreeImagePatch m_hThreePatch { AKHorizontal, this };
    AKContainer m_content { YGFlexDirectionRow, true, &m_hThreePatch };
    AKText m_text;
    SkColor m_backgroundColor { SK_ColorWHITE };
    bool m_enabled { true };
    bool m_pressed { false };
};

#endif // AKBUTTON_H
