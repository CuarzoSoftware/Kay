#include <CZ/AK/Nodes/AKBlurImage.h>
#include <CZ/AK/Events/AKBakeEvent.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Ream/RPass.h>
#include <CZ/skia/core/SkCanvas.h>
#include <CZ/skia/core/SkImage.h>
#include <CZ/skia/effects/SkImageFilters.h>
#include <algorithm>

using namespace CZ;

AKBlurImage::AKBlurImage(std::shared_ptr<RImage> image, AKNode *parent) noexcept :
    AKBakeable(parent), m_image(image) {}

bool AKBlurImage::setImage(std::shared_ptr<RImage> image) noexcept
{
    if (image.get() == m_image.get())
        return false;

    m_image = image;
    addChange(CHImage);
    addDamage(AK_IRECT_INF);
    return true;
}

bool AKBlurImage::setBlurRadius(SkScalar radius) noexcept
{
    radius = std::max(0.f, radius);

    if (radius == m_blurRadius)
        return false;

    m_blurRadius = radius;
    addChange(CHBlurRadius);
    addDamage(AK_IRECT_INF);
    return true;
}

bool AKBlurImage::setSizeMode(SizeMode mode) noexcept
{
    if (mode == m_sizeMode)
        return false;

    m_sizeMode = mode;
    addChange(CHSizeMode);
    addDamage(AK_IRECT_INF);
    return true;
}

bool AKBlurImage::setBackgroundColor(SkColor color) noexcept
{
    if (color == m_backgroundColor)
        return false;

    m_backgroundColor = color;
    addChange(CHBackgroundColor);
    addDamage(AK_IRECT_INF);
    return true;
}

void AKBlurImage::bakeEvent(const AKBakeEvent &e)
{
    // Opaque only if the background is (and the node isn't faded): nothing behind it is drawn
    const SkIRect bounds { SkIRect::MakeSize(worldRect().size()) };
    e.opaque.setEmpty();

    if (SkColorGetA(m_backgroundColor) == 255 && opacity() >= 1.f)
        e.opaque.setRect(bounds);

    if (e.damage.isEmpty() && !e.changes.testAnyOf(CHImage, CHBlurRadius, CHSizeMode, CHBackgroundColor, CHSize))
        return;

    auto pass { e.surface->beginPass(RPassCap_SkCanvas) };

    if (!pass)
        return;

    SkCanvas &c { *pass->getCanvas() };
    c.save();
    c.clipIRect(bounds);
    c.clear(m_backgroundColor);

    const sk_sp<SkImage> image { m_image ? m_image->skImage() : nullptr };

    if (image && !bounds.isEmpty())
    {
        const SkRect src { SkRect::MakeIWH(image->width(), image->height()) };
        const SkRect area { SkRect::Make(bounds) };
        SkRect dst { area };

        if (m_sizeMode != SizeMode::Fill)
        {
            const SkScalar sx { area.width() / src.width() }, sy { area.height() / src.height() };
            const SkScalar s { m_sizeMode == SizeMode::Cover ? std::max(sx, sy) : std::min(sx, sy) };
            const SkScalar w { src.width() * s }, h { src.height() * s };
            dst = SkRect::MakeXYWH(area.centerX() - w / 2.f, area.centerY() - h / 2.f, w, h);
        }

        SkPaint paint;
        paint.setAntiAlias(true);

        // Clamped: the edges keep their color instead of fading out (in logical units: the canvas
        // is scaled, and so is the filter)
        if (m_blurRadius > 0.01f)
            paint.setImageFilter(SkImageFilters::Blur(m_blurRadius, m_blurRadius, SkTileMode::kClamp, nullptr));

        c.drawImageRect(image, src, dst, SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone),
                        &paint, SkCanvas::kFast_SrcRectConstraint);
    }

    c.restore();
    e.damage.op(bounds, SkRegion::kUnion_Op);
}
