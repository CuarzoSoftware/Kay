#include <include/core/SkCanvas.h>
#include <include/gpu/ganesh/SkImageGanesh.h>
#include <AK/nodes/AKImage.h>
#include <AK/AKPen.h>
#include <AK/AKTarget.h>
#include <AK/AKPainter.h>

using namespace AK;

void AKImage::onSceneBegin()
{
    if (!m_autoDamage)
        return;

    const auto &chg { changes() };

    if (chg.test(Chg_Image) ||
        chg.test(Chg_Transform) ||
        chg.test(Chg_SizeMode) ||
        chg.test(Chg_SrcRectMode) ||
        (srcRectMode() == SrcRectMode::Custom && (chg.test(Chg_CustomSrcRect) || chg.test(Chg_CustomSrcRectScale))) ||
        (sizeMode() != SizeMode::Fill && chg.test(Chg_Alignment)))
        addDamage(AK_IRECT_INF);
}

void AKImage::onRender(AKPainter *painter, const SkRegion &damage)
{
    if (!image() || image()->width() == 0 || image()->height() == 0)
        return;

    AKPainter::TextureParams params;

    if (srcRectMode() == SrcRectMode::Custom)
    {
        if (customSrcRect().isEmpty())
            return;

        params.srcRect = customSrcRect();
        params.srcScale = customSrcRectScale();
    }
    else
    {
        params.srcScale = 1.f;

        if (AK::is90Transform(transform()))
            params.srcRect.setWH(image()->height(), image()->width());
        else
            params.srcRect.setWH(image()->width(), image()->height());
    }

    SkRegion finalDamage { damage };

    if (sizeMode() == SizeMode::Fill)
    {
        params.pos = rect().topLeft();
        params.dstSize = rect().size();
    }
    else if (sizeMode() == SizeMode::Contain)
    {
        params.dstSize.fWidth = (params.srcRect.width() * rect().height()) / params.srcRect.height();

        if (params.dstSize.fWidth > rect().width())
        {
            params.dstSize.fWidth = rect().width();
            params.dstSize.fHeight = (params.srcRect.height() * rect().width()) / params.srcRect.width();
        }
        else
            params.dstSize.fHeight = rect().height();
    }
    else // Cover
    {
        params.dstSize.fWidth = (params.srcRect.width() * rect().height()) / params.srcRect.height();

        if (params.dstSize.fWidth < rect().width())
        {
            params.dstSize.fWidth = rect().width();
            params.dstSize.fHeight = (params.srcRect.height() * rect().width()) / params.srcRect.width();
        }
        else
            params.dstSize.fHeight = rect().height();
    }

    // Alignment
    if (sizeMode() != SizeMode::Fill)
    {
        const AKBitset<AKAlignment> alignment { m_alignment };

        // X-axis
        if (alignment.checkAll(AKAlignLeft | AKAlignRight) || !alignment.check(AKAlignLeft | AKAlignRight))
            params.pos.fX = rect().x() + (rect().width() - params.dstSize.width()) / 2;
        else if (alignment.check(AKAlignLeft))
            params.pos.fX = rect().x();
        else if (alignment.check(AKAlignRight))
            params.pos.fX = rect().fRight - params.dstSize.fWidth;

        // Y-axis
        if (alignment.checkAll(AKAlignTop | AKAlignBottom) || !alignment.check(AKAlignTop | AKAlignBottom))
            params.pos.fY = rect().y() + (rect().height() - params.dstSize.height()) / 2;
        else if (alignment.check(AKAlignTop))
            params.pos.fY = rect().y();
        else if (alignment.check(AKAlignBottom))
            params.pos.fY = rect().fBottom - params.dstSize.fHeight;

        if (sizeMode() == SizeMode::Contain)
            finalDamage.op(
                SkIRect::MakeXYWH(
                    params.pos.x(), params.pos.y(),
                    params.dstSize.width(), params.dstSize.height()),
                SkRegion::Op::kIntersect_Op);
    }

    params.texture = image();
    params.srcTransform = transform();
    painter->bindTextureMode(params);
    painter->drawRegion(finalDamage);
}
