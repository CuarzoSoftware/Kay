#ifndef CZ_AKBUTTON_H
#define CZ_AKBUTTON_H

#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Effects/AKBackgroundBoxShadowEffect.h>
#include <CZ/AK/Nodes/AKText.h>
#include <CZ/AK/Nodes/AKFontIcon.h>
#include <CZ/skia/core/SkFontStyle.h>
#include <CZ/AK/AKColorRole.h>
#include <CZ/Core/CZSpringAnimation.h>

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
        CHSize,
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
    AKButton(const std::string &text, Type type, Variant variant, AKNode *parent = nullptr) noexcept;
    AKButton(const std::string &text, const std::string &icon, AKNode *parent = nullptr) noexcept;

    AKText &textNode() noexcept { return m_text; }
    AKFontIcon &icon() noexcept { return m_icon; }

    void setEnabled(bool enabled) noexcept;
    bool enabled() const noexcept { return m_enabled; }

    void setPressed(bool pressed) noexcept;
    bool pressed() const noexcept { return m_pressed; };

    void setSelected(bool selected) noexcept;
    bool selected() const noexcept { return m_selected; }

    void setHovered(bool hover) noexcept;
    bool hovered() const noexcept { return m_hovered; }

    void setSize(Size size) noexcept;
    Size size() const noexcept { return m_size; }

    void setType(Type type) noexcept;
    Type type() const noexcept { return m_type; }

    CZSignal<const CZPointerButtonEvent&> onClick;

    /* Material 3 size specs */
    struct SizeSpecs
    {
        Int32 ContainerHeight;
        Int32 OutlineWidth;
        const char *FontName;
        SkFontStyle::Weight FontWeight;
        Int32 FontSize;
        Int32 LineHeight;
        Float64 FontTracking;
        Int32 IconSize;
        Int32 ShapeRound;
        Int32 ShapeSquare;
        Int32 LeadingSpace;
        Int32 BetweenIconLabelSpace;
        Int32 TailingSpace;
        Int32 ShapePressedMorph;
        Float64 ShapeSpringAnimationDamping;
        Float64 ShapeSpringAnimationStiffness;
        Int32 SelectedContainerShapeRound;
        Int32 SelectedContainerShapeSquare;
    };

    /* Material 3 color specs */
    struct ColorSpecs
    {
        struct {
            AKColorRole Color;
            Float64 Opacity;
            Int32 Elevation;
        } Container;

        struct {
            AKColorRole Color { AKColorRole::Invisible };
            Float64 Opacity { 0.f };
        } StateLayer;

        struct {
            AKColorRole Color;
            Float64 Opacity;
        } Outline;

        struct {
            AKColorRole Color;
            Float64 Opacity;
        } Shadow;

        struct {
            AKColorRole Color;
            Float64 Opacity;
        } Label;

        struct {
            AKColorRole Color;
            Float64 Opacity;
        } Icon;
    };

    static const SizeSpecs &GetSizeSpecs(Size size) noexcept;
    static const ColorSpecs &GetColorSpecs(Variant variant, Type type, bool enabled, bool hovered, bool focused, bool pressed, bool selected) noexcept;

protected:
    using AKRoundSolidColor::setBackgroundColor;
    void init() noexcept;
    void onSceneBegin() override;
    void pointerButtonEvent(const CZPointerButtonEvent &event) override;
    void pointerEnterEvent(const CZPointerEnterEvent &event) override;
    void pointerLeaveEvent(const CZPointerLeaveEvent &event) override;
    void windowStateEvent(const CZWindowStateEvent &event) override;
    void updateStyle() noexcept
    {
        updateSize();
        updateColor();
    }
    void updateSize() noexcept;
    void updateShape() noexcept;
    void updateColor() noexcept;
    void setElevation(Int32 elevation) noexcept;

    AKFontIcon m_icon;
    AKText m_text;
    AKBackgroundBoxShadowEffect m_shadow { 0, {0, 1}, SK_ColorBLACK, false, this };
    bool m_enabled { true };
    bool m_pressed { false };
    bool m_selected { false };
    bool m_hovered { false };
    Type m_type { Type::Default };
    Variant m_variant { Variant::Filled };
    Size m_size { Size::XSmall };
    Shape m_shape { Shape::Round };

    void setBorderRadiusAnimated(Float64 velocity, Float64 stiffness, Float64 dampRatio, Int32 borderRadius) noexcept;
    void setContainerColorAnimated(SkColor A, SkColor B) noexcept;
    CZSpringAnimation m_borderRadiusAnimation;
    CZSpringAnimation m_containerColorAnimation;
};

#endif // CZ_AKBUTTON_H
