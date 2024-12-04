#include <AK/effects/AKRoundCornersEffect.h>
#include <include/core/SkCanvas.h>

using namespace AK;

static constexpr Int32 outset { 2 };
static constexpr Int32 offset { 2 };

bool AKRoundCornersEffect::addDamage(const SkSize &nodeSize, const SkRegion *clip, SkRegion *damage) noexcept
{
    const bool nodeWidthChanged { Int32(nodeSize.width()) != m_rect[BR].fRight };
    const bool nodeHeightChanged { Int32(nodeSize.height()) != m_rect[BR].fBottom };
    bool damageAdded { nodeWidthChanged || nodeHeightChanged };
    SkIRect prevRects[4];
    memcpy(prevRects, m_rect, sizeof(m_rect));

    m_rect[TL] = SkIRect::MakeWH(
        m_corners.fTL,
        m_corners.fTL);
    m_rect[TR] = SkIRect(
        nodeSize.width() - m_corners.fTR, 0,
        nodeSize.width(), m_corners.fTR);
    m_rect[BL] = SkIRect(
        0, nodeSize.height() - m_corners.fBL,
        m_corners.fBL, nodeSize.height());
    m_rect[BR] = SkIRect(
        nodeSize.width() - m_corners.fBR, nodeSize.height() - m_corners.fBR,
        nodeSize.width(), nodeSize.height());

    if (clip->intersects(m_rect[BR]) || m_rect[BR].isEmpty())
    {
        if (Int32(m_mask[BR].getBounds().width()) - offset != m_corners.fBR)
        {
            damageAdded = true;
            SkMatrix m = SkMatrix::Scale(-1.f, -1.f);
            m.postTranslate(m_corners.fBR, m_corners.fBR);
            m_mask[BR].reset();
            m_mask[BR].moveTo(-offset, -offset);
            m_mask[BR].lineTo(-offset, m_corners.fBR);
            m_mask[BR].lineTo(0.f, m_corners.fBR);
            m_mask[BR].arcTo(0.f, 0.f,
                             m_corners.fBR, 0,
                             m_corners.fBR);
            m_mask[BR].lineTo(m_corners.fBR, -offset);
            m_mask[BR].close();
            m_mask[BR].transform(m);
            prevRects[BR].join(m_rect[BR]);
            prevRects[BR].fLeft -= outset;
            prevRects[BR].fTop -= outset;
            damage->op(prevRects[BR], SkRegion::Op::kUnion_Op);
        }
        else if (damageAdded)
            damage->op(prevRects[BR], SkRegion::Op::kUnion_Op);
    }

    if (clip->intersects(m_rect[TL]) || m_rect[TL].isEmpty())
    {
        if (Int32(m_mask[TL].getBounds().width()) - offset!= m_corners.fTL)
        {
            damageAdded = true;
            m_mask[TL].reset();
            m_mask[TL].moveTo(-offset, -offset);
            m_mask[TL].lineTo(-offset, m_corners.fTL);
            m_mask[TL].lineTo(0.f, m_corners.fTL);
            m_mask[TL].arcTo(0.f, 0.f,
                            m_corners.fTL, 0,
                            m_corners.fTL);
            m_mask[TL].lineTo(m_corners.fTL, -offset);
            m_mask[TL].close();

            prevRects[TL].join(m_rect[TL]);
            prevRects[TL].fRight += outset;
            prevRects[TL].fBottom += outset;
            damage->op(prevRects[TL], SkRegion::Op::kUnion_Op);
        }
    }

    if (clip->intersects(m_rect[TR]) || m_rect[TR].isEmpty())
    {
        if (Int32(m_mask[TR].getBounds().width()) - offset != m_corners.fTR)
        {
            damageAdded = true;
            SkMatrix m = SkMatrix::Scale(-1.f, 1.f);
            m.postTranslate(m_corners.fTR, 0.f);
            m_mask[TR].reset();
            m_mask[TR].moveTo(-offset, -offset);
            m_mask[TR].lineTo(-offset, m_corners.fTR);
            m_mask[TR].lineTo(0.f, m_corners.fTR);
            m_mask[TR].arcTo(0.f, 0.f,
                             m_corners.fTR, 0,
                             m_corners.fTR);
            m_mask[TR].lineTo(m_corners.fTR, -offset);
            m_mask[TR].close();
            m_mask[TR].transform(m);
            prevRects[TR].join(m_rect[TR]);
            prevRects[TR].fLeft -= outset;
            prevRects[TR].fBottom += outset;
            damage->op(prevRects[TR], SkRegion::Op::kUnion_Op);
        }
        else if (nodeWidthChanged)
            damage->op(prevRects[TR], SkRegion::Op::kUnion_Op);
    }

    if (clip->intersects(m_rect[BL]) || m_rect[BL].isEmpty())
    {
        if (Int32(m_mask[BL].getBounds().width()) - offset != m_corners.fBL)
        {
            damageAdded = true;
            SkMatrix m = SkMatrix::Scale(1.f, -1.f);
            m.postTranslate(0.f, m_corners.fBL);
            m_mask[BL].reset();
            m_mask[BL].moveTo(-offset, -offset);
            m_mask[BL].lineTo(-offset, m_corners.fBL);
            m_mask[BL].lineTo(0.f, m_corners.fBL);
            m_mask[BL].arcTo(0.f, 0.f,
                             m_corners.fBL, 0,
                             m_corners.fBL);
            m_mask[BL].lineTo(m_corners.fBL, -offset);
            m_mask[BL].close();
            m_mask[BL].transform(m);
            prevRects[BL].join(m_rect[BL]);
            prevRects[BL].fRight += outset;
            prevRects[BL].fTop -= outset;
            damage->op(prevRects[BL], SkRegion::Op::kUnion_Op);
        }
        else if (nodeHeightChanged)
            damage->op(prevRects[BL], SkRegion::Op::kUnion_Op);
    }

    return damageAdded;
}

bool AKRoundCornersEffect::clipCorners(SkCanvas *canvas, const SkRegion *clip, SkRegion *damage, SkRegion *opaque) noexcept
{
    bool cornerClipped { false };

    canvas->save();
    canvas->clipIRect(clip->getBounds());

    for (int i = 0; i < CornerLast; i++)
    {
        if (!clip->quickReject(m_rect[i]) && damage->intersects(m_rect[i]))
        {
            canvas->save();
            canvas->translate(
                SkScalar(m_rect[i].x()),
                SkScalar(m_rect[i].y()));
            canvas->drawPath(m_mask[i], m_brush);
            cornerClipped = true;
            canvas->restore();
        }

        opaque->op(m_rect[i], SkRegion::Op::kDifference_Op);
    }

    canvas->restore();
    return cornerClipped;
}
