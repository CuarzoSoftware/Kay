#include <CZ/AK/Nodes/AKInfoRow.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AK.h>

using namespace CZ;

AKInfoRow::AKInfoRow(const std::string &labelText, const std::string &valueText, AKNode *parent) noexcept :
    AKContainer(YGFlexDirectionRow, false, parent)
{
    layout().setAlignItems(YGAlignFlexStart);
    layout().setGap(YGGutterColumn, Gap);
    layout().setFlexShrink(0.f);
    layout().setPadding(YGEdgeVertical, 1.5f);

    // The label ends where the value starts
    const SkRegion empty;
    labelColumn.setInputRegion(&empty);
    labelColumn.layout().setJustifyContent(YGJustifyFlexEnd);
    labelColumn.layout().setFlexShrink(0.f);
    labelColumn.layout().setWidth(m_labelWidth);

    label.setFontSize(FontSize);
    label.setTextColor(AKTheme::SecondaryLabel);
    label.setText(labelText);

    skia::textlayout::TextStyle style { theme()->DefaultTextStyle };
    style.setFontSize(FontSize);
    style.resetShadows();
    value.setTextStyle(style);
    value.setTextColor(AKTheme::Label);
    value.setSelectionColor(AKTheme::SelectedTextBackground);
    value.setText(valueText);
}

void AKInfoRow::setLabelWidth(SkScalar width) noexcept
{
    m_labelWidth = width;
    labelColumn.layout().setWidth(width);
}
