#include <CZ/AK/Nodes/AKTextInput.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZColorSchemeEvent.h>
#include <linux/input-event-codes.h>

using namespace CZ;
using namespace skia::textlayout;

static constexpr SkScalar PaddingX { 6.f };

AKTextInput::AKTextInput(const std::string &placeholderText, AKNode *parent) noexcept :
    AKRoundSolidColor(0, CZBorderRadius::Make(5), parent)
{
    layout().setFlexDirection(YGFlexDirectionRow);
    layout().setAlignItems(YGAlignCenter);
    layout().setHeight(Height);
    layout().setPadding(YGEdgeHorizontal, PaddingX);
    layout().setFlexShrink(0.f);
    enableChildrenClipping(true);
    setStrokeWidth(1);
    setCursor(CZCursorShape::Text);

    TextStyle style { theme()->DefaultTextStyle };
    style.setFontSize(FontSize);
    style.resetShadows();
    editor.setTextStyle(style);
    editor.setEditable(true);
    editor.setMaxLines(1);
    editor.layout().setFlex(1.f);

    const SkRegion empty;
    placeholder.setInputRegion(&empty);
    placeholder.setFontSize(FontSize);
    placeholder.layout().setPositionType(YGPositionTypeAbsolute);
    placeholder.layout().setPosition(YGEdgeLeft, PaddingX);
    placeholder.setText(placeholderText);

    editor.onTextChanged.subscribe(this, [this]{
        placeholder.setVisible(editor.text().empty());
        onTextChanged.notify(editor.text());
    });
    editor.onSubmit.subscribe(this, [this]{ onSubmit.notify(editor.text()); });
    editor.onCancel.subscribe(this, [this]{ onCancel.notify(); });
    editor.onEditingFinished.subscribe(this, [this]{ updateColors(); });
    editor.onSelectionChanged.subscribe(this, [this]{ updateColors(); }); // Focused: the caret moved there

    updateColors();
}

void AKTextInput::setText(const std::string &text) noexcept
{
    editor.setText(text);
    editor.setCaret(text.size());
    placeholder.setVisible(text.empty());
}

void AKTextInput::pointerButtonEvent(const CZPointerButtonEvent &e)
{
    AKRoundSolidColor::pointerButtonEvent(e);

    if (e.button == BTN_LEFT && e.pressed)
    {
        editor.setKeyboardFocus(true);
        updateColors();
    }
}

void AKTextInput::colorSchemeEvent(const CZColorSchemeEvent &e)
{
    AKRoundSolidColor::colorSchemeEvent(e);
    updateColors();
}

void AKTextInput::updateColors() noexcept
{
    const CZColorScheme scheme { colorScheme() };
    const bool focused { editor.editing() };
    setBackgroundColor(AKTheme::TextBackground.resolve(scheme));
    setStrokeColor(focused ? AKTheme::KeyboardFocusIndicator.resolve(scheme) : AKTheme::OpaqueSeparator.resolve(scheme));
    setStrokeWidth(focused ? 2 : 1);
    editor.setTextColor(AKTheme::Text);
    editor.setSelectionColor(AKTheme::SelectedTextBackground);
    placeholder.setTextColor(AKTheme::PlaceholderText);
}
