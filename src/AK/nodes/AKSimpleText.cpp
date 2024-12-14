#include <include/core/SkCanvas.h>
#include <include/core/SkFontMetrics.h>
#include <AK/nodes/AKSimpleText.h>
#include <AK/AKSurface.h>

using namespace AK;

void AKSimpleText::onSceneBegin()
{
    if (changes().test(Chg_Text) || changes().test(Chg_Font))
        updateDimensions();
}

void AKSimpleText::onBake(OnBakeParams *params)
{
    if (text().empty())
        return;

    SkCanvas &c { *params->surface->surface()->getCanvas() };

    bool needsRepaint { changes().test(Chg_Text) || changes().test(Chg_Font) || changes().test(Chg_Brush) || changes().test(Chg_Pen) };

    if (!params->damage->isEmpty())
    {
        //params->surface->shrink();
        needsRepaint = true;
    }

    if (needsRepaint)
    {
        c.save();
        c.clipIRect(SkIRect::MakeSize(rect().size()));
        c.clear(SK_ColorTRANSPARENT);
        c.restore();

        if (brush().enabled)
            c.drawSimpleText(text().c_str(), text().size(), SkTextEncoding::kUTF8, -m_bounds.x(), -m_bounds.y(), font(), brush());

        if (pen().enabled)
            c.drawSimpleText(text().c_str(), text().size(), SkTextEncoding::kUTF8, -m_bounds.x(), -m_bounds.y(), font(), pen());

        params->damage->setRect(AK_IRECT_INF);
    }
}

void AKSimpleText::updateDimensions() noexcept
{
    font().measureText(text().c_str(), text().size(), SkTextEncoding::kUTF8, &m_bounds);
    m_bounds.outset(1.f, 1.f);
    layout().setWidth(m_bounds.width());
    layout().setMinWidth(m_bounds.width());
    layout().setMaxWidth(m_bounds.width());
    layout().setHeight(m_bounds.height());
    layout().setMinHeight(m_bounds.height());
    layout().setMaxHeight(m_bounds.height());
}
