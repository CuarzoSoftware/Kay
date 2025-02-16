#ifndef AKTEXT_H
#define AKTEXT_H

#include <AK/nodes/AKBakeable.h>

#include <modules/skparagraph/src/ParagraphImpl.h>
#include <modules/skparagraph/src/ParagraphBuilderImpl.h>

class AK::AKText : public AKBakeable
{
public:
    enum Changes
    {
        CHParagraphStyle = AKBakeable::Chg_Last,
        CHTextStyle,
        CHText,
        CHLast
    };

    AKText(const std::string &text, AKNode *parent = nullptr) noexcept;

    void setText(const std::string &text) noexcept;

    const std::string &text() const noexcept
    {
        return m_text;
    }

    bool setTextStyle(const skia::textlayout::TextStyle &textStyle) noexcept
    {
        if (m_textStyle == textStyle)
            return false;

        m_textStyle = textStyle;
        addChange(CHTextStyle);
        updateDimensions();
        return true;
    }

    const skia::textlayout::TextStyle &textStyle() const noexcept
    {
        return m_textStyle;
    }

protected:
    void onBake(OnBakeParams *p) override;
    void updateDimensions() noexcept;
    std::string m_text;
    skia::textlayout::TextStyle m_textStyle;
    skia::textlayout::ParagraphStyle m_paragraphStyle;
    sk_sp<skia::textlayout::FontCollection> m_collection;
    std::unique_ptr<skia::textlayout::ParagraphBuilder> m_builder;
    std::unique_ptr<skia::textlayout::Paragraph> m_paragraph;
};

#endif // AKTEXT_H
