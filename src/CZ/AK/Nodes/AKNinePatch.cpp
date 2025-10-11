#include <CZ/AK/AKLog.h>
#include <CZ/AK/Nodes/AKNinePatch.h>
#include <CZ/Events/CZLayoutEvent.h>
#include <CZ/AK/Events/AKRenderEvent.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Ream/RPass.h>

using namespace CZ;

AKNinePatch::AKNinePatch(std::shared_ptr<RImage> image, SkIRect center, AKNode *parent) noexcept : AKRenderable(RenderableHint::Image, parent)
{
    setImage(image);
    setCenter(center);
}

bool AKNinePatch::setImage(std::shared_ptr<RImage> image) noexcept
{
    if (image == m_image)
        return false;

    m_image = image;
    addChange(CHImage);
    return true;
}

bool AKNinePatch::setCenter(SkIRect center) noexcept
{
    if (center == m_center)
        return false;

    m_center = center;
    addChange(CHCenter);
    return true;
}

bool AKNinePatch::setRegionVisibility(Region reg, bool visible) noexcept
{
    if (regionVisibility(reg) == visible)
        return false;

    m_regions[reg].visible = visible;
    addChange(CHRegionVisibility);
    return true;
}

void AKNinePatch::onSceneBegin()
{
    AKRenderable::onSceneBegin();

    if (changes().testAnyOf(CHImage, CHCenter, CHRegionVisibility, CHLayoutScale, CHLayoutSize))
        updateRegions();
}

void AKNinePatch::renderEvent(const AKRenderEvent &e)
{
    if (!m_image || e.damage.isEmpty())
        return;

    auto *p { e.pass->getPainter() };

    RDrawImageInfo info {};
    info.image = image();
    info.srcScale = scale();

    for (int i = 0; i < Region_Last; i++)
    {
        auto *r { &m_regions[i] };

        if (!r->visible || r->dst.isEmpty())
            continue;

        info.src = SkRect::Make(r->src);
        info.dst = r->dst.makeOffset(e.rect.topLeft());
        p->drawImage(info, &e.damage);
    }
}

void AKNinePatch::updateRegions() noexcept
{
    /* SKIP RENDERING IF THERE IS NO IMAGE OR CENTER IS INVALID */

    if (!image() || m_center.fLeft < 0.f || m_center.fTop < 0.f)
    {
        invisibleRegion.setRect(AK_IRECT_INF);
        return;
    }

    const SkISize imageSize {
        SkScalarCeilToInt((float)image()->size().width() / (float)scale()),
        SkScalarCeilToInt((float)image()->size().height() / (float)scale()) };

    if (m_center.fRight > imageSize.width() || m_center.fBottom > imageSize.height())
    {
        AKLog(CZWarning, CZLN, "Invalid center rect");
        invisibleRegion.setRect(AK_IRECT_INF);
        return;
    }

    /* UPDATE EACH REGION AND SKIP RENDERING INVISIBLE ONES */

    invisibleRegion.setEmpty();

    const auto dstSize { worldRect().size() };

    // TL
    auto *r { &m_regions[TL] };
    r->src.setLTRB(0, 0, m_center.fLeft, m_center.fTop);
    r->dst = r->src;
    if (!r->visible)
        invisibleRegion.op(r->dst, SkRegion::kUnion_Op);

    // TR
    r = &m_regions[TR];
    r->src.setLTRB(m_center.fRight, 0, imageSize.width(), m_center.fTop);
    r->dst.setLTRB(dstSize.width() - r->src.width(), 0,
                   dstSize.width(), r->src.height());
    if (!r->visible)
        invisibleRegion.op(r->dst, SkRegion::kUnion_Op);

    // BR
    r = &m_regions[BR];
    r->src.setLTRB(m_center.fRight, m_center.fBottom,
                   imageSize.width(), imageSize.height());
    r->dst.setLTRB(dstSize.width() - r->src.width(),
                   dstSize.height() - r->src.height(),
                   dstSize.width(), dstSize.height());
    if (!r->visible)
        invisibleRegion.op(r->dst, SkRegion::kUnion_Op);

    // BL
    r = &m_regions[BL];
    r->src.setLTRB(0, m_center.fBottom, m_center.fLeft, imageSize.height());
    r->dst.setLTRB(0, dstSize.height() - r->src.height(),
                   r->src.width(), dstSize.height());
    if (!r->visible)
        invisibleRegion.op(r->dst, SkRegion::kUnion_Op);

    // T
    r = &m_regions[T];
    r->src.setLTRB(m_center.fLeft, 0, m_center.fRight, m_center.fTop);
    r->dst.setLTRB(m_regions[TL].dst.right(), 0,
                   m_regions[TR].dst.left(), r->src.height());
    if (!r->visible)
        invisibleRegion.op(r->dst, SkRegion::kUnion_Op);

    // R
    r = &m_regions[R];
    r->src.setLTRB(m_center.fRight, m_center.fTop, imageSize.width(), m_center.fBottom);
    r->dst.setLTRB(m_regions[TR].dst.left(), m_regions[TR].dst.bottom(),
                   dstSize.width(), m_regions[BR].dst.top());
    if (!r->visible)
        invisibleRegion.op(r->dst, SkRegion::kUnion_Op);

    // B
    r = &m_regions[B];
    r->src.setLTRB(m_center.fLeft, m_center.fBottom, m_center.fRight, imageSize.height());
    r->dst.setLTRB(m_regions[BL].dst.right(), dstSize.height() - r->src.height(),
                   m_regions[BR].dst.left(), dstSize.height());
    if (!r->visible)
        invisibleRegion.op(r->dst, SkRegion::kUnion_Op);

    // L
    r = &m_regions[L];
    r->src.setLTRB(0, m_center.fTop, m_center.fLeft, m_center.fBottom);
    r->dst.setLTRB(0, m_regions[TL].dst.bottom(),
                   r->src.width(), m_regions[BL].dst.top());
    if (!r->visible)
        invisibleRegion.op(r->dst, SkRegion::kUnion_Op);

    // C
    r = &m_regions[C];
    r->src.setLTRB(m_center.fLeft, m_center.fTop, m_center.fRight, m_center.fBottom);
    r->dst.setLTRB(m_regions[L].dst.right(), m_regions[T].dst.bottom(),
                   m_regions[R].dst.left(), m_regions[B].dst.top());

    if (!r->visible)
        invisibleRegion.op(r->dst, SkRegion::kUnion_Op);
}
