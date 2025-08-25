#ifndef CZ_AKTEXT_H
#define CZ_AKTEXT_H

#include <CZ/AK/Nodes/AKBakeable.h>

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
        CHLast
    };

    AKText(const std::string &text, AKNode *parent = nullptr) noexcept;
    CZ_DISABLE_COPY(AKText)

    bool setText(const std::string &text) noexcept;
    const std::string &text() const noexcept;
    const std::string &skText() const noexcept;

    bool setTextStyle(const skia::textlayout::TextStyle &textStyle) noexcept;
    const skia::textlayout::TextStyle &textStyle() const noexcept;

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
    std::string m_text, m_skText;
    std::vector<size_t> m_codePointByteOffsets;
    skia::textlayout::TextStyle m_textStyle, m_selectedStyle;
    skia::textlayout::ParagraphStyle m_paragraphStyle;
    std::unique_ptr<skia::textlayout::ParagraphBuilder> m_builder;
    std::unique_ptr<skia::textlayout::Paragraph> m_paragraph;
    size_t m_selection[2] { 0, 0 };
};

#endif // CZ_AKTEXT_H
