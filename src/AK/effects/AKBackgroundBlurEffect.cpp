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

    if (!m_brush1.getImageFilter() || chgs.test(CHSigma))
    {
        const SkScalar sigma = 160.f;
        m_brush1.setImageFilter(SkImageFilters::Blur(sigma, sigma, SkTileMode::kMirror, nullptr));
        m_brush1.setImageFilter(SkImageFilters::Blur(AKTheme::BlurVibrancySigma * 0., AKTheme::BlurVibrancySigma * 0., SkTileMode::kMirror, nullptr));
    }
}

void AKBackgroundBlurEffect::renderEvent(const AKRenderEvent &p)
{
    if (!p.target.image() || p.damage.isEmpty())
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

    const SkRect srcRect { SkRect::Make(pathLocalRect).makeOffset(p.rect.x(), p.rect.y()) };

    constexpr SkScalar d1 { 0.25f };
    constexpr SkScalar d4 { 0.125f };
    const bool shrink { true };
    const SkScalar q { SkScalar(p.target.xyScale().x())};

    bool reblur { !bdt.damage.isEmpty() };
    bool copyAll { false };

    auto &bd { m_blurData.emplace(&p.target, BlurData()).first->second };

    SkISize copySize = pathLocalRect.size();
    copySize.fWidth += 16 - (copySize.fWidth % 16);
    copySize.fHeight += 16 - (copySize.fHeight % 16);

    if (bd.backgroundCopy)
    {
        copyAll = bd.backgroundCopy->resize(copySize, q * d1, false);
        reblur |= copyAll;
    }
    else
    {
        copyAll = true;
        bd.backgroundCopy = AKSurface::Make(copySize, q * d1, false);
    }

    /*
    if (bd.backgroundCopy2)
    {
        bd.backgroundCopy2->resize(pathLocalRect.size(), q * d2, shrink);
    }
    else
    {
        bd.backgroundCopy2 = AKSurface::Make(pathLocalRect.size(), q * d2, false);
    }  */

    if (bd.blur)
        reblur |= bd.blur->resize(copySize, q * d4, shrink);
    else
    {
        reblur = true;
        bd.blur = AKSurface::Make(copySize, q * d4, true);
    }

    SkIRect finalSrcRect = SkIRect::MakeWH(bd.blur->imageSrcRect().width() /2 , bd.blur->imageSrcRect().height() /2 );

    if (reblur)
    {
        // Copy background 1/4 size
        const SkIRect backgroundCopySrc { SkIRect::MakeSize(bd.backgroundCopy->size()) };
        p.painter.bindTarget(bd.backgroundCopy.get());
        glDisable(GL_BLEND);
        p.painter.setAlpha(1.f);
        p.painter.setColorFactor(1.f, 1.f, 1.f, 1.f);
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

        // Copy to 1/4 and apply some blur
        p.painter.bindTarget(bd.blur.get());
        p.painter.bindTextureMode({
            .texture = bd.backgroundCopy->image(),
            .pos = {0, 0},
            .srcRect = SkRect::Make(bd.backgroundCopy->imageSrcRect()),
            .dstSize =  SkISize( bd.blur->size().width(), bd.blur->size().height()),
            .srcTransform = AKTransform::Normal,
            .srcScale = 1.f
        });

        p.painter.blendMode = AKPainter::Vibrancy1;
        p.painter.drawRect(backgroundCopySrc);

        p.painter.bindTextureMode({
            .texture = bd.blur->image(),
            .pos = {0, 0},
            .srcRect = SkRect::Make(bd.blur->imageSrcRect()),
            .dstSize =  SkISize( bd.blur->size().width() / 2, bd.blur->size().height() / 2),
            .srcTransform = AKTransform::Normal,
            .srcScale = 1.f
        });

        p.painter.blendMode = AKPainter::Vibrancy2;
        p.painter.drawRect(backgroundCopySrc);
        p.painter.blendMode = AKPainter::Normal;
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
        c.drawImageRect(bd.blur->image(),
                        SkRect::Make(finalSrcRect),
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
            .texture = bd.blur->image(),
            .pos = p.rect.topLeft(),
            .srcRect = SkRect::Make(finalSrcRect),
            .dstSize = p.rect.size(),
            .srcTransform = AKTransform::Normal,
            .srcScale = 1.f
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

    if (bd.backgroundCopy)
    {
        copyAll = bd.backgroundCopy->resize(pathLocalRect.size(), q * d1, shrink);
        reblur |= copyAll;
    }
    else
    {
        copyAll = true;
        bd.backgroundCopy = AKSurface::Make(pathLocalRect.size(), q * d1, false);
    }

    if (bd.backgroundCopy2)
    {
        bd.backgroundCopy2->resize(pathLocalRect.size(), q * d2, shrink);
    }
    else
    {
        bd.backgroundCopy2 = AKSurface::Make(pathLocalRect.size(), q * d2, false);
    }

    if (bd.blur)
        reblur |= bd.blur->resize(pathLocalRect.size(), q * d4, shrink);
    else
    {
        reblur = true;
        bd.blur = AKSurface::Make(pathLocalRect.size(), q * d4, true);
    }

    assert(bd.backgroundCopy->size().width() % 4 == 0 && bd.backgroundCopy->size().height() % 4 == 0);

    int bW = bd.blur->imageSrcRect().width() * 1.;
    int bH = bd.blur->imageSrcRect().height() * 1.;
    SkRect blurSrc { SkRect::MakeWH(bW, bH) };

    if (reblur)
    {
        // Copy background 3/4 size
        const SkIRect backgroundCopySrc { SkIRect::MakeSize(bd.backgroundCopy->size()) };
        p.painter.bindTarget(bd.backgroundCopy.get());
        p.painter.setAlpha(1.f);
        const SkScalar cf { 1.f };
        p.painter.setColorFactor(cf,cf,cf, 1.f);
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

        // Scale again to 1/2

        p.painter.bindTarget(bd.backgroundCopy2.get());
        glDisable(GL_BLEND);
        float CF { 0.86f };
        p.painter.setColorFactor(CF, CF, 1.f, 1.f);
        p.painter.bindTextureMode({
            .texture = bd.backgroundCopy->image(),
            .pos = {0, 0},
            .srcRect = SkRect::Make(bd.backgroundCopy->imageSrcRect()),
            .dstSize =  SkISize( bd.backgroundCopy2->size().width(), bd.backgroundCopy2->size().height()),
            .srcTransform = AKTransform::Normal,
            .srcScale = 1.f
        });

        if (true || copyAll)
            p.painter.drawRect(backgroundCopySrc);
        else
            p.painter.drawRegion(bdt.damage);

        bd.backgroundCopy2->surface()->recordingContext()->asDirectContext()->resetContext();
        SkCanvas &c { *bd.backgroundCopy2->surface()->getCanvas() };
        SkPaint pa;
        pa.setBlendMode(SkBlendMode::kScreen);
        c.drawImageRect(bd.backgroundCopy2->image(),
                         SkRect::Make(bd.backgroundCopy2->imageSrcRect()),
                         SkRect::Make(bd.backgroundCopy2->imageSrcRect()),
                         SkFilterMode::kLinear,
                         &pa,
                         SkCanvas::kFast_SrcRectConstraint);
        bd.backgroundCopy2->surface()->recordingContext()->asDirectContext()->flush();


        // Copy to 1/4 and apply some blur

        SkScalar sigma = 6.f;
        SkPaint paint;

        bd.blur->surface()->recordingContext()->asDirectContext()->resetContext();
        paint.setBlendMode(SkBlendMode::kSrc);
        paint.setImageFilter(SkImageFilters::Blur(sigma, sigma, SkTileMode::kMirror, nullptr));
        SkCanvas &c3 { *bd.blur->surface()->getCanvas() };
        c3.drawImageRect(bd.backgroundCopy2->image(),
                         SkRect::Make(bd.backgroundCopy2->imageSrcRect()),
                         SkRect::Make(bd.blur->imageSrcRect()),
                         SkFilterMode::kLinear,
                         &paint,
                         SkCanvas::kFast_SrcRectConstraint);
        bd.blur->surface()->recordingContext()->asDirectContext()->flush();

        paint.setAlphaf(1.f);
        paint.setImageFilter(nullptr);
        paint.setBlendMode(SkBlendMode::kScreen);
        c3.drawImageRect(bd.blur->image(),
                         blurSrc,
                         blurSrc,
                         SkFilterMode::kLinear,
                         &paint,
                         SkCanvas::kFast_SrcRectConstraint);
        bd.blur->surface()->recordingContext()->asDirectContext()->flush();


        sigma = 6.f;
        paint.setImageFilter(SkImageFilters::Blur(sigma, sigma, SkTileMode::kMirror, nullptr));
        paint.setBlendMode(SkBlendMode::kSrc);
        c3.drawImageRect(bd.blur->image(),
                         SkRect::Make(bd.blur->imageSrcRect()),
                         blurSrc,
                         SkFilterMode::kLinear,
                         &paint,
                         SkCanvas::kFast_SrcRectConstraint);

        float t = 0.86f;
        paint.setImageFilter(nullptr);
        paint.setBlendMode(SkBlendMode::kSrcOver);
        paint.setColor({t,t,t,0.76f});
        c3.drawRect(blurSrc, paint);

        bd.blur->surface()->recordingContext()->asDirectContext()->flush();
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
        c.drawImageRect(bd.blur->image(),
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
            .texture = bd.blur->image(),
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
        // Copy background 3/4 size
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

        // Scale again to 1/2

        p.painter.bindTarget(bd.backgroundCopy2.get());
        p.painter.setColorFactor(1.f, 1.f, 1.f, 1.f);
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

        bd.blur->surface()->recordingContext()->asDirectContext()->resetContext();
        SkCanvas &c3 { *bd.backgroundCopy3->surface()->getCanvas() };
        c3.save();
        m_brush1.setAntiAlias(false);
        m_brush1.setBlendMode(SkBlendMode::kSrc);
        c3.drawImageRect(bd.backgroundCopy2->image(),
                        SkRect::Make(bd.backgroundCopy2->imageSrcRect()),
                        SkRect::Make(bd.backgroundCopy3->imageSrcRect()),
                        SkFilterMode::kLinear,
                        &m_brush1,
                        SkCanvas::kFast_SrcRectConstraint);
        bd.backgroundCopy3->surface()->recordingContext()->asDirectContext()->flush();
        c3.restore();

        // Copy to 1/8 and aply more blur
        SkCanvas &c { *bd.blur->surface()->getCanvas() };
        c.save();
        m_brush2.setAntiAlias(false);
        m_brush2.setBlendMode(SkBlendMode::kSrc);
        c.drawImageRect(bd.backgroundCopy3->image(),
                        SkRect::Make(bd.backgroundCopy3->imageSrcRect()),
                        SkRect::Make(bd.blur->imageSrcRect()),
                        SkFilterMode::kLinear,
                        &m_brush2,
                        SkCanvas::kFast_SrcRectConstraint);
        bd.blur->surface()->recordingContext()->asDirectContext()->flush();
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
    }

    // Using the full node size
    else
    {
        p.painter.bindProgram();
        p.painter.bindTarget(&p.target);
        p.painter.setParamsFromRenderable(this);
        glDisable(GL_BLEND);
        p.painter.bindTextureMode({
            .texture = bd.blur->image(),
            .pos = p.rect.topLeft(),
            .srcRect = SkRect::Make(bd.blur->imageSrcRect()),
            .dstSize = p.rect.size(),
            .srcTransform = AKTransform::Normal,
            .srcScale = 1.f
        });
        p.painter.drawRegion(p.damage);
    }
}
*/
