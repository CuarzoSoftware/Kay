#include <AK/nodes/AKRenderableRect.h>

#include <include/core/SkCanvas.h>
#include <include/core/SkRegion.h>
#include <include/effects/SkGradientShader.h>

using namespace AK;

// TODO: Fix damage and opaque region

void AKRenderableRect::onSceneBegin()
{
    if (m_pendingBrush != m_currentBrush)
    {
        m_currentBrush = m_pendingBrush;
        //addDamage(AK_IRECT_INF);
    }

    m_currentPen = m_pendingPen;
    //setOpaqueRegion(SkColorGetA(brush().getAlpha()) == 255 ? AK_IRECT_INF : SkIRect::MakeEmpty());
    AKRenderable::onSceneBegin();
}

void AKRenderableRect::onRender(SkCanvas *canvas, const SkRegion &damage, bool opaque)
{
    const SkIRect rect { SkIRect::MakeWH(this->rect().width(), this->rect().height()) };

    if (!m_currentBrush.nothingToDraw())
    {
        if (m_currentBrush.autoBlendMode)
            m_currentBrush.setBlendMode(opaque ? SkBlendMode::kSrc : SkBlendMode::kSrcOver);

        SkRegion::Iterator it(damage);
        while (!it.done())
        {
            canvas->save();
            canvas->clipIRect(it.rect());
            canvas->drawIRect(rect, m_currentBrush);
            canvas->restore();
            it.next();
        }
    }

    if (!m_currentPen.nothingToDraw())
    {
        m_currentPen.setBlendMode(opaque ? SkBlendMode::kSrc : SkBlendMode::kSrcOver);
        SkRegion::Iterator it(damage);
        while (!it.done())
        {
            canvas->save();
            canvas->clipIRect(it.rect());
            canvas->drawIRect(rect, m_currentPen);
            canvas->restore();
            it.next();
        }
    }

    m_currentBrush = m_pendingBrush;
    m_currentPen = m_pendingPen;
}
