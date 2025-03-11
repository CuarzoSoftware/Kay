#include <AK/events/AKBakeEvent.h>
#include <AK/AKAnimation.h>
#include <ProgressBar.h>
#include <AK/AKBrush.h>
#include <AK/AKPen.h>

ProgressBar::ProgressBar(AKNode *parent) noexcept : AKBakeable(parent)
{
    layout().setMargin(YGEdgeTop, 40.f);
    layout().setHeight(10);
    layout().setWidth(256);

    AKAnimation::OneShot(duration, [this](AKAnimation *anim){
        setPercent(std::pow(anim->value(), 3.f));
    });
}

bool ProgressBar::setPercent(SkScalar percent) noexcept
{
    if (percent < 0.f) percent = 0.f;
    else if (percent > 1.f) percent = 1.f;

    if (percent == m_percent)
        return false;

    m_percent = percent;
    addChange(CHPercent);
    return true;
}

void ProgressBar::bakeEvent(const AKBakeEvent &e)
{
    if (!e.changes.test(CHPercent) && e.damage.isEmpty())
        return;

    SkCanvas &c { e.canvas() };
    AKPen pen;
    AKBrush brush;
    constexpr SkScalar borderRadius { 4.f };
    constexpr SkScalar strokeWidth { 0.5f };
    SkRect rect { SkRect::MakeWH(globalRect().width(), globalRect().height()) };
    rect.inset(strokeWidth, strokeWidth);

    c.clear(SK_ColorTRANSPARENT);

    // Outline
    pen.setColor(SK_ColorWHITE);
    pen.setStrokeWidth(strokeWidth);
    pen.setAntiAlias(true);
    c.drawRoundRect(rect, borderRadius, borderRadius, pen);
    c.drawRoundRect(rect, borderRadius, borderRadius, pen);

    // Fill
    c.save();
    brush.setColor(SK_ColorWHITE);
    brush.setAntiAlias(true);
    c.clipRect(SkRect(rect.fLeft, rect.fTop, rect.fRight * percent(), rect.fBottom));
    c.drawRoundRect(rect, borderRadius, borderRadius, brush);
    c.restore();

    // Damage everything
    e.damage.setRect(AK_IRECT_INF);
}
