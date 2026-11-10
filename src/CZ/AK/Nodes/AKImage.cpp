#include <CZ/AK/Nodes/AKImage.h>
#include <CZ/AK/Events/AKRenderEvent.h>
#include <CZ/Ream/RPass.h>
#include <CZ/Ream/RImage.h>

using namespace CZ;

void AKImage::onSceneBegin()
{
    if (!m_autoDamage)
        return;

    const auto &chg { changes() };

    if (chg.testAnyOf(CHSrcTransform, CHSrcRectMode) ||
        (srcRectMode() == SrcRectMode::Custom && (chg.testAnyOf(CHCustomSrcRect, CHCustomSrcRectScale))))
        addDamage(AK_IRECT_INF);
}

void AKImage::renderEvent(const AKRenderEvent &e)
{
    if (e.damage.isEmpty() || !image())
        return;

    auto *p { e.pass->getPainter() };

    RDrawImageInfo info {};
    info.image = image();
    info.dst = e.rect;
    info.srcTransform = srcTransform();

    if (srcRectMode() == SrcRectMode::Custom)
    {
        info.src = customSrcRect();
        info.srcScale = customSrcRectScale();
    }
    else
    {
        info.srcScale = 1.f;

        if (CZ::Is90Transform(srcTransform()))
            info.src.setWH(image()->size().height(), image()->size().width());
        else
            info.src.setWH(image()->size().width(), image()->size().height());
    }

    if (m_mask)
    {
        RDrawImageInfo mask {};
        mask.image = m_mask;
        mask.dst = e.rect;
        mask.src = SkRect::MakeWH(m_mask->size().width(), m_mask->size().height());
        p->drawImage(info, &e.damage, &mask);
        return;
    }

    p->drawImage(info, &e.damage);
}
