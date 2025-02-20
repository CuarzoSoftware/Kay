#include <include/core/SkCanvas.h>
#include <include/core/SkFontMetrics.h>
#include <AK/nodes/AKSimpleText.h>
#include <AK/AKSurface.h>
#include <AK/events/AKPointerMoveEvent.h>
#include <AK/events/AKBakeEvent.h>

using namespace AK;

static void replaceAllInPlace(std::string &dst, const std::string &find, const std::string &replace) {
    size_t pos { 0 };
    while ((pos = dst.find(find, pos)) != std::string::npos) {
        dst.replace(pos, 1, replace);
        pos += replace.size();
    }
}

void AKSimpleText::setText(const std::string &text) noexcept
{
    if (m_text == text)
        return;

    m_text = text;
    m_skText = text;
    replaceAllInPlace(m_skText, "\t", "    ");
    addChange(CHText);
    updateDimensions();
    signalTextChanged.notify();
}

Int32 AKSimpleText::charIndexAtX(SkScalar x) const noexcept
{
    if (m_skText.empty())
        return -1;

    if (x <= 0)
        return 0;

    SkScalar width { 0.f };
    SkRect bounds;

    for (size_t i = 0; i < m_skText.size(); i++)
    {
        m_font.measureText(&m_skText[i], sizeof(m_skText[i]), SkTextEncoding::kUTF8, &bounds);
        width += bounds.width();
        if (width > x)
            return i;
    }

    return m_skText.size() - 1;
}

void AKSimpleText::bakeEvent(const AKBakeEvent &event)
{
    if (m_skText.empty())
        return;

    bool needsRepaint { event.changes.testAnyOf(CHText, CHFont, CHBrush, CHPen) };

    if (!event.damage.isEmpty())
    {
        //params->surface->shrink();
        needsRepaint = true;
    }

    if (needsRepaint)
    {
        SkCanvas &c { event.canvas() };
        c.save();
        c.clipIRect(SkIRect::MakeSize(globalRect().size()));
        c.clear(SK_ColorTRANSPARENT);
        c.restore();

        if (brush().enabled)
            c.drawSimpleText(m_skText.c_str(), text().size(), SkTextEncoding::kUTF8, -m_bounds.x(), -m_bounds.y(), font(), brush());

        if (pen().enabled)
            c.drawSimpleText(m_skText.c_str(), text().size(), SkTextEncoding::kUTF8, -m_bounds.x(), -m_bounds.y(), font(), pen());

        event.damage.setRect(AK_IRECT_INF);
    }
}

void AKSimpleText::updateDimensions() noexcept
{
    SkFontMetrics metrics;
    font().getMetrics(&metrics);
    m_bounds.fTop = metrics.fTop;
    m_bounds.fBottom = metrics.fBottom;
    m_bounds.fLeft = 0;
    m_bounds.fRight = font().measureText(m_skText.c_str(), m_skText.size(), SkTextEncoding::kUTF8);
    layout().setWidth(m_bounds.fRight);
    layout().setMinWidth(m_bounds.fRight);
    layout().setMaxWidth(m_bounds.fRight);
    layout().setHeight(m_bounds.height());
    layout().setMinHeight(m_bounds.height());
    layout().setMaxHeight(m_bounds.height());
}

