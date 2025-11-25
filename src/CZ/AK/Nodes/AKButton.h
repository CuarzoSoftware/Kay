#ifndef CZ_AKBUTTON_H
#define CZ_AKBUTTON_H

#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Effects/AKBackgroundBoxShadowEffect.h>
#include <CZ/AK/Nodes/AKText.h>
#include <CZ/AK/Nodes/AKFontIcon.h>

/**
 * @brief A button.
 * @ingroup AKNodes
 */
class CZ::AKButton : public AKRoundSolidColor
{
public:
    enum Changes
    {
        CHEnabled = AKRoundSolidColor::CHLast,
        CHPressed,
        CHLast
    };

    enum class Type
    {
        Default = '\0',
        Toggle
    };

    enum class Variant
    {
        Elevated = '\0',
        Filled,
        Tonal,
        Outlined,
        Text
    };

    enum class Size
    {
        XSmall = '\0',
        Small,
        Medium,
        Large,
        XLarge
    };

    enum class Shape
    {
        Round = '\0',
        Square
    };

    AKButton(const std::string &text, AKNode *parent = nullptr) noexcept;

    AKText &textNode() noexcept { return m_text; }

    void setEnabled(bool enabled) noexcept;
    bool enabled() const noexcept { return m_enabled; }

    void setPressed(bool pressed) noexcept;
    bool pressed() const noexcept { return m_pressed; };

    CZSignal<const CZPointerButtonEvent&> onClick;

protected:
    void onSceneBegin() override;
    void pointerButtonEvent(const CZPointerButtonEvent &event) override;
    void windowStateEvent(const CZWindowStateEvent &event) override;
    void updateStyle() noexcept;
    void updateStyleFilled() noexcept;
    AKFontIcon m_icon;
    AKText m_text;
    std::shared_ptr<AKBackgroundBoxShadowEffect> m_shadow;
    bool m_enabled { true };
    bool m_pressed { false };
    bool m_selected { false };
    Type m_type { Type::Default };
    Variant m_variant { Variant::Filled };
    Size m_size { Size::Small };
    Shape m_shape { Shape::Round };
};

#endif // CZ_AKBUTTON_H
