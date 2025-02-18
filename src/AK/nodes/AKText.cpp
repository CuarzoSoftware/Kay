#include <include/core/SkCanvas.h>
#include <AK/nodes/AKText.h>
#include <AK/AKSurface.h>
#include <AK/AKLog.h>

using namespace AK;

static void replaceAllInPlace(std::string &dst, const std::string &find, const std::string &replace) {
    size_t pos { 0 };
    while ((pos = dst.find(find, pos)) != std::string::npos) {
        dst.replace(pos, 1, replace);
        pos += replace.size();
    }
}


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

bool AKText::setText(const std::string &text) noexcept
{
    if (m_text == text)
        return false;

    m_text = text;
    m_skText = text;
    replaceAllInPlace(m_skText, "\t", "    ");
    addChange(CHText);
    updateDimensions();
    return true;
}

void AKText::onBake(const BakeEvent &event)
{
    if (!m_paragraph)
        return;

    SkCanvas &c { event.canvas() };

    if (event.damage.isEmpty() && !event.changes.testAnyOf(CHText, CHTextStyle, CHParagraphStyle))
        return;

    AKLog::debug("AKText repainted: Tint %d %dx%d %s", customTextureColorEnabled(), globalRect().width(), globalRect().height(), m_text.c_str());
    c.save();
    c.clipIRect(SkIRect::MakeSize(globalRect().size()));
    c.clear(SK_ColorTRANSPARENT);
    m_paragraph->paint(&c, 0.f, 0.f);
    c.restore();
}

void AKText::updateDimensions() noexcept
{
    m_builder = skia::textlayout::ParagraphBuilderImpl::make(m_paragraphStyle, m_collection);
    m_builder->pushStyle(m_textStyle);
    m_builder->addText(m_skText.c_str());
    m_builder->pop();
    m_paragraph = m_builder->Build();
    m_paragraph->layout(3000000);
    layout().setWidth(SkScalarRoundToScalar(m_paragraph->getMaxIntrinsicWidth()));
    layout().setHeight(SkScalarRoundToScalar(m_paragraph->getHeight()));
    layout().setMaxWidth(layout().width().value);
    layout().setMaxHeight(layout().height().value);
    layout().setMinWidth(layout().width().value);
    layout().setMinHeight(layout().height().value);
    addDamage(AK_IRECT_INF);
}
