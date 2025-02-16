#include <include/core/SkCanvas.h>
#include <AK/nodes/AKText.h>
#include <AK/AKSurface.h>
#include <AK/AKLog.h>

using namespace AK;

AKText::AKText(const std::string &text, AKNode *parent) noexcept : AKBakeable(parent)
{
    m_paragraphStyle.setTextDirection(skia::textlayout::TextDirection::kLtr);
    m_collection = sk_make_sp<skia::textlayout::FontCollection>();
    m_collection->setDefaultFontManager(AKFontManager());
    m_collection->enableFontFallback();
    m_textStyle.setFontSize(12);
    m_textStyle.setColor(SK_ColorBLACK);
    std::vector<SkString> fams;
    fams.push_back(SkString("Inter"));
    m_textStyle.setFontFamilies(fams);
    setText(text);
}

void AKText::setText(const std::string &text) noexcept
{
    if (m_text == text)
        return;

    m_text = text;
    addChange(CHText);
    updateDimensions();
}

void AKText::onBake(OnBakeParams *p)
{
    if (!m_paragraph)
        return;
    const auto &ch { changes() };

    if (p->damage->isEmpty() && !ch.test(CHText) && !ch.test(CHTextStyle) && !ch.test(CHParagraphStyle))
        return;

    AKLog::debug("AKText repainted: Tint %d %dx%d %s", customTextureColorEnabled(), globalRect().width(), globalRect().height(), m_text.c_str());
    SkCanvas &c { *p->surface->surface()->getCanvas() };
    c.save();
    c.clipIRect(SkIRect::MakeSize(globalRect().size()));
    c.clear(SK_ColorTRANSPARENT);
    m_paragraph->paint(p->surface->surface()->getCanvas(), 0.f, 0.f);
    c.restore();
}

void AKText::updateDimensions() noexcept
{
    m_builder = skia::textlayout::ParagraphBuilderImpl::make(m_paragraphStyle, m_collection);
    m_builder->pushStyle(m_textStyle);
    m_builder->addText(m_text.c_str());
    m_builder->pop();
    m_paragraph = m_builder->Build();
    m_paragraph->layout(3000000);
    layout().setWidth(m_paragraph->getLongestLine());
    layout().setHeight(m_paragraph->getHeight());
    layout().setMaxWidth(layout().width().value);
    layout().setMaxHeight(layout().height().value);
    layout().setMinWidth(layout().width().value);
    layout().setMinHeight(layout().height().value);
    addDamage(AK_IRECT_INF);
}
