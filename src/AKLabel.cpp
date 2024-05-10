#include <AKLabel.h>
#include <AKPainter.h>
#include <private/AKFrame.h>
#include "include/core/SkFontMetrics.h"

using namespace AK;

AKLabel::AKLabel(const std::string &text, AKWidget *parent) noexcept : AKFrame(parent)
{
    m_text = text;
    m_font.setTypeface(SkTypeface::MakeFromName("Inter", SkFontStyle::Normal()));
    m_font.setSize(32);
    SkScalar width = m_font.measureText(m_text.c_str(), m_text.size(),
                               SkTextEncoding::kUTF8);

    SkFontMetrics metrics;
    m_font.getMetrics(&metrics);
    SkScalar height = metrics.fDescent - metrics.fAscent;
    setSize(AKSize(width, height));
}

void AKLabel::paintEvent(AKPainter &painter) noexcept
{
    AKWidget::paintEvent(painter);
    AKFrame *frame = (AKFrame*)painter.backend();
    SkPaint p;
    SkFontMetrics metrics;
    m_font.getMetrics(&metrics);
    frame->imp()->skiaSurface->getCanvas()->drawSimpleText(
        m_text.c_str(), m_text.size(),
        SkTextEncoding::kUTF8, 0, -metrics.fAscent,
        m_font, p);
}
