#include <include/core/SkCanvas.h>
#include <include/effects/SkImageFilters.h>
#include <include/effects/SkColorMatrixFilter.h>
#include <AK/effects/AKBackgroundImageShadowEffect.h>
#include <AK/AKTarget.h>
#include <AK/AKSurface.h>
#include <AK/AKBrush.h>

using namespace AK;

void AKBackgroundImageShadowEffect::onSceneCalculatedRect()
{
    auto *targetNodeData { targetNode()->targetData(currentTarget()) };

    if (!(targetNode()->caps() & AKNode::Bake) || !targetNodeData || !targetNodeData->bake)
        return;

    bool needsNewSurface { m_targets.find(currentTarget()) == m_targets.end() };
    bool needsFullDamage { needsNewSurface };
    const auto &chg { changes() };

    m_currentData = &m_targets[currentTarget()];

    if (!m_currentData->surface)
    {
        currentTarget()->AKObject::on.destroyed.subscribe(this, [this](AKObject *obj){
            AKTarget *target { static_cast<AKTarget*>(obj) };
            m_targets.erase(target);
        });
    }

    if (m_currentData->prevScale != currentTarget()->bakedComponentsScale())
    {
        needsNewSurface = needsFullDamage = true;
        m_currentData->prevScale = currentTarget()->bakedComponentsScale();
    }

    const SkISize prevSize { effectRect.size() };
    effectRect = SkIRect::MakeWH(targetNode()->rect().width(), targetNode()->rect().height());
    effectRect.outset(m_radius, m_radius);
    effectRect.offset(offset().x(), offset().y());

    if (prevSize != effectRect.size() || chg.test(Chg_Radius) || targetNodeData->onBakeGeneratedDamage)
        needsFullDamage = needsNewSurface = true;

    if (needsNewSurface)
    {
        const SkSize surfaceSize { SkSize::Make(effectRect.size()) };

        if (m_currentData->surface)
            m_currentData->surface->resize(surfaceSize, currentTarget()->bakedComponentsScale(), true);
        else
        {
            m_currentData->surface = AKSurface::Make(
                surfaceSize,
                currentTarget()->bakedComponentsScale(),
                true);
        }

        SkCanvas &canvas { *m_currentData->surface->surface()->getCanvas() };
        AKBrush brush;
        canvas.save();
        canvas.scale(
            currentTarget()->bakedComponentsScale(),
            currentTarget()->bakedComponentsScale());
        canvas.clear(SK_ColorTRANSPARENT);
        brush.setBlendMode(SkBlendMode::kSrc);
        brush.setImageFilter(SkImageFilters::DropShadowOnly(0, 0, m_radius/3.f, m_radius/3.f, SK_ColorBLACK, nullptr));
        canvas.drawImageRect(
            targetNodeData->bake->image(),
            SkRect::MakeXYWH(0, 0,
                             targetNode()->rect().width() * currentTarget()->bakedComponentsScale(),
                             targetNode()->rect().height() * currentTarget()->bakedComponentsScale()),
            SkRect::MakeXYWH(m_radius, m_radius, targetNode()->rect().width(), targetNode()->rect().height()),
            SkFilterMode::kLinear,
            &brush,
            SkCanvas::kFast_SrcRectConstraint);
        canvas.restore();
    }

    if (needsFullDamage)
        addDamage(AK_IRECT_INF);
}


void AKBackgroundImageShadowEffect::onRender(AKPainter *painter, const SkRegion &damage)
{
    auto *targetNodeData { targetNode()->targetData(currentTarget()) };

    if (!(targetNode()->caps() & AKNode::Bake) || !targetNodeData || !targetNodeData->bake)
        return;

    const SkRect src { SkRect::MakeWH(rect().size().width(), rect().size().height()) };

    painter->bindTextureMode({
        .texture = m_currentData->surface->image(),
        .pos = { rect().x(), rect().y() },
        .srcRect = src,
        .dstSize = rect().size(),
        .srcTransform = AKTransform::Normal,
        .srcScale = SkScalar(m_currentData->surface->scale())
    });

    painter->drawRegion(damage);

}

void AKBackgroundImageShadowEffect::onTargetNodeChanged()
{

}
