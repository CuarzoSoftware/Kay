#include <CZ/AK/Nodes/AKSearchField.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/Input/AKPointer.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZPointerMoveEvent.h>
#include <CZ/Core/Events/CZPointerEnterEvent.h>
#include <CZ/Core/Events/CZColorSchemeEvent.h>
#include <linux/input-event-codes.h>

using namespace CZ;
using namespace skia::textlayout;

AKSearchField::AKSearchField(const std::string &placeholderText, AKNode *parent) noexcept :
    AKRoundSolidColor(0, CZBorderRadius::Make(7), parent)
{
    layout().setFlexDirection(YGFlexDirectionRow);
    layout().setAlignItems(YGAlignCenter);
    layout().setHeight(Height);
    layout().setFlexShrink(0.f);
    layout().setPadding(YGEdgeLeft, 7.f);
    layout().setPadding(YGEdgeRight, 6.f);
    layout().setGap(YGGutterColumn, 5.f);
    enableChildrenClipping(true);
    setCursor(CZCursorShape::Text);

    const SkRegion empty;
    searchIcon.setInputRegion(&empty);
    searchIcon.layout().setFlexShrink(0.f);

    TextStyle style { theme()->DefaultTextStyle };
    style.setFontSize(FontSize);
    style.resetShadows();
    editor.setTextStyle(style);
    editor.setEditable(true);
    editor.setMaxLines(1);
    editor.layout().setFlex(1.f);

    // Exactly where the editor's text starts.
    placeholder.setInputRegion(&empty);
    placeholder.setFontSize(FontSize);
    placeholder.layout().setPositionType(YGPositionTypeAbsolute);
    placeholder.layout().setPosition(YGEdgeLeft, 7.f + 16.f + 5.f);
    placeholder.setText(placeholderText);

    // Clicks on it reach the field (see pointerButtonEvent()).
    clearButton.setInputRegion(&empty);
    clearButton.layout().setFlexShrink(0.f);

    editor.onTextChanged.subscribe(this, [this]{
        updateVisibility();
        onTextChanged.notify(editor.text());
    });

    editor.onSubmit.subscribe(this, [this]{ onSubmit.notify(editor.text()); });

    editor.onCancel.subscribe(this, [this]{
        if (editor.text().empty())
            editor.setKeyboardFocus(false);
        else
            clear();
    });

    updateColors();
    updateVisibility();
}

void AKSearchField::setText(const std::string &text) noexcept
{
    // The editor emits onTextChanged (see the constructor).
    editor.setText(text);
    editor.setCaret(text.size());
}

void AKSearchField::setFillColor(CZAdaptiveColor color) noexcept
{
    m_fillColor = color;
    updateColors();
}

void AKSearchField::pointerButtonEvent(const CZPointerButtonEvent &e)
{
    AKRoundSolidColor::pointerButtonEvent(e);

    if (e.button != BTN_LEFT || !e.pressed)
        return;

    // The clear button, or anywhere around the editor: focus it.
    if (overClearButton())
        clear();

    editor.setKeyboardFocus(true);
}

void AKSearchField::pointerEnterEvent(const CZPointerEnterEvent &e)
{
    AKRoundSolidColor::pointerEnterEvent(e);
    updateCursor();
}

void AKSearchField::pointerMoveEvent(const CZPointerMoveEvent &e)
{
    AKRoundSolidColor::pointerMoveEvent(e);
    updateCursor();
}

bool AKSearchField::overClearButton() const noexcept
{
    return clearButton.visible() && clearButton.worldRect().contains(akPointer().pos().x(), akPointer().pos().y());
}

void AKSearchField::updateCursor() noexcept
{
    // The clear button takes no input (clicks on it reach the field, see pointerButtonEvent()),
    // so the field's cursor is the one shown over it
    setCursor(overClearButton() ? CZCursorShape::Default : CZCursorShape::Text);
}

void AKSearchField::colorSchemeEvent(const CZColorSchemeEvent &e)
{
    AKRoundSolidColor::colorSchemeEvent(e);
    updateColors();
}

void AKSearchField::updateColors() noexcept
{
    const CZColorScheme scheme { colorScheme() };
    setBackgroundColor(m_fillColor.resolve(scheme));
    searchIcon.setColor(AKTheme::SecondaryLabel.resolve(scheme));
    clearButton.setColor(AKTheme::TertiaryLabel.resolve(scheme));
    editor.setTextColor(AKTheme::Label);
    editor.setSelectionColor(AKTheme::SelectedTextBackground);
    placeholder.setTextColor(AKTheme::PlaceholderText);
}

void AKSearchField::updateVisibility() noexcept
{
    const bool empty { editor.text().empty() };
    placeholder.setVisible(empty);
    clearButton.setVisible(!empty);
    updateCursor(); // E.g. cleared with the pointer over the button
}
