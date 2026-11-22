#include <CZ/AK/Nodes/AKActivityIndicator.h>
#include <CZ/AK/Events/AKBakeEvent.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/Core/Events/CZColorSchemeEvent.h>
#include <CZ/Ream/RPass.h>
#include <CZ/skia/core/SkCanvas.h>
#include <cmath>

using namespace CZ;

AKActivityIndicator::AKActivityIndicator(Int32 size, AKNode *parent) noexcept :
    AKBakeable(parent),
    m_color(AKTheme::SecondaryLabel)
{
    const SkRegion empty;
    setInputRegion(&empty);
    layout().setFlexShrink(0.f);
    setSize(size);
    m_timer.setCallback([this](CZTimer*){ step(); });
    start();
}

void AKActivityIndicator::setSize(Int32 size) noexcept
{
    layout().setWidth(std::max(size, 1));
    layout().setHeight(std::max(size, 1));
}

void AKActivityIndicator::start() noexcept
{
    if (m_running)
        return;

    m_running = true;
    m_timer.start(StepMs);
}

void AKActivityIndicator::stop() noexcept
{
    m_running = false;
    m_timer.stop();
}

void AKActivityIndicator::setSpokeColor(CZAdaptiveColor color) noexcept
{
    m_color = color;
    addChange(CHPhase);
    addDamage(AK_IRECT_INF);
}

void AKActivityIndicator::step() noexcept
{
    if (!m_running)
        return;

    // Only redrawn while shown (it keeps its phase meanwhile)
    if (visible())
    {
        m_phase = (m_phase + 1) % Spokes;
        addChange(CHPhase);
        addDamage(AK_IRECT_INF);
    }

    m_timer.start(StepMs);
}

void AKActivityIndicator::colorSchemeEvent(const CZColorSchemeEvent &e)
{
    AKBakeable::colorSchemeEvent(e);
    addChange(CHPhase);
    addDamage(AK_IRECT_INF);
}

void AKActivityIndicator::bakeEvent(const AKBakeEvent &e)
{
    if (e.damage.isEmpty() && !e.changes.test(CHPhase))
        return;

    auto pass { e.surface->beginPass(RPassCap_SkCanvas) };
    auto *c { pass->getCanvas() };
    const SkISize size { worldRect().size() };
    const SkScalar r { std::min(size.width(), size.height()) / 2.f };

    c->save();
    c->clipIRect(SkIRect::MakeSize(size));
    c->clear(SK_ColorTRANSPARENT);
    c->translate(size.width() / 2.f, size.height() / 2.f);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    paint.setStrokeWidth(std::max(1.f, r * 0.2f));

    const SkColor color { m_color.resolve(colorScheme()) };

    // Clockwise from the bright spoke, each one dimmer
    for (Int32 i = 0; i < Spokes; i++)
    {
        const Int32 age { (m_phase - i + Spokes) % Spokes };
        const SkScalar alpha { 1.f - 0.75f * SkScalar(age) / SkScalar(Spokes - 1) };
        paint.setColor(SkColorSetA(color, U8CPU(SkColorGetA(color) * alpha)));

        const SkScalar angle { SkScalar(i) * 2.f * SK_ScalarPI / SkScalar(Spokes) - SK_ScalarPI / 2.f };
        const SkScalar cs { std::cos(angle) }, sn { std::sin(angle) };
        c->drawLine(cs * r * 0.45f, sn * r * 0.45f, cs * (r - paint.getStrokeWidth() / 2.f), sn * (r - paint.getStrokeWidth() / 2.f), paint);
    }

    c->restore();
}
