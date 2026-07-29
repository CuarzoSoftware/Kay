#ifndef CZ_AKTEXT_H
#define CZ_AKTEXT_H

#include <CZ/AK/Nodes/AKBakeable.h>
#include <CZ/Core/CZAdaptive.h>

#include <CZ/skia/modules/skparagraph/src/ParagraphImpl.h>
#include <CZ/skia/modules/skparagraph/src/ParagraphBuilderImpl.h>

/**
 * @brief Node for displaying text.
 * @ingroup AKNodes
 */
class CZ::AKText : public AKBakeable
{
public:
    enum Changes
    {
        CHParagraphStyle = AKBakeable::CHLast,
        CHTextStyle,
        CHText,
        CHSelection,
        CHFontSize,
        CHFontFamilies,
        CHFontStyle,
        CHColor,
        CHLetterSpacing,
        CHLineHeight,
        CHLast
    };

    AKText(const std::string &text, AKNode *parent = nullptr) noexcept;

    bool setText(const std::string &text) noexcept;
    const std::string &text() const noexcept;
    const std::string &skText() const noexcept;

    /**
     * @brief Replaces the whole text style at once.
     *
     * Prefer the individual setters below to tweak specific properties. Each of those touches only
     * one field of the single stored style and flags a dedicated change.
     */
    bool setTextStyle(const skia::textlayout::TextStyle &textStyle) noexcept;
    const skia::textlayout::TextStyle &textStyle() const noexcept;

    /** @brief Font size in logical pixels. */
    bool setFontSize(SkScalar fontSize) noexcept;
    SkScalar fontSize() const noexcept { return m_textStyle.getFontSize(); }

    /** @brief Ordered list of font families; the first available one is used. */
    bool setFontFamilies(std::vector<SkString> families) noexcept;
    const std::vector<SkString> &fontFamilies() const noexcept { return m_textStyle.getFontFamilies(); }

    /** @brief Weight, width and slant. */
    bool setFontStyle(SkFontStyle fontStyle) noexcept;
    SkFontStyle fontStyle() const noexcept { return m_textStyle.getFontStyle(); }

    /**
     * @brief Glyph color, as a @ref CZAdaptiveColor (distinct light/dark, resolved for the current
     *        color scheme and re-resolved automatically when it changes).
     *
     * @note Named setTextColor (not setColor) to avoid colliding with the node-level
     *       AKRenderable::setColor(), which — with enableReplaceImageColor — is a separate,
     *       lower-level recoloring of the baked image (e.g. how AKButton tints its label).
     */
    bool setTextColor(CZAdaptiveColor color) noexcept;
    CZAdaptiveColor textColor() const noexcept { return m_color; }

    /** @brief Extra spacing added between letters, in logical pixels. */
    bool setLetterSpacing(SkScalar letterSpacing) noexcept;
    SkScalar letterSpacing() const noexcept { return m_textStyle.getLetterSpacing(); }

    /** @brief Line height (as a multiple of the font size; only applied when height override is on). */
    bool setLineHeight(SkScalar lineHeight) noexcept;
    SkScalar lineHeight() const noexcept { return m_textStyle.getHeight(); }

    void setSelection(size_t start, size_t count) noexcept;
    const size_t *selection() const noexcept;

    size_t codePointAt(SkScalar x, SkScalar y) const noexcept;
    SkRect glyphAtCodePoint(size_t codePoint) const noexcept;

    /**
     * @brief Gets the byte offsets of each code point in the UTF-8 string.
     *
     * @note For bounds safety, use codePointByteOffset().
     */
    const std::vector<size_t> &codePointByteOffsets() const noexcept;

    /**
     * @brief Gets the byte offset of a specific code point in the UTF-8 string.
     *
     * @param codePoint The index of the code point.
     * @return The byte offset in the UTF-8 string.
     *         If codePoint >= codePointByteOffsets.size()
     *         it returns the length of the string.
     */
    size_t codePointByteOffset(size_t codePoint) const noexcept;

    CZSignal<> onSelectionChanged;
    CZSignal<> onTextChanged;

protected:
    void bakeEvent(const AKBakeEvent &event) override;
    void updateDimensions() noexcept;
    void updateCodePointByteOffsets() noexcept;
    void updateParagraph() noexcept;
    // Flags the change, re-lays out the paragraph and re-bakes. Called by every style setter.
    void applyStyleChange(Change change) noexcept;
    // Applies a resolved glyph color via the foreground paint (which overrides TextStyle::setColor),
    // keeping fColor in sync so getColor() reflects it.
    void applyTextColor(SkColor color) noexcept;
    // The style used for the selected run, derived from m_textStyle for the current color scheme.
    skia::textlayout::TextStyle selectedStyle() const noexcept;
    std::string m_text, m_skText;
    std::vector<size_t> m_codePointByteOffsets;
    // The single "to-apply" style. m_color is the adaptive source resolved into its glyph color.
    skia::textlayout::TextStyle m_textStyle;
    CZAdaptiveColor m_color;
    skia::textlayout::ParagraphStyle m_paragraphStyle;
    std::unique_ptr<skia::textlayout::ParagraphBuilder> m_builder;
    std::unique_ptr<skia::textlayout::Paragraph> m_paragraph;
    size_t m_selection[2] { 0, 0 };
};

#endif // CZ_AKTEXT_H
