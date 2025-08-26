#include <CZ/skia/core/SkCanvas.h>
#include <CZ/AK/Nodes/AKText.h>
#include <CZ/AK/Events/AKBakeEvent.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKLog.h>
#include <CZ/AK/AKApp.h>
#include <CZ/Ream/RSurface.h>
#include <CZ/Ream/RPass.h>
#include <locale>
#include <codecvt>

using namespace CZ;

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
    setTextStyle(theme()->DefaultTextStyle);
    setText(text);
    SkRegion empty;
    setInputRegion(&empty);
}

bool AKText::setText(const std::string &text) noexcept
{
    if (m_text == text)
        return false;

    m_text = text;
    m_skText = text;
    replaceAllInPlace(m_skText, "\t", "    ");
    updateCodePointByteOffsets();
    const size_t prevSelectionB { m_selection[1] };
    m_selection[0] = m_selection[1] = 0;
    addChange(CHText);
    updateDimensions();
    onTextChanged.notify();

    if (prevSelectionB != m_selection[1])
        onSelectionChanged.notify();
    return true;
}

bool AKText::setTextStyle(const skia::textlayout::TextStyle &textStyle) noexcept
{
    if (m_textStyle == textStyle)
        return false;

    m_textStyle = textStyle;
    m_selectedStyle = textStyle;
    SkPaint paint;
    paint.setColor(AKTheme::SystemCyan);
    m_selectedStyle.setBackgroundPaint(paint);
    paint.setColor(SK_ColorWHITE);
    paint.setAntiAlias(true);
    m_selectedStyle.setForegroundColor(paint);
    addChange(CHTextStyle);
    updateDimensions();
    return true;
}

const skia::textlayout::TextStyle &AKText::textStyle() const noexcept
{
    return m_textStyle;
}

const std::string &AKText::text() const noexcept
{
    return m_text;
}

const std::string &AKText::skText() const noexcept
{
    return m_skText;
}

void AKText::setSelection(size_t start, size_t count) noexcept
{
    //AKLog::debug("Selection %zu %zu", start, count);

    if (m_codePointByteOffsets.empty() || start >= m_codePointByteOffsets.size())
        start = count = 0;

    if (start + count > m_codePointByteOffsets.size())
        count = m_codePointByteOffsets.size() - start;

    if (m_selection[0] != start || m_selection[1] != count)
    {
        m_selection[0] = start;
        m_selection[1] = count;
        updateParagraph();
        addChange(CHSelection);
        addDamage(AK_IRECT_INF);
        onSelectionChanged.notify();
    }
}

const size_t *AKText::selection() const noexcept
{
    return m_selection;
}

int utf16ToCodePointIndex(const std::u16string& utf16Str, size_t utf16Index) {
    int codePointIndex = 0;

    for (size_t i = 0; i < utf16Index; ++i) {
        char16_t c = utf16Str[i];
        if (c >= 0xD800 && c <= 0xDBFF) {
            // High-surrogate, skip the next code unit
            ++i;
        }
        ++codePointIndex;
    }

    return codePointIndex;
}

size_t AKText::codePointAt(SkScalar x, SkScalar y) const noexcept
{
    if (!m_paragraph)
        return 0;

    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    std::u16string utf16Str = converter.from_bytes(m_skText);
    return utf16ToCodePointIndex(utf16Str, m_paragraph->getGlyphPositionAtCoordinate(x, y).position);
}

SkRect AKText::glyphAtCodePoint(size_t codePoint) const noexcept
{
    if (!m_paragraph)
        return {0.f, 0.f, 0.f, 0.f};

    skia::textlayout::Paragraph::GlyphInfo info;
    auto *par = (skia::textlayout::ParagraphImpl*)m_paragraph.get();
    if (codePoint > 0 && codePoint >= codePointByteOffsets().size())
        codePoint--;

    par->ensureUTF16Mapping();
    m_paragraph->getGlyphInfoAtUTF16Offset(par->getUTF16Index(codePointByteOffset(codePoint)), &info);
    return info.fGraphemeLayoutBounds;
}

size_t AKText::codePointByteOffset(size_t codePoint) const noexcept
{
    if (codePoint >= m_codePointByteOffsets.size())
        return m_skText.size();

    return m_codePointByteOffsets[codePoint];
}

const std::vector<size_t> &AKText::codePointByteOffsets() const noexcept
{
    return m_codePointByteOffsets;
}

void AKText::bakeEvent(const AKBakeEvent &e)
{
    if (e.damage.isEmpty() && !e.changes.testAnyOf(CHText, CHTextStyle, CHParagraphStyle, CHSelection))
        return;

    auto pass { e.surface->beginPass(RPassCap_SkCanvas) };
    auto *c { pass->getCanvas() };

    c->save();
    c->clipIRect(SkIRect::MakeSize(worldRect().size()));
    c->clear(SK_ColorTRANSPARENT);

    if (m_paragraph)
    {
        m_paragraph->paint(c, 0.f, 0.f);
        /*AKLog::debug("AKText repainted: Tint %d %dx%d Alpha Baseline %f Ideo Baseline %f %s",
                     customTextureColorEnabled(),
                     globalRect().width(), globalRect().height(),
                     m_paragraph->getAlphabeticBaseline(),
                     m_paragraph->getIdeographicBaseline(),
                     m_text.c_str());*/
    }
    c->restore();
}

void AKText::updateDimensions() noexcept
{
    if (m_text.empty())
    {
        layout().setWidthAuto();
        layout().setHeightAuto();
        return;
    }

    updateParagraph();
    layout().setWidth(SkScalarRoundToScalar(m_paragraph->getMaxIntrinsicWidth()));
    layout().setHeight(SkScalarRoundToScalar(m_paragraph->getHeight()));
    addDamage(AK_IRECT_INF);
}

void AKText::updateCodePointByteOffsets() noexcept
{
    m_codePointByteOffsets.clear();
    m_codePointByteOffsets.reserve(m_skText.size() * 4);
    size_t offset { 0 };

    for (size_t i = 0; i < m_skText.size(); ++i) {
        m_codePointByteOffsets.push_back(offset);

        if ((m_skText[i] & 0x80) == 0) { // 1-byte character (ASCII)
            offset += 1;
        } else if ((m_skText[i] & 0xE0) == 0xC0) { // 2-byte character
            offset += 2;
            i += 1; // Skip the continuation byte
        } else if ((m_skText[i] & 0xF0) == 0xE0) { // 3-byte character
            offset += 3;
            i += 2; // Skip the continuation bytes
        } else if ((m_skText[i] & 0xF8) == 0xF0) { // 4-byte character
            offset += 4;
            i += 3; // Skip the continuation bytes
        }
    }
}

void AKText::updateParagraph() noexcept
{
    m_builder = skia::textlayout::ParagraphBuilderImpl::make(m_paragraphStyle, AKApp::Get()->fontCollection());

    const size_t A { 0 };
    const size_t B { codePointByteOffset(m_selection[0]) };
    const size_t C { codePointByteOffset(m_selection[0] + m_selection[1]) };
    const size_t D { m_skText.size() };

    const std::string start { m_skText.substr(A, B) };
    const std::string middle { m_skText.substr(B, C - B) };
    const std::string end { m_skText.substr(C, D - C + 1) };

    if (!start.empty())
    {
        m_builder->pushStyle(m_textStyle);
        m_builder->addText(start.c_str());
        m_builder->pop();
    }

    if (!middle.empty())
    {
        m_builder->pushStyle(m_selectedStyle);
        m_builder->addText(middle.c_str());
        m_builder->pop();
    }

    if (!end.empty())
    {
        m_builder->pushStyle(m_textStyle);
        m_builder->addText(end.c_str());
        m_builder->pop();
    }

    // std::cout << "Text [" << m_selection[0] << "," << m_selection[1] << "](" << A << "," << B << "," << C << "," << D << "): " << start << "_" << middle << "_" << end << std::endl;

    m_paragraph = m_builder->Build();
    m_paragraph->layout(3000000);
}
