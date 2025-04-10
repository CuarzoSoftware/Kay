#include <include/core/SkCanvas.h>
#include <include/effects/SkImageFilters.h>
#include <include/gpu/ganesh/GrDirectContext.h>
#include <AK/effects/AKBackgroundBlurEffect.h>
#include <AK/events/AKRenderEvent.h>
#include <AK/AKSceneTarget.h>
#include <AK/AKLog.h>

using namespace AK;

AKBackgroundBlurEffect::AKBackgroundBlurEffect(ClipMode clipMode, const SkVector &sigma, AKNode *target) noexcept :
    AKBackgroundEffect(Behind),
    m_sigma(sigma),
    m_clipMode(clipMode)
{
    if (target)
        target->addBackgroundEffect(this);
}

void AKBackgroundBlurEffect::onSceneCalculatedRect()
{
    if (!currentTarget()->image())
        return;

    if (clipMode() == Automatic)
    {
        effectRect = SkIRect::MakeSize(targetNode()->globalRect().size());
        bdt.reactiveRegion.setRect(SkIRect::MakeSize(effectRect.size()));
    }
    else
    {
        on.targetLayoutUpdated.notify();
        bdt.reactiveRegion.setRect(clip.getBounds().roundOut());
    }

    const auto &chgs { changes() };

    if (chgs.testAnyOf(CHSigma, CHClipMode))
        addDamage(AK_IRECT_INF);

    if (!m_brush.getImageFilter() || chgs.test(CHSigma))
        m_brush.setImageFilter(SkImageFilters::Blur(m_sigma.x(), m_sigma.y(), SkTileMode::kMirror, nullptr));
}

void AKBackgroundBlurEffect::renderEvent(const AKRenderEvent &p)
{
    if (!p.target.image() || p.damage.isEmpty())
        return;

    glDisable(GL_BLEND);

    SkIRect pathLocalRect;

    if (clipMode() == Manual)
        pathLocalRect = clip.getBounds().roundOut();
    else
        pathLocalRect = SkIRect::MakeSize(p.rect.size());

    SkRect srcRect { SkRect::Make(pathLocalRect) };
    srcRect.offset(p.rect.x(), p.rect.y());

    SkScalar d1 { 0.75f };
    SkScalar d2 { 0.5f };
    SkScalar d3 { 0.25f };
    SkScalar d4 { 0.125f };

    SkScalar q { SkScalar(p.target.xyScale().x())};

    bool reblur { !bdt.damage.isEmpty() };
    bool copyAll { false };

    auto &bd { m_blurData.emplace(&p.target, BlurData()).first->second };

    if (bd.backgroundCopy)
    {
        copyAll = bd.backgroundCopy->resize(pathLocalRect.size(), q * d1, false);
        reblur |= copyAll;
    }
    else
    {
        copyAll = true;
        bd.backgroundCopy = AKSurface::Make(pathLocalRect.size(), q * d1, false);
    }

    if (bd.backgroundCopy2)
    {
        bd.backgroundCopy2->resize(pathLocalRect.size(), q * d2, false);
    }
    else
    {
        bd.backgroundCopy2 = AKSurface::Make(pathLocalRect.size(), q * d2, false);
    }

    if (bd.backgroundCopy3)
    {
        bd.backgroundCopy3->resize(pathLocalRect.size(), q * d3, false);
    }
    else
    {
        bd.backgroundCopy3 = AKSurface::Make(pathLocalRect.size(), q * d3, false);
    }

    if (bd.blur)
        reblur |= bd.blur->resize(pathLocalRect.size(), q * d4, false);
    else
    {
        reblur = true;
        bd.blur = AKSurface::Make(pathLocalRect.size(), q * d4, true);
    }

    if (reblur)
    {
        // Copy lowq background
        const SkIRect backgroundCopySrc { SkIRect::MakeSize(bd.backgroundCopy->size()) };
        p.painter.bindTarget(bd.backgroundCopy.get());
        const SkScalar up { 1.f };
        p.painter.setColorFactor(0.8, 0.8, up, up);
        p.painter.bindTextureMode({
            .texture = p.target.image(),
            .pos = {0, 0},
            .srcRect = SkRect::MakeXYWH(srcRect.x() - p.target.viewport().x(), srcRect.y() - p.target.viewport().y(), srcRect.width(), srcRect.height()),
            .dstSize =  SkISize( bd.backgroundCopy->size().width(), bd.backgroundCopy->size().height()),
            .srcTransform = AKTransform::Normal,
            .srcScale = p.target.xyScale().x()
        });

        if (copyAll)
            p.painter.drawRect(backgroundCopySrc);
        else
        {
            bdt.damage.translate(-srcRect.x(), -srcRect.y());
            bdt.damage.op(backgroundCopySrc, SkRegion::Op::kIntersect_Op);
            p.painter.drawRegion(bdt.damage);
        }

        p.painter.bindColorMode();
        p.painter.setColorFactor(1.f, 1.f, 1.f, 1.f);
        const SkScalar add { 1.5f };
        p.painter.setColor({add, add, add, add});
        p.painter.setAlpha(0.3f);
        glEnable(GL_BLEND);

        if (copyAll)
            p.painter.drawRect(backgroundCopySrc);
        else
            p.painter.drawRegion(bdt.damage);

        // 2

        p.painter.bindTarget(bd.backgroundCopy2.get());
        p.painter.setAlpha(1.f);
        glDisable(GL_BLEND);
        p.painter.bindTextureMode({
            .texture = bd.backgroundCopy->image(),
            .pos = {0, 0},
            .srcRect = SkRect::Make(bd.backgroundCopy->imageSrcRect()),
            .dstSize =  SkISize( bd.backgroundCopy2->size().width(), bd.backgroundCopy2->size().height()),
            .srcTransform = AKTransform::Normal,
            .srcScale = 1.f
        });

        if (copyAll)
            p.painter.drawRect(backgroundCopySrc);
        else
            p.painter.drawRegion(bdt.damage);


        // Blur 1

        // Recreate blur
        bd.blur->surface()->recordingContext()->asDirectContext()->resetContext();
        SkCanvas &c3 { *bd.backgroundCopy3->surface()->getCanvas() };
        c3.save();
        m_brush.setAntiAlias(false);
        m_brush.setBlendMode(SkBlendMode::kSrc);
        c3.drawImageRect(bd.backgroundCopy2->image(),
                        SkRect::Make(bd.backgroundCopy2->imageSrcRect()),
                        SkRect::Make(bd.backgroundCopy3->imageSrcRect()),
                        SkFilterMode::kLinear,
                        &m_brush,
                        SkCanvas::kFast_SrcRectConstraint);
        bd.backgroundCopy3->surface()->recordingContext()->asDirectContext()->flush();
        c3.restore();

        // Recreate blur
        SkCanvas &c { *bd.blur->surface()->getCanvas() };
        c.save();
        m_brush.setAntiAlias(false);
        m_brush.setBlendMode(SkBlendMode::kSrc);
        c.drawImageRect(bd.backgroundCopy3->image(),
                        SkRect::Make(bd.backgroundCopy3->imageSrcRect()),
                        SkRect::Make(bd.blur->imageSrcRect()),
                        SkFilterMode::kLinear,
                        &m_brush,
                        SkCanvas::kFast_SrcRectConstraint);
        bd.blur->surface()->recordingContext()->asDirectContext()->flush();
        c.restore();
    }

    p.target.surface()->recordingContext()->asDirectContext()->resetContext();
    auto &c { *p.target.surface()->getCanvas() };
    c.save();

    SkRegion dam = p.damage;
    dam.op(srcRect.roundOut(), SkRegion::Op::kIntersect_Op);

    SkPath damagePath;
    dam.getBoundaryPath(&damagePath);
    c.resetMatrix();
    c.scale(p.target.xyScale().x(), p.target.xyScale().y());
    c.translate(-p.target.viewport().x(), - p.target.viewport().y());


    if (clipMode() == Manual)
    {
        c.translate(p.rect.x(), p.rect.y());
        c.clipPath(clip);
        c.translate(-p.rect.x(), -p.rect.y());
    }

    c.clipPath(damagePath);

    SkPaint paint;
    paint.setAntiAlias(true);
    c.drawImageRect(bd.blur->image(),
                    SkRect::Make(bd.blur->imageSrcRect()),
                    srcRect,
                    SkFilterMode::kLinear,
                    &paint,
                    SkCanvas::kFast_SrcRectConstraint);

    p.target.surface()->recordingContext()->asDirectContext()->flush();
    c.restore();

    p.painter.bindProgram();
    p.painter.bindTarget(&p.target);

    /*
    p.painter.bindProgram();
    p.painter.bindTarget(&p.target);
    p.painter.setParamsFromRenderable(this);
    glEnable(GL_BLEND);
    p.painter.bindTextureMode({
        .texture = bd.blur->image(),
        .pos = p.rect.topLeft() + pathLocalRect.topLeft(),
        .srcRect = SkRect::Make(bd.blur->imageSrcRect()),
        .dstSize = bd.blur->imageSrcRect().size(),
        .srcTransform = AKTransform::Normal,
        .srcScale = bd.blur->scale()
    });
    SkRegion dam = p.damage;
    dam.op(srcRect.roundOut(), SkRegion::Op::kIntersect_Op);
    p.painter.drawRegion(dam);
    */
}
