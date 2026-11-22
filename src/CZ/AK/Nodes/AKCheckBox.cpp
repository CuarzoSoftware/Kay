#include <CZ/AK/Nodes/AKCheckBox.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZPointerLeaveEvent.h>
#include <CZ/Core/Events/CZColorSchemeEvent.h>
#include <linux/input-event-codes.h>

using namespace CZ;

AKCheckBox::AKCheckBox(const std::string &labelText, bool checked, AKNode *parent) noexcept :
    AKContainer(YGFlexDirectionRow, false, parent),
    m_checked(checked)
{
    layout().setAlignItems(YGAlignCenter);
    layout().setGap(YGGutterColumn, 6.f);
    layout().setFlexShrink(0.f);
    setCursor(CZCursorShape::Default);

    // Clicks land on the container
    const SkRegion empty;
    box.setInputRegion(&empty);
    check.setInputRegion(&empty);
    label.setInputRegion(&empty);

    box.layout().setWidth(BoxSize);
    box.layout().setHeight(BoxSize);
    box.layout().setJustifyContent(YGJustifyCenter);
    box.layout().setAlignItems(YGAlignCenter);
    label.setFontSize(FontSize);
    setLabel(labelText);
    updateColors();
}

void AKCheckBox::setChecked(bool checked) noexcept
{
    m_checked = checked;
    updateColors();
}

void AKCheckBox::setEnabled(bool enabled) noexcept
{
    m_enabled = enabled;
    m_pressed = false;
    box.setOpacity(enabled ? 1.f : 0.4f);
    check.setOpacity(enabled ? 1.f : 0.4f);
    label.setOpacity(enabled ? 1.f : 0.4f);
    updateColors();
}

void AKCheckBox::setLabel(const std::string &text) noexcept
{
    label.setText(text);
    label.setVisible(!text.empty());
}

void AKCheckBox::pointerButtonEvent(const CZPointerButtonEvent &e)
{
    AKContainer::pointerButtonEvent(e);

    if (!m_enabled || e.button != BTN_LEFT)
        return;

    if (e.pressed)
        m_pressed = true;
    else if (m_pressed)
    {
        m_pressed = false;
        m_checked = !m_checked;
        updateColors();
        onToggled.notify(m_checked);
        return;
    }

    updateColors();
}

void AKCheckBox::pointerLeaveEvent(const CZPointerLeaveEvent &e)
{
    AKContainer::pointerLeaveEvent(e);
    m_pressed = false;
    updateColors();
}

void AKCheckBox::colorSchemeEvent(const CZColorSchemeEvent &e)
{
    AKContainer::colorSchemeEvent(e);
    updateColors();
}

void AKCheckBox::updateColors() noexcept
{
    const CZColorScheme scheme { colorScheme() };
    check.setVisible(m_checked);
    check.setColor(SK_ColorWHITE);
    label.setTextColor(AKTheme::Label);

    if (m_checked)
    {
        SkColor fill { AKTheme::ControlAccent.resolve(scheme) };

        if (m_pressed) // Darker while pressed
            fill = SkColorSetRGB(SkColorGetR(fill) * 4 / 5, SkColorGetG(fill) * 4 / 5, SkColorGetB(fill) * 4 / 5);

        box.setBackgroundColor(fill);
        box.setStrokeWidth(0);
    }
    else
    {
        box.setBackgroundColor(m_pressed ? AKTheme::SystemFill.resolve(scheme) : AKTheme::ControlBackground.resolve(scheme));
        box.setStrokeColor(AKTheme::TertiaryLabel.resolve(scheme));
        box.setStrokeWidth(1);
    }
}
