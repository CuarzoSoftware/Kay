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

AKBackgroundBlurEffect::AKBackgroundBlurEffect(ClipMode clipMode, const SkVector &sigma, AKNode *target) noexcept :
    AKBackgroundEffect(Behind),
    m_sigma(sigma),
    m_clipMode(clipMode)
{
    if (target)
        target->addBackgroundEffect(this);

    m_brush1.setAntiAlias(false);
    m_brush1.setBlendMode(SkBlendMode::kSrc);
    m_brush2.setAntiAlias(false);
    m_brush2.setBlendMode(SkBlendMode::kSrc);
    m_brush3.setAntiAlias(false);
    m_brush3.setBlendMode(SkBlendMode::kPlus);
    bdt.enabled = true;
    bdt.divisibleBy = 16;
    bdt.q = 0.25f;
    bdt.r = 100;
}

void AKBackgroundBlurEffect::onSceneCalculatedRect()
{
    if (!currentTarget()->image())
        return;

    if (clipMode() == Automatic)
    {
        effectRect = SkIRect::MakeSize(targetNode()->globalRect().size());
        bdt.reactiveRect = SkIRect::MakeSize(effectRect.size()).makeOutset(5, 5);
        bdt.repaintAnyway.setRect(bdt.reactiveRect);
        bdt.repaintAnyway.op(bdt.reactiveRect.makeInset(5, 5), SkRegion::Op::kDifference_Op);
    }
    else
    {
        on.targetLayoutUpdated.notify();
        effectRect = clip.getBounds().roundOut();
        bdt.reactiveRect = SkIRect::MakeSize(effectRect.size()).makeOutset(20, 20);
        bdt.repaintAnyway.setRect(bdt.reactiveRect);
        bdt.repaintAnyway.op(bdt.reactiveRect.makeInset(30, 30), SkRegion::Op::kDifference_Op);
    }

    const auto &chgs { changes() };

    if (chgs.testAnyOf(CHSigma, CHClipMode))
        addDamage(AK_IRECT_INF);

    if (!m_brush1.getImageFilter() || chgs.test(CHSigma))
    {
        const SkScalar sigma = 160.f;
        m_brush1.setImageFilter(SkImageFilters::Blur(sigma, sigma, SkTileMode::kMirror, nullptr));
        m_brush1.setImageFilter(SkImageFilters::Blur(AKTheme::BlurVibrancySigma * 0., AKTheme::BlurVibrancySigma * 0., SkTileMode::kMirror, nullptr));
    }
}

void AKBackgroundBlurEffect::renderEvent(const AKRenderEvent &p)
{
    if (!bdt.currentSurface || p.damage.isEmpty() || p.rect.isEmpty())
        return;

    glDisable(GL_BLEND);

    SkIRect pathLocalRect;

    int mod { 16 };
    if (clipMode() == Manual)
    {
        pathLocalRect = clip.getBounds().round();
        pathLocalRect.setXYWH(
            pathLocalRect.x(),
            pathLocalRect.y(),
            pathLocalRect.width() + mod - (pathLocalRect.width() % mod),
            pathLocalRect.height() + mod - (pathLocalRect.height() % mod));
    }
    else
        pathLocalRect = SkIRect::MakeSize(p.rect.size());

    //const SkRect srcRect { SkRect::Make(pathLocalRect).makeOffset(p.rect.x(), p.rect.y()) };
    const SkRect srcRect { SkRect::Make(p.rect) };

    //constexpr SkScalar d4 { 0.5f * 0.5f };
    const bool shrink { true };
    //const SkScalar q { SkScalar(targetNode()->scale()) };

    bool reblur { !bdt.damage.isEmpty() };
    bool copyAll { false };

    SkScalar scale { bdt.currentSurface->scale() * 0.5f};

    SkISize copySize = p.rect.size();

    int m { SkScalarRoundToInt(1.f / (scale * 0.5f)) };
    const int modW { copySize.fWidth % m };
    const int modH { copySize.fHeight % m };

    if (modW != 0)
        copySize.fWidth +=  m - modW;
    if (modH != 0)
        copySize.fHeight += m - modH;

    if (m_blur)
    {
        reblur |= m_blur->resize(copySize, scale, shrink);
        copyAll |= reblur;
    }
    else
    {
        copyAll = true;
        reblur = true;
        m_blur = AKSurface::Make(copySize, scale, false);
    }

    if (reblur)
    {
        const SkIRect backgroundCopySrc { SkIRect::MakeSize(m_blur->size()) };

        p.painter.bindTarget(m_blur.get());
        glDisable(GL_BLEND);
        p.painter.setAlpha(1.f);
        p.painter.setColorFactor(1.f, 1.f, 1.f, 1.f);
        p.painter.bindTextureMode({
            .texture = bdt.currentSurface->image(),
            .pos = {0, 0},
            .srcRect = SkRect::Make(p.rect),
            .dstSize =  m_blur->size(),
            .srcTransform = AKTransform::Normal,
            .srcScale = bdt.currentSurface->scale()
        });

        p.painter.blendMode = AKPainter::Vibrancy1;
        if (copyAll || true)
            p.painter.drawRect(SkIRect::MakeSize(p.rect.size()));
        else
        {
            bdt.damage.translate(-srcRect.x(), -srcRect.y());
            bdt.damage.op(backgroundCopySrc, SkRegion::Op::kIntersect_Op);
            p.painter.drawRegion(bdt.damage);
        }

        const auto size { SkISize(
            (m_blur->size().width() + 12)/2,
            (m_blur->size().height() + 12)/2) };

        p.painter.bindTextureMode({
            .texture = m_blur->image(),
            .pos = {0, 0},
            .srcRect = SkRect::Make(m_blur->imageSrcRect()),
            .dstSize =  size,
            .srcTransform = AKTransform::Normal,
            .srcScale = 1.f
        });

        p.painter.blendMode = AKPainter::Vibrancy2;
        p.painter.drawRect(SkIRect::MakeSize(size));
        p.painter.blendMode = AKPainter::Normal;
    }


    // This means using an SkPath
    if (clipMode() == Manual && false)
    {
        p.target.surface()->recordingContext()->asDirectContext()->resetContext();
        auto &c { *p.target.surface()->getCanvas() };
        c.save();

        SkRegion damage = p.damage;
        damage.op(srcRect.roundOut(), SkRegion::Op::kIntersect_Op);

        SkPath damagePath;
        damage.getBoundaryPath(&damagePath);
        damagePath.setIsVolatile(true);
        c.resetMatrix();
        c.scale(p.target.xyScale().x(), p.target.xyScale().y());
        c.translate(-p.target.viewport().x(), - p.target.viewport().y());
        c.translate(-pathLocalRect.x(), -pathLocalRect.y());
        c.translate(p.rect.x(), p.rect.y());
        c.clipPath(clip);
        c.translate(pathLocalRect.x(), pathLocalRect.y());
        c.translate(-p.rect.x(), -p.rect.y());

        c.clipPath(damagePath);

        SkPaint paint;
        paint.setAntiAlias(false);
        c.drawImageRect(m_blur->image(),
            SkRect::MakeWH(m_blur->imageSrcRect().width()/2, m_blur->imageSrcRect().height()/2),
            SkRect::Make(p.rect),
            SkFilterMode::kLinear,
            &paint,
            SkCanvas::kFast_SrcRectConstraint);

        p.target.surface()->recordingContext()->asDirectContext()->flush();
        c.restore();
        p.painter.bindProgram();
        p.painter.bindTarget(&p.target);
    }

    // Using the full node size
    else
    {
        p.painter.bindProgram();
        p.painter.bindTarget(&p.target);
        p.painter.setParamsFromRenderable(this);
        glDisable(GL_BLEND);
        p.painter.bindTextureMode({
            .texture = m_blur->image(),
            .pos = p.rect.topLeft(),
            .srcRect = SkRect::Make(m_blur->imageSrcRect()),
            .dstSize = p.rect.size(),
            .srcTransform = AKTransform::Normal,
            .srcScale = 0.5f,
        });
        p.painter.drawRegion(p.damage);
    }
}

/*
void AKBackgroundBlurEffect::renderEvent(const AKRenderEvent &p)
{
    if (!p.target.image() || p.damage.isEmpty())
        return;

    glDisable(GL_BLEND);

    SkIRect pathLocalRect;

    if (clipMode() == Manual)
    {
        pathLocalRect = clip.getBounds().round();
        pathLocalRect.setXYWH(
            pathLocalRect.x(),
            pathLocalRect.y(),
            pathLocalRect.width() + 4 - (pathLocalRect.width() % 4),
            pathLocalRect.height() + 4 - (pathLocalRect.height() % 4));
    }
    else
        pathLocalRect = SkIRect::MakeSize(p.rect.size());

    const SkRect srcRect { SkRect::Make(pathLocalRect).makeOffset(p.rect.x(), p.rect.y()) };

    constexpr SkScalar d1 { 0.5f };
    constexpr SkScalar d2 { 0.25f };
    constexpr SkScalar d4 { 0.125f };
    const bool shrink { true };
    const SkScalar q { SkScalar(p.target.xyScale().x())};

    bool reblur { !bdt.damage.isEmpty() };
    bool copyAll { false };

    auto &bd { m_blurData.emplace(&p.target, BlurData()).first->second };

    if (m_backgroundCopy[m_i])
    {
        copyAll = m_backgroundCopy[m_i]->resize(pathLocalRect.size(), q * d1, shrink);
        reblur |= copyAll;
    }
    else
    {
        copyAll = true;
        m_backgroundCopy[m_i] = AKSurface::Make(pathLocalRect.size(), q * d1, false);
    }

    if (m_backgroundCopy[m_i]2)
    {
        m_backgroundCopy[m_i]2->resize(pathLocalRect.size(), q * d2, shrink);
    }
    else
    {
        m_backgroundCopy[m_i]2 = AKSurface::Make(pathLocalRect.size(), q * d2, false);
    }

    if (m_blur)
        reblur |= m_blur->resize(pathLocalRect.size(), q * d4, shrink);
    else
    {
        reblur = true;
        m_blur = AKSurface::Make(pathLocalRect.size(), q * d4, true);
    }

    assert(m_backgroundCopy[m_i]->size().width() % 4 == 0 && m_backgroundCopy[m_i]->size().height() % 4 == 0);

    int bW = m_blur->imageSrcRect().width() * 1.;
    int bH = m_blur->imageSrcRect().height() * 1.;
    SkRect blurSrc { SkRect::MakeWH(bW, bH) };

    if (reblur)
    {
        // Copy background 3/4 size
        const SkIRect backgroundCopySrc { SkIRect::MakeSize(m_backgroundCopy[m_i]->size()) };
        p.painter.bindTarget(m_backgroundCopy[m_i].get());
        p.painter.setAlpha(1.f);
        const SkScalar cf { 1.f };
        p.painter.setColorFactor(cf,cf,cf, 1.f);
        p.painter.bindTextureMode({
            .texture = p.target.image(),
            .pos = {0, 0},
            .srcRect = SkRect::MakeXYWH(srcRect.x() - p.target.viewport().x(), srcRect.y() - p.target.viewport().y(), srcRect.width(), srcRect.height()),
            .dstSize =  SkISize( m_backgroundCopy[m_i]->size().width(), m_backgroundCopy[m_i]->size().height()),
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

        // Scale again to 1/2

        p.painter.bindTarget(m_backgroundCopy[m_i]2.get());
        glDisable(GL_BLEND);
        float CF { 0.86f };
        p.painter.setColorFactor(CF, CF, 1.f, 1.f);
        p.painter.bindTextureMode({
            .texture = m_backgroundCopy[m_i]->image(),
            .pos = {0, 0},
            .srcRect = SkRect::Make(m_backgroundCopy[m_i]->imageSrcRect()),
            .dstSize =  SkISize( m_backgroundCopy[m_i]2->size().width(), m_backgroundCopy[m_i]2->size().height()),
            .srcTransform = AKTransform::Normal,
            .srcScale = 1.f
        });

        if (true || copyAll)
            p.painter.drawRect(backgroundCopySrc);
        else
            p.painter.drawRegion(bdt.damage);

        m_backgroundCopy[m_i]2->surface()->recordingContext()->asDirectContext()->resetContext();
        SkCanvas &c { *m_backgroundCopy[m_i]2->surface()->getCanvas() };
        SkPaint pa;
        pa.setBlendMode(SkBlendMode::kScreen);
        c.drawImageRect(m_backgroundCopy[m_i]2->image(),
                         SkRect::Make(m_backgroundCopy[m_i]2->imageSrcRect()),
                         SkRect::Make(m_backgroundCopy[m_i]2->imageSrcRect()),
                         SkFilterMode::kLinear,
                         &pa,
                         SkCanvas::kFast_SrcRectConstraint);
        m_backgroundCopy[m_i]2->surface()->recordingContext()->asDirectContext()->flush();


        // Copy to 1/4 and apply some blur

        SkScalar sigma = 6.f;
        SkPaint paint;

        m_blur->surface()->recordingContext()->asDirectContext()->resetContext();
        paint.setBlendMode(SkBlendMode::kSrc);
        paint.setImageFilter(SkImageFilters::Blur(sigma, sigma, SkTileMode::kMirror, nullptr));
        SkCanvas &c3 { *m_blur->surface()->getCanvas() };
        c3.drawImageRect(m_backgroundCopy[m_i]2->image(),
                         SkRect::Make(m_backgroundCopy[m_i]2->imageSrcRect()),
                         SkRect::Make(m_blur->imageSrcRect()),
                         SkFilterMode::kLinear,
                         &paint,
                         SkCanvas::kFast_SrcRectConstraint);
        m_blur->surface()->recordingContext()->asDirectContext()->flush();

        paint.setAlphaf(1.f);
        paint.setImageFilter(nullptr);
        paint.setBlendMode(SkBlendMode::kScreen);
        c3.drawImageRect(m_blur->image(),
                         blurSrc,
                         blurSrc,
                         SkFilterMode::kLinear,
                         &paint,
                         SkCanvas::kFast_SrcRectConstraint);
        m_blur->surface()->recordingContext()->asDirectContext()->flush();


        sigma = 6.f;
        paint.setImageFilter(SkImageFilters::Blur(sigma, sigma, SkTileMode::kMirror, nullptr));
        paint.setBlendMode(SkBlendMode::kSrc);
        c3.drawImageRect(m_blur->image(),
                         SkRect::Make(m_blur->imageSrcRect()),
                         blurSrc,
                         SkFilterMode::kLinear,
                         &paint,
                         SkCanvas::kFast_SrcRectConstraint);

        float t = 0.86f;
        paint.setImageFilter(nullptr);
        paint.setBlendMode(SkBlendMode::kSrcOver);
        paint.setColor({t,t,t,0.76f});
        c3.drawRect(blurSrc, paint);

        m_blur->surface()->recordingContext()->asDirectContext()->flush();
        c3.restore();
    }

    // This means using an SkPath
    if (clipMode() == Manual)
    {
        p.target.surface()->recordingContext()->asDirectContext()->resetContext();
        auto &c { *p.target.surface()->getCanvas() };
        c.save();

        SkRegion damage = p.damage;
        damage.op(srcRect.roundOut(), SkRegion::Op::kIntersect_Op);

        SkPath damagePath;
        damage.getBoundaryPath(&damagePath);
        damagePath.setIsVolatile(true);
        c.resetMatrix();
        c.scale(p.target.xyScale().x(), p.target.xyScale().y());
        c.translate(-p.target.viewport().x(), - p.target.viewport().y());

        c.translate(p.rect.x(), p.rect.y());
        c.clipPath(clip);

        c.translate(-p.rect.x(), -p.rect.y());

        c.clipPath(damagePath);

        SkPaint paint;
        paint.setAntiAlias(true);
        c.drawImageRect(m_blur->image(),
                        blurSrc,
                        srcRect,
                        SkFilterMode::kLinear,
                        &paint,
                        SkCanvas::kFast_SrcRectConstraint);

        p.target.surface()->recordingContext()->asDirectContext()->flush();
        c.restore();

        p.painter.bindProgram();
        p.painter.bindTarget(&p.target);
    }

    // Using the full node size
    else
    {
        p.painter.bindProgram();
        p.painter.bindTarget(&p.target);
        p.painter.setParamsFromRenderable(this);
        glDisable(GL_BLEND);
        p.painter.bindTextureMode({
            .texture = m_blur->image(),
            .pos = p.rect.topLeft(),
            .srcRect = blurSrc,
            .dstSize = p.rect.size(),
            .srcTransform = AKTransform::Normal,
            .srcScale = 1.f
        });
        p.painter.drawRegion(p.damage);
    }
}*/

/*
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

    const SkRect srcRect { SkRect::Make(pathLocalRect).makeOffset(p.rect.x(), p.rect.y()) };

    constexpr SkScalar d1 { 0.75f };
    constexpr SkScalar d2 { 0.5f };
    constexpr SkScalar d3 { 0.25f };
    constexpr SkScalar d4 { 0.125f };
    const SkScalar q { SkScalar(p.target.xyScale().x())};

    bool reblur { !bdt.damage.isEmpty() };
    bool copyAll { false };

    auto &bd { m_blurData.emplace(&p.target, BlurData()).first->second };

    if (m_backgroundCopy[m_i])
    {
        copyAll = m_backgroundCopy[m_i]->resize(pathLocalRect.size(), q * d1, false);
        reblur |= copyAll;
    }
    else
    {
        copyAll = true;
        m_backgroundCopy[m_i] = AKSurface::Make(pathLocalRect.size(), q * d1, false);
    }

    if (m_backgroundCopy[m_i]2)
    {
        m_backgroundCopy[m_i]2->resize(pathLocalRect.size(), q * d2, false);
    }
    else
    {
        m_backgroundCopy[m_i]2 = AKSurface::Make(pathLocalRect.size(), q * d2, false);
    }

    if (m_backgroundCopy[m_i]3)
    {
        m_backgroundCopy[m_i]3->resize(pathLocalRect.size(), q * d3, false);
    }
    else
    {
        m_backgroundCopy[m_i]3 = AKSurface::Make(pathLocalRect.size(), q * d3, false);
    }

    if (m_blur)
        reblur |= m_blur->resize(pathLocalRect.size(), q * d4, false);
    else
    {
        reblur = true;
        m_blur = AKSurface::Make(pathLocalRect.size(), q * d4, true);
    }

    if (reblur)
    {
        // Copy background 3/4 size
        const SkIRect backgroundCopySrc { SkIRect::MakeSize(m_backgroundCopy[m_i]->size()) };
        p.painter.bindTarget(m_backgroundCopy[m_i].get());
        const SkScalar up { 1.f };
        p.painter.setColorFactor(0.8, 0.8, up, up);
        p.painter.bindTextureMode({
            .texture = p.target.image(),
            .pos = {0, 0},
            .srcRect = SkRect::MakeXYWH(srcRect.x() - p.target.viewport().x(), srcRect.y() - p.target.viewport().y(), srcRect.width(), srcRect.height()),
            .dstSize =  SkISize( m_backgroundCopy[m_i]->size().width(), m_backgroundCopy[m_i]->size().height()),
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

        // Scale again to 1/2

        p.painter.bindTarget(m_backgroundCopy[m_i]2.get());
        p.painter.setColorFactor(1.f, 1.f, 1.f, 1.f);
        p.painter.setAlpha(1.f);
        glDisable(GL_BLEND);
        p.painter.bindTextureMode({
            .texture = m_backgroundCopy[m_i]->image(),
            .pos = {0, 0},
            .srcRect = SkRect::Make(m_backgroundCopy[m_i]->imageSrcRect()),
            .dstSize =  SkISize( m_backgroundCopy[m_i]2->size().width(), m_backgroundCopy[m_i]2->size().height()),
            .srcTransform = AKTransform::Normal,
            .srcScale = 1.f
        });

        if (copyAll)
            p.painter.drawRect(backgroundCopySrc);
        else
            p.painter.drawRegion(bdt.damage);

        // Apply tint

        p.painter.bindColorMode();
        const SkScalar add { 1.5f };
        p.painter.setColor({add, add, add, add});
        p.painter.setAlpha(0.3f);
        glEnable(GL_BLEND);

        if (copyAll)
            p.painter.drawRect(backgroundCopySrc);
        else
            p.painter.drawRegion(bdt.damage);


        // Copy to 1/4 and apply some blur

        m_blur->surface()->recordingContext()->asDirectContext()->resetContext();
        SkCanvas &c3 { *m_backgroundCopy[m_i]3->surface()->getCanvas() };
        c3.save();
        m_brush1.setAntiAlias(false);
        m_brush1.setBlendMode(SkBlendMode::kSrc);
        c3.drawImageRect(m_backgroundCopy[m_i]2->image(),
                        SkRect::Make(m_backgroundCopy[m_i]2->imageSrcRect()),
                        SkRect::Make(m_backgroundCopy[m_i]3->imageSrcRect()),
                        SkFilterMode::kLinear,
                        &m_brush1,
                        SkCanvas::kFast_SrcRectConstraint);
        m_backgroundCopy[m_i]3->surface()->recordingContext()->asDirectContext()->flush();
        c3.restore();

        // Copy to 1/8 and aply more blur
        SkCanvas &c { *m_blur->surface()->getCanvas() };
        c.save();
        m_brush2.setAntiAlias(false);
        m_brush2.setBlendMode(SkBlendMode::kSrc);
        c.drawImageRect(m_backgroundCopy[m_i]3->image(),
                        SkRect::Make(m_backgroundCopy[m_i]3->imageSrcRect()),
                        SkRect::Make(m_blur->imageSrcRect()),
                        SkFilterMode::kLinear,
                        &m_brush2,
                        SkCanvas::kFast_SrcRectConstraint);
        m_blur->surface()->recordingContext()->asDirectContext()->flush();
        c.restore();
    }

    // This means using an SkPath
    if (clipMode() == Manual)
    {
        p.target.surface()->recordingContext()->asDirectContext()->resetContext();
        auto &c { *p.target.surface()->getCanvas() };
        c.save();

        SkRegion damage = p.damage;
        damage.op(srcRect.roundOut(), SkRegion::Op::kIntersect_Op);

        SkPath damagePath;
        damage.getBoundaryPath(&damagePath);
        damagePath.setIsVolatile(true);
        c.resetMatrix();
        c.scale(p.target.xyScale().x(), p.target.xyScale().y());
        c.translate(-p.target.viewport().x(), - p.target.viewport().y());

        c.translate(p.rect.x(), p.rect.y());
        c.clipPath(clip);
        c.translate(-p.rect.x(), -p.rect.y());

        c.clipPath(damagePath);

        SkPaint paint;
        paint.setAntiAlias(true);
        c.drawImageRect(m_blur->image(),
                        SkRect::Make(m_blur->imageSrcRect()),
                        srcRect,
                        SkFilterMode::kLinear,
                        &paint,
                        SkCanvas::kFast_SrcRectConstraint);

        p.target.surface()->recordingContext()->asDirectContext()->flush();
        c.restore();

        p.painter.bindProgram();
        p.painter.bindTarget(&p.target);
    }

    // Using the full node size
    else
    {
        p.painter.bindProgram();
        p.painter.bindTarget(&p.target);
        p.painter.setParamsFromRenderable(this);
        glDisable(GL_BLEND);
        p.painter.bindTextureMode({
            .texture = m_blur->image(),
            .pos = p.rect.topLeft(),
            .srcRect = SkRect::Make(m_blur->imageSrcRect()),
            .dstSize = p.rect.size(),
            .srcTransform = AKTransform::Normal,
            .srcScale = 1.f
        });
        p.painter.drawRegion(p.damage);
    }
}
*/
