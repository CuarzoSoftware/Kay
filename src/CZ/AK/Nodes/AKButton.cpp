#include <CZ/Core/Events/CZPointerEnterEvent.h>
#include <CZ/Core/Events/CZPointerLeaveEvent.h>
#include <CZ/Core/Events/CZPointerMoveEvent.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZWindowStateEvent.h>
#include <CZ/Core/Events/CZLayoutEvent.h>
#include <CZ/Core/Utils/CZColorUtils.h>
#include <CZ/Core/CZSpringAnimation.h>
#include <CZ/Core/CZColor.h>
#include <CZ/AK/AKColorTheme.h>
#include <CZ/AK/AKScene.h>
#include <CZ/AK/Nodes/AKButton.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKLog.h>

using namespace CZ;

AKButton::AKButton(const std::string &text, AKNode *parent) noexcept :
    AKRoundSolidColor(0x00000000, AKTheme::AKButtonBorderRadius, parent),
    m_icon("", 0, this),
    m_text(text, this)
{
    m_icon.setVisible(false);
    init();
}

AKButton::AKButton(const std::string &text, Type type, Variant variant, AKNode *parent) noexcept :
    AKRoundSolidColor(0x00000000, AKTheme::AKButtonBorderRadius, parent),
    m_icon("", 0, this),
    m_text(text, this),
    m_type(type),
    m_variant(variant)
{
    m_icon.setVisible(false);
    init();
}

AKButton::AKButton(const std::string &text, const std::string &icon, AKNode *parent) noexcept :
    AKRoundSolidColor(0x00000000, AKTheme::AKButtonBorderRadius, parent),
    m_icon(icon, 0, this),
    m_text(text, this)
{
    init();
}

void AKButton::init() noexcept
{
    m_borderRadiusAnimation.setOnUpdateCallback([this](auto *a){
        setBorderRadius(a->value());
        m_shadow.setBorderRadius(AKBorderRadius::Make(a->value()));
    });

    m_borderRadiusAnimation.setOnFinishCallback([this](auto *a){
        setBorderRadius(a->value());
        m_shadow.setBorderRadius(AKBorderRadius::Make(a->value()));
    });

    m_shadow.setOpacity(0.45f);
    const auto &scheme { colorTheme()->scheme(colorScheme()) };
    setColor(scheme.GetPrimary());
    SkRegion empty;
    setCursor(CZCursorShape::Pointer);
    layout().setJustifyContent(YGJustifyCenter);
    layout().setAlignItems(YGAlignCenter);
    layout().setFlexDirection(YGFlexDirectionRow);
    m_text.setInputRegion(&empty);
    m_text.enableReplaceImageColor(true);
    updateStyle();
}

void AKButton::setEnabled(bool enabled) noexcept
{
    if (m_enabled == enabled)
        return;

    setCursor(enabled ? CZCursorShape::Pointer : CZCursorShape::NotAllowed);
    m_enabled = enabled;
    addChange(CHEnabled);
    updateStyle();
}

void AKButton::setPressed(bool pressed) noexcept
{
    if (!enabled())
        return;

    if (m_pressed == pressed)
        return;

    m_pressed = pressed;
    addChange(CHPressed);

    if (type() == Type::Toggle && !pressed)
        setSelected(!selected());
    else
        updateStyle();
}

void AKButton::setSelected(bool selected) noexcept
{
    if (m_selected == selected)
        return;

    m_selected = selected;

    if (type() == Type::Toggle)
        updateStyle();
}

void AKButton::setHovered(bool hover) noexcept
{
    if (!enabled())
        hover = false;

    if (m_hovered == hover) return;
    m_hovered = hover;
    updateStyle();
}

void AKButton::setSize(Size size) noexcept
{
    if (m_size == size) return;
    m_size = size;
    addChange(CHSize);
    updateStyle();
}

void AKButton::setType(Type type) noexcept
{
    if (m_variant == Variant::Text)
        type = Type::Default;

    if (m_type == type)
        return;
    m_type = type;

    updateStyle();
}

const AKButton::ColorSpecs &AKButton::GetColorSpecs(Variant variant, Type type, bool enabled, bool hovered, bool focused, bool pressed, bool selected) noexcept
{
    enum State
    {
        Enabled,
        Disabled,
        Hovered,
        Focused,
        Pressed
    };

    const int A { static_cast<int>(variant) };

    int B { Disabled };

    if (enabled)
    {
        if (pressed)
            B = Pressed;
        else if (hovered)
            B = Hovered;
        else if (focused)
            B = Focused;
        else
            B = Enabled;
    }

    int C { 0 };

    if (type == Type::Toggle)
        C = selected ? 2 : 1;

    // [Variant][State][Toggle]
    static const ColorSpecs S[5][5][3]
    {
        { // Elevated
            { // Enabled
                { // Normal
                    .Container { .Color = AKColorRole::SurfaceContainerLow, .Opacity = 1.0, .Elevation = 1 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::Primary, .Opacity = 1.0 },
                    .Icon { .Color =  AKColorRole::Primary, .Opacity = 1.0 }
                },
                { // Unselected
                    .Container { .Color = AKColorRole::SurfaceContainerLow, .Opacity = 1.0, .Elevation = 1 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::Primary, .Opacity = 1.0 },
                    .Icon { .Color =  AKColorRole::Primary, .Opacity = 1.0 }
                },
                { // Selected
                    .Container { .Color = AKColorRole::Primary, .Opacity = 1.0, .Elevation = 1 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnPrimary, .Opacity = 1.0 },
                    .Icon { .Color =  AKColorRole::OnPrimary, .Opacity = 1.0 }
                }
            },
            { // Disabled
                { // Normal
                    .Container { .Color = AKColorRole::OnSurface, .Opacity = 0.1, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSurface, .Opacity = 0.38 },
                    .Icon { .Color =  AKColorRole::OnSurface, .Opacity = 0.38 }
                },
                { // Unselected
                    .Container { .Color = AKColorRole::OnSurface, .Opacity = 0.1, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSurface, .Opacity = 0.38 },
                    .Icon { .Color =  AKColorRole::OnSurface, .Opacity = 0.38 }
                },
                { // Selected
                    .Container { .Color = AKColorRole::OnSurface, .Opacity = 0.1, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSurface, .Opacity = 0.38 },
                    .Icon { .Color =  AKColorRole::OnSurface, .Opacity = 0.38 }
                }
            },
            { // Hovered
                { // Normal
                    .Container { S[0][0][0].Container },
                    .StateLayer { .Color = AKColorRole::Primary, .Opacity = 0.08 },
                    .Outline { S[0][0][0].Outline },
                    .Shadow { S[0][0][0].Shadow },
                    .Label { .Color = AKColorRole::Primary, .Opacity = S[0][0][0].Label.Opacity },
                    .Icon { .Color =  AKColorRole::Primary, .Opacity = S[0][0][0].Icon.Opacity }
                },
                { // Unselected
                    .Container { S[0][0][1].Container },
                    .StateLayer { .Color = AKColorRole::Primary, .Opacity = 0.08 },
                    .Outline { S[0][0][1].Outline },
                    .Shadow { S[0][0][1].Shadow },
                    .Label { .Color = AKColorRole::Primary, .Opacity = S[0][0][1].Label.Opacity },
                    .Icon { .Color =  AKColorRole::Primary, .Opacity = S[0][0][1].Icon.Opacity }
                },
                { // Selected
                    .Container { S[0][0][2].Container },
                    .StateLayer { .Color = AKColorRole::OnPrimary, .Opacity = 0.08 },
                    .Outline { S[0][0][2].Outline },
                    .Shadow { S[0][0][2].Shadow },
                    .Label { .Color = AKColorRole::OnPrimary, .Opacity = S[0][0][2].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnPrimary, .Opacity = S[0][0][2].Icon.Opacity }
                }
            },
            { // Focused
                { // Normal
                    .Container { S[0][0][0].Container },
                    .StateLayer { .Color = AKColorRole::Primary, .Opacity = 0.1 },
                    .Outline { S[0][0][0].Outline },
                    .Shadow { S[0][0][0].Shadow },
                    .Label { .Color = AKColorRole::Primary, .Opacity = S[0][0][0].Label.Opacity },
                    .Icon { .Color =  AKColorRole::Primary, .Opacity = S[0][0][0].Icon.Opacity }
                },
                { // Unselected
                    .Container { S[0][0][1].Container },
                    .StateLayer { .Color = AKColorRole::Primary, .Opacity = 0.1 },
                    .Outline { S[0][0][1].Outline },
                    .Shadow { S[0][0][1].Shadow },
                    .Label { .Color = AKColorRole::Primary, .Opacity = S[0][0][1].Label.Opacity },
                    .Icon { .Color =  AKColorRole::Primary, .Opacity = S[0][0][1].Icon.Opacity }
                },
                { // Selected
                    .Container { S[0][0][2].Container },
                    .StateLayer { .Color = AKColorRole::OnPrimary, .Opacity = 0.1 },
                    .Outline { S[0][0][2].Outline },
                    .Shadow { S[0][0][2].Shadow },
                    .Label { .Color = AKColorRole::OnPrimary, .Opacity = S[0][0][2].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnPrimary, .Opacity = S[0][0][2].Icon.Opacity }
                }
            },
            { // Pressed
                { // Normal
                    .Container { S[0][0][0].Container },
                    .StateLayer { .Color = AKColorRole::Primary, .Opacity = 0.1 },
                    .Outline { S[0][0][0].Outline },
                    .Shadow { S[0][0][0].Shadow },
                    .Label { .Color = AKColorRole::Primary, .Opacity = S[0][0][0].Label.Opacity },
                    .Icon { .Color =  AKColorRole::Primary, .Opacity = S[0][0][0].Icon.Opacity }
                },
                { // Unselected
                    .Container { S[0][0][1].Container },
                    .StateLayer { .Color = AKColorRole::Primary, .Opacity = 0.1 },
                    .Outline { S[0][0][1].Outline },
                    .Shadow { S[0][0][1].Shadow },
                    .Label { .Color = AKColorRole::Primary, .Opacity = S[0][0][1].Label.Opacity },
                    .Icon { .Color =  AKColorRole::Primary, .Opacity = S[0][0][1].Icon.Opacity }
                },
                { // Selected
                    .Container { S[0][0][2].Container },
                    .StateLayer { .Color = AKColorRole::OnPrimary, .Opacity = 0.1 },
                    .Outline { S[0][0][2].Outline },
                    .Shadow { S[0][0][2].Shadow },
                    .Label { .Color = AKColorRole::OnPrimary, .Opacity = S[0][0][2].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnPrimary, .Opacity = S[0][0][2].Icon.Opacity }
                }
            }
        },
        { // Filled
            { // Enabled
                { // Normal
                    .Container { .Color = AKColorRole::Primary, .Opacity = 1.0, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnPrimary, .Opacity = 1.0 },
                    .Icon { .Color =  AKColorRole::OnPrimary, .Opacity = 1.0 }
                },
                { // Unselected
                    .Container { .Color = AKColorRole::SurfaceContainer, .Opacity = 1.0, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSurfaceVariant, .Opacity = 1.0 },
                    .Icon { .Color =  AKColorRole::OnSurfaceVariant, .Opacity = 1.0 }
                },
                { // Selected
                    .Container { .Color = AKColorRole::Primary, .Opacity = 1.0, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnPrimary, .Opacity = 1.0 },
                    .Icon { .Color =  AKColorRole::OnPrimary, .Opacity = 1.0 }
                }
            },
            { // Disabled
                { // Normal
                    .Container { .Color = AKColorRole::OnSurface, .Opacity = 0.1, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSurface, .Opacity = 0.38 },
                    .Icon { .Color =  AKColorRole::OnSurface, .Opacity = 0.38 }
                },
                { // Unselected
                    .Container { .Color = AKColorRole::OnSurface, .Opacity = 0.1, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSurface, .Opacity = 0.38 },
                    .Icon { .Color =  AKColorRole::OnSurface, .Opacity = 0.38 }
                },
                { // Selected
                    .Container { .Color = AKColorRole::OnSurface, .Opacity = 0.1, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSurface, .Opacity = 0.38 },
                    .Icon { .Color =  AKColorRole::OnSurface, .Opacity = 0.38 }
                }
            },
            { // Hovered
                { // Normal
                    .Container { S[1][0][0].Container },
                    .StateLayer { .Color = AKColorRole::OnPrimary, .Opacity = 0.08 },
                    .Outline { S[1][0][0].Outline },
                    .Shadow { S[1][0][0].Shadow },
                    .Label { .Color = AKColorRole::OnPrimary, .Opacity = S[1][0][0].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnPrimary, .Opacity = S[1][0][0].Icon.Opacity }
                },
                { // Unselected
                    .Container { S[1][0][1].Container },
                    .StateLayer { .Color = AKColorRole::OnSurfaceVariant, .Opacity = 0.08 },
                    .Outline { S[1][0][1].Outline },
                    .Shadow { S[1][0][1].Shadow },
                    .Label { .Color = AKColorRole::OnSurfaceVariant, .Opacity = S[1][0][1].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnSurfaceVariant, .Opacity = S[1][0][1].Icon.Opacity }
                },
                { // Selected
                    .Container { S[1][0][2].Container },
                    .StateLayer { .Color = AKColorRole::OnPrimary, .Opacity = 0.08 },
                    .Outline { S[1][0][2].Outline },
                    .Shadow { S[1][0][2].Shadow },
                    .Label { .Color = AKColorRole::OnPrimary, .Opacity = S[1][0][2].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnPrimary, .Opacity = S[1][0][2].Icon.Opacity }
                }
            },
            { // Focused
                { // Normal
                    .Container { S[1][0][0].Container },
                    .StateLayer { .Color = AKColorRole::OnPrimary, .Opacity = 0.08 },
                    .Outline { S[1][0][0].Outline },
                    .Shadow { S[1][0][0].Shadow },
                    .Label { .Color = AKColorRole::OnPrimary, .Opacity = S[1][0][0].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnPrimary, .Opacity = S[1][0][0].Icon.Opacity }
                },
                { // Unselected
                    .Container { S[1][0][1].Container },
                    .StateLayer { .Color = AKColorRole::OnSurfaceVariant, .Opacity = 0.08 },
                    .Outline { S[1][0][1].Outline },
                    .Shadow { S[1][0][1].Shadow },
                    .Label { .Color = AKColorRole::OnSurfaceVariant, .Opacity = S[1][0][1].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnSurfaceVariant, .Opacity = S[1][0][1].Icon.Opacity }
                },
                { // Selected
                    .Container { S[1][0][2].Container },
                    .StateLayer { .Color = AKColorRole::OnPrimary, .Opacity = 0.08 },
                    .Outline { S[1][0][2].Outline },
                    .Shadow { S[1][0][2].Shadow },
                    .Label { .Color = AKColorRole::OnPrimary, .Opacity = S[1][0][2].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnPrimary, .Opacity = S[1][0][2].Icon.Opacity }
                }
            },
            { // Pressed
                { // Normal
                    .Container { S[1][0][0].Container },
                    .StateLayer { .Color = AKColorRole::OnPrimary, .Opacity = 0.1 },
                    .Outline { S[1][0][0].Outline },
                    .Shadow { S[1][0][0].Shadow },
                    .Label { .Color = AKColorRole::OnPrimary, .Opacity = S[1][0][0].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnPrimary, .Opacity = S[1][0][0].Icon.Opacity }
                },
                { // Unselected
                    .Container { S[1][0][1].Container },
                    .StateLayer { .Color = AKColorRole::OnSurfaceVariant, .Opacity = 0.1 },
                    .Outline { S[1][0][1].Outline },
                    .Shadow { S[1][0][1].Shadow },
                    .Label { .Color = AKColorRole::OnSurfaceVariant, .Opacity = S[1][0][1].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnSurfaceVariant, .Opacity = S[1][0][1].Icon.Opacity }
                },
                { // Selected
                    .Container { S[1][0][2].Container },
                    .StateLayer { .Color = AKColorRole::OnPrimary, .Opacity = 0.1 },
                    .Outline { S[1][0][2].Outline },
                    .Shadow { S[1][0][2].Shadow },
                    .Label { .Color = AKColorRole::OnPrimary, .Opacity = S[1][0][2].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnPrimary, .Opacity = S[1][0][2].Icon.Opacity }
                }
            }
        },
        { // Tonal
            { // Enabled
                { // Normal
                    .Container { .Color = AKColorRole::SecondaryContainer, .Opacity = 1.0, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSecondaryContainer, .Opacity = 1.0 },
                    .Icon { .Color =  AKColorRole::OnSecondaryContainer, .Opacity = 1.0 }
                },
                { // Unselected
                    .Container { .Color = AKColorRole::SecondaryContainer, .Opacity = 1.0, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSecondaryContainer, .Opacity = 1.0 },
                    .Icon { .Color =  AKColorRole::OnSecondaryContainer, .Opacity = 1.0 }
                },
                { // Selected
                    .Container { .Color = AKColorRole::Secondary, .Opacity = 1.0, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSecondary, .Opacity = 1.0 },
                    .Icon { .Color =  AKColorRole::OnSecondary, .Opacity = 1.0 }
                }
            },
            { // Disabled
                { // Normal
                    .Container { .Color = AKColorRole::OnSurface, .Opacity = 0.1, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSurface, .Opacity = 0.38 },
                    .Icon { .Color =  AKColorRole::OnSurface, .Opacity = 0.38 }
                },
                { // Unselected
                    .Container { .Color = AKColorRole::OnSurface, .Opacity = 0.1, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSurface, .Opacity = 0.38 },
                    .Icon { .Color =  AKColorRole::OnSurface, .Opacity = 0.38 }
                },
                { // Selected
                    .Container { .Color = AKColorRole::OnSurface, .Opacity = 0.1, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSurface, .Opacity = 0.38 },
                    .Icon { .Color =  AKColorRole::OnSurface, .Opacity = 0.38 }
                }
            },
            { // Hovered
                { // Normal
                    .Container { S[2][0][0].Container },
                    .StateLayer { .Color = AKColorRole::OnSecondaryContainer, .Opacity = 0.08 },
                    .Outline { S[2][0][0].Outline },
                    .Shadow { S[2][0][0].Shadow },
                    .Label { .Color = AKColorRole::OnSecondaryContainer, .Opacity = S[2][0][0].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnSecondaryContainer, .Opacity = S[2][0][0].Icon.Opacity }
                },
                { // Unselected
                    .Container { S[2][0][1].Container },
                    .StateLayer { .Color = AKColorRole::OnSecondaryContainer, .Opacity = 0.08 },
                    .Outline { S[2][0][1].Outline },
                    .Shadow { S[2][0][1].Shadow },
                    .Label { .Color = AKColorRole::OnSecondaryContainer, .Opacity = S[2][0][1].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnSecondaryContainer, .Opacity = S[2][0][1].Icon.Opacity }
                },
                { // Selected
                    .Container { S[2][0][2].Container },
                    .StateLayer { .Color = AKColorRole::OnSecondary, .Opacity = 0.08 },
                    .Outline { S[2][0][2].Outline },
                    .Shadow { S[2][0][2].Shadow },
                    .Label { .Color = AKColorRole::OnSecondary, .Opacity = S[2][0][2].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnSecondary, .Opacity = S[2][0][2].Icon.Opacity }
                }
            },
            { // Focused
                { // Normal
                    .Container { S[2][0][0].Container },
                    .StateLayer { .Color = AKColorRole::OnSecondaryContainer, .Opacity = 0.1 },
                    .Outline { S[2][0][0].Outline },
                    .Shadow { S[2][0][0].Shadow },
                    .Label { .Color = AKColorRole::OnSecondaryContainer, .Opacity = S[2][0][0].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnSecondaryContainer, .Opacity = S[2][0][0].Icon.Opacity }
                },
                { // Unselected
                    .Container { S[2][0][1].Container },
                    .StateLayer { .Color = AKColorRole::OnSecondaryContainer, .Opacity = 0.1 },
                    .Outline { S[2][0][1].Outline },
                    .Shadow { S[2][0][1].Shadow },
                    .Label { .Color = AKColorRole::OnSecondaryContainer, .Opacity = S[2][0][1].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnSecondaryContainer, .Opacity = S[2][0][1].Icon.Opacity }
                },
                { // Selected
                    .Container { S[2][0][2].Container },
                    .StateLayer { .Color = AKColorRole::OnSecondary, .Opacity = 0.1 },
                    .Outline { S[2][0][2].Outline },
                    .Shadow { S[2][0][2].Shadow },
                    .Label { .Color = AKColorRole::OnSecondary, .Opacity = S[2][0][2].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnSecondary, .Opacity = S[2][0][2].Icon.Opacity }
                }
            },
            { // Pressed
                { // Normal
                    .Container { S[2][0][0].Container },
                    .StateLayer { .Color = AKColorRole::OnSecondaryContainer, .Opacity = 0.1 },
                    .Outline { S[2][0][0].Outline },
                    .Shadow { S[2][0][0].Shadow },
                    .Label { .Color = AKColorRole::OnSecondaryContainer, .Opacity = S[2][0][0].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnSecondaryContainer, .Opacity = S[2][0][0].Icon.Opacity }
                },
                { // Unselected
                    .Container { S[2][0][1].Container },
                    .StateLayer { .Color = AKColorRole::OnSecondaryContainer, .Opacity = 0.1 },
                    .Outline { S[2][0][1].Outline },
                    .Shadow { S[2][0][1].Shadow },
                    .Label { .Color = AKColorRole::OnSecondaryContainer, .Opacity = S[2][0][1].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnSecondaryContainer, .Opacity = S[2][0][1].Icon.Opacity }
                },
                { // Selected
                    .Container { S[2][0][2].Container },
                    .StateLayer { .Color = AKColorRole::OnSecondaryContainer, .Opacity = 0.1 },
                    .Outline { S[2][0][2].Outline },
                    .Shadow { S[2][0][2].Shadow },
                    .Label { .Color = AKColorRole::OnSecondary, .Opacity = S[2][0][2].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnSecondary, .Opacity = S[2][0][2].Icon.Opacity }
                }
            }
        },
        { // Outlined
            { // Enabled
                { // Normal
                    .Container { .Color = AKColorRole::Invisible, .Opacity = 1.0, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::OutlineVariant, .Opacity = 1.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSurfaceVariant, .Opacity = 1.0 },
                    .Icon { .Color =  AKColorRole::OnSurfaceVariant, .Opacity = 1.0 }
                },
                { // Unselected
                    .Container { .Color = AKColorRole::Invisible, .Opacity = 1.0, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::OutlineVariant, .Opacity = 1.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSurfaceVariant, .Opacity = 1.0 },
                    .Icon { .Color =  AKColorRole::OnSurfaceVariant, .Opacity = 1.0 }
                },
                { // Selected
                    .Container { .Color = AKColorRole::Secondary, .Opacity = 1.0, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 1.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::InverseOnSurface, .Opacity = 1.0 },
                    .Icon { .Color =  AKColorRole::InverseOnSurface, .Opacity = 1.0 }
                }
            },
            { // Disabled
                { // Normal
                    .Container { .Color = AKColorRole::Invisible, .Opacity = 0.1, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::OutlineVariant, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSurface, .Opacity = 0.38 },
                    .Icon { .Color =  AKColorRole::OnSurface, .Opacity = 0.38 }
                },
                { // Unselected
                    .Container { .Color = AKColorRole::Invisible, .Opacity = 0.1, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::OutlineVariant, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSurface, .Opacity = 0.38 },
                    .Icon { .Color =  AKColorRole::OnSurface, .Opacity = 0.38 }
                },
                { // Selected
                    .Container { .Color = AKColorRole::OnSurface, .Opacity = 0.1, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSurface, .Opacity = 0.38 },
                    .Icon { .Color =  AKColorRole::OnSurface, .Opacity = 0.38 }
                }
            },
            { // Hovered
                { // Normal
                    .Container { S[3][0][0].Container },
                    .StateLayer { .Color = AKColorRole::OnSurfaceVariant, .Opacity = 0.08 },
                    .Outline { .Color = AKColorRole::OutlineVariant, .Opacity = 1.0 },
                    .Shadow { S[3][0][0].Shadow },
                    .Label { .Color = AKColorRole::OnSurfaceVariant, .Opacity = S[3][0][0].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnSurfaceVariant, .Opacity = S[3][0][0].Icon.Opacity }
                },
                { // Unselected
                    .Container { S[3][0][1].Container },
                    .StateLayer { .Color = AKColorRole::OnSurfaceVariant, .Opacity = 0.08 },
                    .Outline { .Color = AKColorRole::OutlineVariant, .Opacity = 1.0 },
                    .Shadow { S[3][0][1].Shadow },
                    .Label { .Color = AKColorRole::OnSurfaceVariant, .Opacity = S[3][0][1].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnSurfaceVariant, .Opacity = S[3][0][1].Icon.Opacity }
                },
                { // Selected
                    .Container { S[3][0][2].Container },
                    .StateLayer { .Color = AKColorRole::InverseOnSurface, .Opacity = 0.08 },
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 1.0 },
                    .Shadow { S[3][0][2].Shadow },
                    .Label { .Color = AKColorRole::InverseOnSurface, .Opacity = S[3][0][2].Label.Opacity },
                    .Icon { .Color =  AKColorRole::InverseOnSurface, .Opacity = S[3][0][2].Icon.Opacity }
                }
            },
            { // Focused
                { // Normal
                    .Container { S[3][0][0].Container },
                    .StateLayer { .Color = AKColorRole::OnSurfaceVariant, .Opacity = 0.1 },
                    .Outline { .Color = AKColorRole::OutlineVariant, .Opacity = 1.0 },
                    .Shadow { S[3][0][0].Shadow },
                    .Label { .Color = AKColorRole::OnSurfaceVariant, .Opacity = S[3][0][0].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnSurfaceVariant, .Opacity = S[3][0][0].Icon.Opacity }
                },
                { // Unselected
                    .Container { S[3][0][1].Container },
                    .StateLayer { .Color = AKColorRole::OnSurfaceVariant, .Opacity = 0.1 },
                    .Outline { .Color = AKColorRole::OutlineVariant, .Opacity = 1.0 },
                    .Shadow { S[3][0][1].Shadow },
                    .Label { .Color = AKColorRole::OnSurfaceVariant, .Opacity = S[3][0][1].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnSurfaceVariant, .Opacity = S[3][0][1].Icon.Opacity }
                },
                { // Selected
                    .Container { S[3][0][2].Container },
                    .StateLayer { .Color = AKColorRole::InverseOnSurface, .Opacity = 0.1 },
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 1.0 },
                    .Shadow { S[3][0][2].Shadow },
                    .Label { .Color = AKColorRole::InverseOnSurface, .Opacity = S[3][0][2].Label.Opacity },
                    .Icon { .Color =  AKColorRole::InverseOnSurface, .Opacity = S[3][0][2].Icon.Opacity }
                }
            },
            { // Pressed
                { // Normal
                    .Container { S[3][0][0].Container },
                    .StateLayer { .Color = AKColorRole::OnSurfaceVariant, .Opacity = 0.1 },
                    .Outline { .Color = AKColorRole::OutlineVariant, .Opacity = 1.0 },
                    .Shadow { S[3][0][0].Shadow },
                    .Label { .Color = AKColorRole::OnSurfaceVariant, .Opacity = S[3][0][0].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnSurfaceVariant, .Opacity = S[3][0][0].Icon.Opacity }
                },
                { // Unselected
                    .Container { S[3][0][1].Container },
                    .StateLayer { .Color = AKColorRole::OnSurfaceVariant, .Opacity = 0.1 },
                    .Outline { .Color = AKColorRole::OutlineVariant, .Opacity = 1.0 },
                    .Shadow { S[3][0][1].Shadow },
                    .Label { .Color = AKColorRole::OnSurfaceVariant, .Opacity = S[3][0][1].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnSurfaceVariant, .Opacity = S[3][0][1].Icon.Opacity }
                },
                { // Selected
                    .Container { S[3][0][2].Container },
                    .StateLayer { .Color = AKColorRole::InverseOnSurface, .Opacity = 0.1 },
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 1.0 },
                    .Shadow { S[3][0][2].Shadow },
                    .Label { .Color = AKColorRole::InverseOnSurface, .Opacity = S[3][0][2].Label.Opacity },
                    .Icon { .Color =  AKColorRole::InverseOnSurface, .Opacity = S[3][0][2].Icon.Opacity }
                }
            }
        },
        { // Text
            { // Enabled
                { // Normal
                    .Container { .Color = AKColorRole::Invisible, .Opacity = 1.0, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 1.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::Primary, .Opacity = 1.0 },
                    .Icon { .Color =  AKColorRole::Primary, .Opacity = 1.0 }
                },
                { // Unselected
                    .Container { .Color = AKColorRole::Invisible, .Opacity = 1.0, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 1.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::Primary, .Opacity = 1.0 },
                    .Icon { .Color =  AKColorRole::Primary, .Opacity = 1.0 }
                },
                { // Selected
                    .Container { .Color = AKColorRole::Invisible, .Opacity = 1.0, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 1.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::Primary, .Opacity = 1.0 },
                    .Icon { .Color =  AKColorRole::Primary, .Opacity = 1.0 }
                }
            },
            { // Disabled
                { // Normal
                    .Container { .Color = AKColorRole::OnSurface, .Opacity = 0.1, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSurface, .Opacity = 0.38 },
                    .Icon { .Color =  AKColorRole::OnSurface, .Opacity = 0.38 }
                },
                { // Unselected
                    .Container { .Color = AKColorRole::OnSurface, .Opacity = 0.1, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSurface, .Opacity = 0.38 },
                    .Icon { .Color =  AKColorRole::OnSurface, .Opacity = 0.38 }
                },
                { // Selected
                    .Container { .Color = AKColorRole::OnSurface, .Opacity = 0.1, .Elevation = 0 },
                    .StateLayer {},
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 0.0 },
                    .Shadow { .Color = AKColorRole::Shadow, .Opacity = 0.45 },
                    .Label { .Color = AKColorRole::OnSurface, .Opacity = 0.38 },
                    .Icon { .Color =  AKColorRole::OnSurface, .Opacity = 0.38 }
                }
            },
            { // Hovered
                { // Normal
                    .Container { S[4][0][0].Container },
                    .StateLayer { .Color = AKColorRole::Primary, .Opacity = 0.08 },
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 1.0 },
                    .Shadow { S[4][0][0].Shadow },
                    .Label { .Color = AKColorRole::Primary, .Opacity = S[4][0][0].Label.Opacity },
                    .Icon { .Color =  AKColorRole::Primary, .Opacity = S[4][0][0].Icon.Opacity }
                },
                { // Unselected
                    .Container { S[4][0][1].Container },
                    .StateLayer { .Color = AKColorRole::Primary, .Opacity = 0.08 },
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 1.0 },
                    .Shadow { S[4][0][1].Shadow },
                    .Label { .Color = AKColorRole::Primary, .Opacity = S[4][0][1].Label.Opacity },
                    .Icon { .Color =  AKColorRole::Primary, .Opacity = S[4][0][1].Icon.Opacity }
                },
                { // Selected
                    .Container { S[4][0][2].Container },
                    .StateLayer { .Color = AKColorRole::Primary, .Opacity = 0.08 },
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 1.0 },
                    .Shadow { S[4][0][2].Shadow },
                    .Label { .Color = AKColorRole::Primary, .Opacity = S[4][0][2].Label.Opacity },
                    .Icon { .Color =  AKColorRole::Primary, .Opacity = S[4][0][2].Icon.Opacity }
                }
            },
            { // Focused
                { // Normal
                    .Container { S[4][0][0].Container },
                    .StateLayer { .Color = AKColorRole::Primary, .Opacity = 0.1 },
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 1.0 },
                    .Shadow { S[4][0][0].Shadow },
                    .Label { .Color = AKColorRole::OnSurfaceVariant, .Opacity = S[4][0][0].Label.Opacity },
                    .Icon { .Color =  AKColorRole::OnSurfaceVariant, .Opacity = S[4][0][0].Icon.Opacity }
                },
                { // Unselected
                    .Container { S[4][0][1].Container },
                    .StateLayer { .Color = AKColorRole::Primary, .Opacity = 0.1 },
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 1.0 },
                    .Shadow { S[4][0][1].Shadow },
                    .Label { .Color = AKColorRole::Primary, .Opacity = S[4][0][1].Label.Opacity },
                    .Icon { .Color =  AKColorRole::Primary, .Opacity = S[4][0][1].Icon.Opacity }
                },
                { // Selected
                    .Container { S[4][0][2].Container },
                    .StateLayer { .Color = AKColorRole::Primary, .Opacity = 0.1 },
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 1.0 },
                    .Shadow { S[4][0][2].Shadow },
                    .Label { .Color = AKColorRole::Primary, .Opacity = S[4][0][2].Label.Opacity },
                    .Icon { .Color =  AKColorRole::Primary, .Opacity = S[4][0][2].Icon.Opacity }
                }
            },
            { // Pressed
                { // Normal
                    .Container { S[4][0][0].Container },
                    .StateLayer { .Color = AKColorRole::Primary, .Opacity = 0.1 },
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 1.0 },
                    .Shadow { S[4][0][0].Shadow },
                    .Label { .Color = AKColorRole::Primary, .Opacity = S[4][0][0].Label.Opacity },
                    .Icon { .Color =  AKColorRole::Primary, .Opacity = S[4][0][0].Icon.Opacity }
                },
                { // Unselected
                    .Container { S[4][0][1].Container },
                    .StateLayer { .Color = AKColorRole::Primary, .Opacity = 0.1 },
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 1.0 },
                    .Shadow { S[4][0][1].Shadow },
                    .Label { .Color = AKColorRole::Primary, .Opacity = S[4][0][1].Label.Opacity },
                    .Icon { .Color =  AKColorRole::Primary, .Opacity = S[4][0][1].Icon.Opacity }
                },
                { // Selected
                    .Container { S[4][0][2].Container },
                    .StateLayer { .Color = AKColorRole::Primary, .Opacity = 0.1 },
                    .Outline { .Color = AKColorRole::Invisible, .Opacity = 1.0 },
                    .Shadow { S[4][0][2].Shadow },
                    .Label { .Color = AKColorRole::Primary, .Opacity = S[4][0][2].Label.Opacity },
                    .Icon { .Color =  AKColorRole::Primary, .Opacity = S[4][0][2].Icon.Opacity }
                }
            }
        }
    };

    return S[A][B][C];
}

const AKButton::SizeSpecs &AKButton::GetSizeSpecs(Size size) noexcept
{
    static const SizeSpecs Specs[]
    {
        SizeSpecs { // XSmall
            .ContainerHeight = 32,
            .OutlineWidth = 1,
            .FontName = "Roboto",
            .FontWeight = SkFontStyle::kMedium_Weight,
            .FontSize = 14,
            .LineHeight = 20,
            .FontTracking = 0.1,
            .IconSize = 20,
            .ShapeRound = 16,
            .ShapeSquare = 12,
            .LeadingSpace = 12,
            .BetweenIconLabelSpace = 8,
            .TailingSpace = 12,
            .ShapePressedMorph = 8,
            .ShapeSpringAnimationDamping = 0.9,
            .ShapeSpringAnimationStiffness = 1400,
            .SelectedContainerShapeRound = 12,
            .SelectedContainerShapeSquare = 16
        },
        SizeSpecs { // Small
            .ContainerHeight = 40,
            .OutlineWidth = 1,
            .FontName = "Roboto",
            .FontWeight = SkFontStyle::kMedium_Weight,
            .FontSize = 14,
            .LineHeight = 20,
            .FontTracking = 0.1,
            .IconSize = 20,
            .ShapeRound = 20,
            .ShapeSquare = 12,
            .LeadingSpace = 16,
            .BetweenIconLabelSpace = 8,
            .TailingSpace = 16,
            .ShapePressedMorph = 8,
            .ShapeSpringAnimationDamping = 0.9,
            .ShapeSpringAnimationStiffness = 1400,
            .SelectedContainerShapeRound = 12,
            .SelectedContainerShapeSquare = 20
        },
        SizeSpecs { // Medium
            .ContainerHeight = 56,
            .OutlineWidth = 1,
            .FontName = "Roboto",
            .FontWeight = SkFontStyle::kMedium_Weight,
            .FontSize = 16,
            .LineHeight = 24,
            .FontTracking = 0.15,
            .IconSize = 24,
            .ShapeRound = 56/2,
            .ShapeSquare = 16,
            .LeadingSpace = 24,
            .BetweenIconLabelSpace = 8,
            .TailingSpace = 24,
            .ShapePressedMorph = 12,
            .ShapeSpringAnimationDamping = 0.9,
            .ShapeSpringAnimationStiffness = 1400,
            .SelectedContainerShapeRound = 16,
            .SelectedContainerShapeSquare = 56/2
        },
        SizeSpecs { // Large
            .ContainerHeight = 96,
            .OutlineWidth = 2,
            .FontName = "Roboto",
            .FontWeight = SkFontStyle::kNormal_Weight,
            .FontSize = 24,
            .LineHeight = 32,
            .FontTracking = 0.0,
            .IconSize = 32,
            .ShapeRound = 96/2,
            .ShapeSquare = 28,
            .LeadingSpace = 48,
            .BetweenIconLabelSpace = 12,
            .TailingSpace = 48,
            .ShapePressedMorph = 16,
            .ShapeSpringAnimationDamping = 0.9,
            .ShapeSpringAnimationStiffness = 1400,
            .SelectedContainerShapeRound = 28,
            .SelectedContainerShapeSquare = 96/2
        },
        SizeSpecs { // XLarge
            .ContainerHeight = 136,
            .OutlineWidth = 3,
            .FontName = "Roboto",
            .FontWeight = SkFontStyle::kNormal_Weight,
            .FontSize = 32,
            .LineHeight = 40,
            .FontTracking = 0.0,
            .IconSize = 40,
            .ShapeRound = 136/2,
            .ShapeSquare = 28,
            .LeadingSpace = 64,
            .BetweenIconLabelSpace = 16,
            .TailingSpace = 64,
            .ShapePressedMorph = 16,
            .ShapeSpringAnimationDamping = 0.9,
            .ShapeSpringAnimationStiffness = 1400,
            .SelectedContainerShapeRound = 28,
            .SelectedContainerShapeSquare = 136/2
        }
    };

    return Specs[(int)size];
}

void AKButton::onSceneBegin()
{
    AKRoundSolidColor::onSceneBegin();
}

void AKButton::pointerButtonEvent(const CZPointerButtonEvent &event)
{
    AKRoundSolidColor::pointerButtonEvent(event);

    if (event.button == BTN_LEFT)
    {
        const bool triggerOnClicked { !event.pressed && pressed() && isPointerOver() };
        setPressed(event.pressed);

        if (event.pressed)
        {
            if (scene())
                scene()->setPointerGrab(this);
        }
        else
        {
            if (scene() && scene()->pointerGrab() == this)
                scene()->setPointerGrab(nullptr);
        }

        if (triggerOnClicked)
            onClick.notify(event);

        event.accept();
    }
}

void AKButton::pointerEnterEvent(const CZPointerEnterEvent &event)
{
    AKRoundSolidColor::pointerEnterEvent(event);
    setHovered(true);
}

void AKButton::pointerLeaveEvent(const CZPointerLeaveEvent &event)
{
    AKRoundSolidColor::pointerLeaveEvent(event);
    setHovered(false);
}

void AKButton::windowStateEvent(const CZWindowStateEvent &event)
{
    AKRoundSolidColor::windowStateEvent(event);
    event.accept();
}

void AKButton::updateSize() noexcept
{
    const auto &s { GetSizeSpecs(size()) };

    // Container Height
    layout().setHeight(s.ContainerHeight);
    layout().setMaxHeight(s.ContainerHeight);
    layout().setMinHeight(s.ContainerHeight);

    // Outline Width
    setStrokeWidth(s.OutlineWidth);

    // Label

    auto textStyle { m_text.textStyle() };
    textStyle.setFontStyle(SkFontStyle(
        s.FontWeight,
        SkFontStyle::Width::kNormal_Width,
        SkFontStyle::Slant::kUpright_Slant));

    textStyle.setFontSize(s.FontSize);
    textStyle.setFontFamilies({SkString(s.FontName)});
    textStyle.setLetterSpacing(s.FontTracking);
    textStyle.setHeight(s.LineHeight);
    m_text.setTextStyle(textStyle);

    // Icon
    m_icon.setSize(s.IconSize);

    // Padding
    layout().setPadding(YGEdgeTop, 0);
    layout().setPadding(YGEdgeBottom, 0);
    layout().setPadding(YGEdgeLeft, s.LeadingSpace);
    layout().setPadding(YGEdgeRight, s.TailingSpace);

    // Gap
    layout().setGap(YGGutterAll, s.BetweenIconLabelSpace);

    // Shape
    updateShape();
}

void AKButton::updateShape() noexcept
{
    const auto &s { GetSizeSpecs(size()) };

    if (m_shape == Shape::Round)
    {
        if (pressed() && enabled())
            setBorderRadiusAnimated(0, s.ShapeSpringAnimationStiffness, s.ShapeSpringAnimationDamping, s.ShapePressedMorph);
        else
        {
            if (type() == Type::Toggle && selected())
                setBorderRadiusAnimated(0, s.ShapeSpringAnimationStiffness, s.ShapeSpringAnimationDamping, s.SelectedContainerShapeRound);
            else
                setBorderRadiusAnimated(0, s.ShapeSpringAnimationStiffness, s.ShapeSpringAnimationDamping, s.ShapeRound);
        }
    }
    else // Square
    {
        setBorderRadiusAnimated(0, s.ShapeSpringAnimationStiffness, s.ShapeSpringAnimationDamping, s.ShapeSquare);
    }
}

void AKButton::updateColor() noexcept
{
    const auto &scheme { colorTheme()->scheme(colorScheme()) };
    const auto &spec { GetColorSpecs(m_variant, type(), enabled(), hovered(), false, pressed(), selected()) };

    // Container
    setOpacity(spec.Container.Opacity);
    setElevation(spec.Container.Elevation);
    if (spec.StateLayer.Color == AKColorRole::Invisible)
        setContainerColorAnimated(backgroundColor(), scheme.GetColor(spec.Container.Color));
    else
        setContainerColorAnimated(backgroundColor(),
            CZColorUtils::StateSrcOver(
                scheme.GetColor(spec.Container.Color),
                scheme.GetColor(spec.StateLayer.Color),
                spec.StateLayer.Opacity));

    // Outline
    const auto strokeColor { scheme.GetColor(spec.Outline.Color) };
    setStrokeColor(SkColorSetA(strokeColor, SkColorGetA(strokeColor) * spec.Outline.Opacity));

    // Shadow
    m_shadow.setOpacity(spec.Shadow.Opacity);
    m_shadow.setColor(scheme.GetColor(spec.Shadow.Color));

    // Label
    m_text.setColor(scheme.GetColor(spec.Label.Color));
    m_text.setOpacity(spec.Label.Opacity);

    // Icon
    m_icon.setColor(scheme.GetColor(spec.Icon.Color));
    m_icon.setOpacity(spec.Icon.Opacity);
}

void AKButton::setElevation(Int32 elevation) noexcept
{
    if (elevation <= 0)
        m_shadow.setVisible(false);
    else
    {
        m_shadow.setVisible(true);
        m_shadow.setRadius(elevation * 5);
    }
}

void AKButton::setBorderRadiusAnimated(Float64 velocity, Float64 stiffness, Float64 dampRatio, Int32 borderRadius) noexcept
{
    if (this->borderRadius() == borderRadius)
        return;

    m_borderRadiusAnimation.setStiffness(stiffness);
    m_borderRadiusAnimation.setDampingRatio(dampRatio);

    if (m_borderRadiusAnimation.isRunning())
        m_borderRadiusAnimation.setTo(borderRadius);
    else
    {
        m_borderRadiusAnimation.setVelocity(velocity);
        m_borderRadiusAnimation.setFrom(this->borderRadius());
        m_borderRadiusAnimation.setTo(borderRadius);
        m_borderRadiusAnimation.start();
    }
}

void AKButton::setContainerColorAnimated(SkColor A, SkColor B) noexcept
{
    m_containerColorAnimation.stop();

    if (A == B) return;

    m_containerColorAnimation.setValue(0.0);
    m_containerColorAnimation.setVelocity(0.0);
    m_containerColorAnimation.setFrom(0.0);
    m_containerColorAnimation.setTo(1.0);
    m_containerColorAnimation.setDampingRatio(CZSpringAnimation::DampingRatioNoBouncy);
    m_containerColorAnimation.setStiffness(CZSpringAnimation::StiffnessMedium);

    m_containerColorAnimation.setOnUpdateCallback([this, A, B](auto *a){
        setBackgroundColor(CZColorUtils::Mix(B, A, a->value()));
    });

    m_containerColorAnimation.setOnFinishCallback([this, B](auto){
        setBackgroundColor(B);
    });

    m_containerColorAnimation.start();
}

