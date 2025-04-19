#include <include/core/SkCanvas.h>
#include <include/effects/SkImageFilters.h>
#include <include/gpu/ganesh/GrDirectContext.h>
#include <include/core/SkRRect.h>
#include <AK/effects/AKBackgroundBlurEffect.h>
#include <AK/events/AKRenderEvent.h>
#include <AK/AKSceneTarget.h>
#include <AK/AKTheme.h>
#include <AK/AKLog.h>

using namespace AK;

AKBackgroundBlurEffect::AKBackgroundBlurEffect(AKNode *target) noexcept :
    AKBackgroundEffect(Behind)
{
    if (target)
        target->addBackgroundEffect(this);

    bdt.enabled = true;
    bdt.divisibleBy = 1;
    bdt.q = 0.25f;
    bdt.r = 24;
}

void AKBackgroundBlurEffect::onSceneCalculatedRect()
{
    if (!currentTarget()->image())
        return;

    onTargetLayoutUpdated.notify();

    if (areaType() == FullSize)
    {
        effectRect = SkIRect::MakeSize(targetNode()->globalRect().size());
        bdt.reactiveRect = SkIRect::MakeSize(effectRect.size()).makeOutset(5, 5);
        bdt.repaintAnyway.setRect(bdt.reactiveRect);
        bdt.repaintAnyway.op(bdt.reactiveRect.makeInset(5, 5), SkRegion::Op::kDifference_Op);
    }
    else
    {
        effectRect = m_path.getBounds().roundOut();
        bdt.reactiveRect = SkIRect::MakeSize(effectRect.size()).makeOutset(20, 20);
        bdt.repaintAnyway.setRect(bdt.reactiveRect);
        bdt.repaintAnyway.op(bdt.reactiveRect.makeInset(30, 30), SkRegion::Op::kDifference_Op);
    }

    const auto &chgs { changes() };

    if (chgs.test(CHArea))
        addDamage(AK_IRECT_INF);
}

void AKBackgroundBlurEffect::renderEvent(const AKRenderEvent &p)
{
    if (!bdt.currentSurface || p.damage.isEmpty() || p.rect.isEmpty())
        return;

    glDisable(GL_BLEND);
    bool reblur { !bdt.damage.isEmpty() };
    bool copyAll { false };
    SkScalar scale { bdt.currentSurface->scale() * 0.5f};
    SkISize copySize = p.rect.size();
    const int m { SkScalarRoundToInt(1.f / (scale * 0.5f)) };
    const int modW { copySize.fWidth % m };
    const int modH { copySize.fHeight % m };

    if (modW != 0)
        copySize.fWidth -= modW;
    if (modH != 0)
        copySize.fHeight -= modH;

    if (m_blur)
    {
        copyAll |= m_blur->resize(copySize, scale, true);
        reblur |= copyAll;
    }
    else
    {
        copyAll = reblur = true;
        m_blur = AKSurface::Make(copySize, scale, false);
    }

    if (m_blur2)
        reblur |= m_blur2->resize(copySize, scale * 0.5, true);
    else
        m_blur2 = AKSurface::Make(copySize, scale * 0.5, false);

    if (reblur)
    {
        p.painter.bindTarget(m_blur.get());
        glDisable(GL_BLEND);
        p.painter.setAlpha(1.f);
        p.painter.setColorFactor(1.f, 1.f, 1.f, 1.f);
        p.painter.bindTextureMode({
            .texture = bdt.currentSurface->image(),
            .pos = {0, 0},
            .srcRect = SkRect::MakeXYWH(
                p.rect.x() - p.target.viewport().x(),
                p.rect.y() - p.target.viewport().y(),
                p.rect.width(),
                p.rect.height()),
            .dstSize =  m_blur->size(),
            .srcTransform = AKTransform::Normal,
            .srcScale = bdt.currentSurface->scale()
        });

        p.painter.blendMode = AKPainter::Vibrancy1;

        if (copyAll)
            p.painter.drawRect(SkIRect::MakeSize(p.rect.size()));
        else
        {
            bdt.damage.translate(-p.rect.x(), -p.rect.y());
            bdt.damage.op(SkIRect::MakeSize(m_blur->size()), SkRegion::Op::kIntersect_Op);
            p.painter.drawRegion(bdt.damage);
        }

        const SkIRect rect { SkIRect::MakeWH(m_blur2->size().width(), m_blur2->size().height()) };

        p.painter.bindTarget(m_blur2.get());
        glDisable(GL_BLEND);
        p.painter.bindTextureMode({
            .texture = m_blur->image(),
            .pos = {0, 0},
            .srcRect = SkRect::Make(m_blur->imageSrcRect()),
            .dstSize =  rect.size(),
            .srcTransform = AKTransform::Normal,
            .srcScale = 1.f
        });

        p.painter.blendMode = AKPainter::Vibrancy2;

        if (copyAll)
            p.painter.drawRect(rect);
        else
            p.painter.drawRegion(bdt.damage);

        p.painter.blendMode = AKPainter::Normal;
    }

    if (areaType() == Path)
    {
        constexpr int mod { 2 };
        SkIRect pathLocalRect;
        pathLocalRect = m_path.getBounds().round();
        pathLocalRect.setXYWH(
            pathLocalRect.x(),
            pathLocalRect.y(),
            pathLocalRect.width() + mod - (pathLocalRect.width() % mod),
            pathLocalRect.height() + mod - (pathLocalRect.height() % mod));

        p.target.surface()->recordingContext()->asDirectContext()->resetContext();
        auto &c { *p.target.surface()->getCanvas() };
        c.save();

        SkRegion damage = p.damage;
        damage.op(p.rect, SkRegion::Op::kIntersect_Op);

        SkPath damagePath;
        damage.getBoundaryPath(&damagePath);
        damagePath.setIsVolatile(true);
        c.resetMatrix();
        c.scale(p.target.xyScale().x(), p.target.xyScale().y());
        c.translate(
            p.rect.x() - p.target.viewport().x() - pathLocalRect.x(),
            p.rect.y() - p.target.viewport().y() -pathLocalRect.y());
        c.clipPath(m_path);
        c.translate(
            pathLocalRect.x() - p.rect.x(),
            pathLocalRect.y() -p.rect.y());
        c.clipPath(damagePath);

        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        paint.setAntiAlias(false);
        c.drawImageRect(m_blur2->image(),
            SkRect::Make(m_blur2->imageSrcRect()),
            SkRect::Make(p.rect),
            SkFilterMode::kLinear,
            &paint,
            SkCanvas::kFast_SrcRectConstraint);

        p.target.surface()->recordingContext()->asDirectContext()->flush();
        c.restore();
        p.painter.bindProgram();
        p.painter.bindTarget(&p.target);
    }
    else if (areaType() == Region)
    {
        SkRegion damage;
        damage.op(m_region, p.damage, SkRegion::Op::kIntersect_Op);
        p.painter.bindProgram();
        p.painter.bindTarget(&p.target);
        p.painter.setParamsFromRenderable(this);
        glDisable(GL_BLEND);
        p.painter.bindTextureMode({
            .texture = m_blur2->image(),
            .pos = p.rect.topLeft(),
            .srcRect = SkRect::Make(m_blur2->imageSrcRect()),
            .dstSize = p.rect.size(),
            .srcTransform = AKTransform::Normal,
            .srcScale = 1.f,
        });
        p.painter.drawRegion(p.damage);
    }
    else if (areaType() == RoundRect)
    {
        SkRegion damage;
        damage.op(m_region, p.damage, SkRegion::Op::kIntersect_Op);

        if (m_rRect.fRadTL > 0)
        {
            SkRegion cornerDamage;
            const SkIRect cornerRect { SkIRect::MakeXYWH(p.rect.x(), p.rect.y(), m_rRect.fRadTL, m_rRect.fRadTL) };
            cornerDamage.op(cornerRect, damage, SkRegion::kIntersect_Op);

            if (cornerDamage.isEmpty())
                goto skipTL;

            if (m_roundCorners[0])
                m_roundCorners[0]->resize(cornerRect.size(), this->scale(), false);
            else
                m_roundCorners[0] = AKSurface::Make(cornerRect.size(), this->scale(), false);

            m_roundCorners[0]->setViewportPos(p.rect.x(), p.rect.y());
        }

        skipTL:

        p.painter.bindTarget(&p.target);
        p.painter.setParamsFromRenderable(this);
        glDisable(GL_BLEND);
        p.painter.bindTextureMode({
            .texture = m_blur2->image(),
            .pos = p.rect.topLeft(),
            .srcRect = SkRect::Make(m_blur2->imageSrcRect()),
            .dstSize = p.rect.size(),
            .srcTransform = AKTransform::Normal,
            .srcScale = 1.f,
        });
        p.painter.drawRegion(p.damage);
    }

    // Using the full node size
    else
    {
        p.painter.bindProgram();
        p.painter.bindTarget(&p.target);
        p.painter.setParamsFromRenderable(this);
        glDisable(GL_BLEND);
        p.painter.bindTextureMode({
            .texture = m_blur2->image(),
            .pos = p.rect.topLeft(),
            .srcRect = SkRect::Make(m_blur2->imageSrcRect()),
            .dstSize = p.rect.size(),
            .srcTransform = AKTransform::Normal,
            .srcScale = 1.f,
        });
        p.painter.drawRegion(p.damage);
    }
}
