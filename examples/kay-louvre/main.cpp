#include <LLauncher.h>
#include <LCompositor.h>
#include <LOutput.h>
#include <LOutputMode.h>
#include <LSurface.h>
#include <LCursor.h>
#include <LLog.h>

#include <AK/Widgets/AKSubScene.h>
#include <AK/Widgets/AKImage.h>
#include <AK/Widgets/AKContainer.h>
#include <AK/Widgets/AKRenderableRect.h>

#include <AK/AKScene.h>

#include <include/gpu/gl/GrGLInterface.h>
#include <include/gpu/gl/GrGLTypes.h>
#include <include/gpu/GrDirectContext.h>
#include <include/gpu/GrBackendSurface.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/gpu/ganesh/SkImageGanesh.h>
#include <include/core/SkImage.h>
#include <include/core/SkSurface.h>
#include <include/core/SkCanvas.h>
#include <include/gpu/gl/GrGLAssembleInterface.h>
#include <include/core/SkColorSpace.h>
#include <include/effects/SkImageFilters.h>
#include <include/effects/SkGradientShader.h>

using namespace AK;
using namespace Louvre;

static sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeSRGB();
static SkSurfaceProps skSurfaceProps(0, kUnknown_SkPixelGeometry);


static sk_sp<SkImage> louvreTex2SkiaImage(LTexture *texture, GrRecordingContext *ctx, LOutput *o)
{
    if (!texture || !ctx)
        return nullptr;

    GrGLTextureInfo skTextureInfo;
    GrBackendTexture skTexture;
    skTextureInfo.fFormat = GL_BGRA8_EXT;
    skTextureInfo.fID = texture->id(o);
    skTextureInfo.fTarget = texture->target();

    skTexture = GrBackendTexture(
        texture->sizeB().w(),
        texture->sizeB().h(),
        GrMipMapped::kYes,
        skTextureInfo);

    return SkImages::BorrowTextureFrom(
        ctx,
        skTexture,
        GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
        kBGRA_8888_SkColorType,
        SkAlphaType::kPremul_SkAlphaType,
        colorSpace,
        nullptr,
        nullptr);
}

class Compositor final : public LCompositor
{
public:
    Compositor() noexcept
    {
        background.layout().setPositionType(YGPositionTypeAbsolute);
        surfaces.layout().setPositionType(YGPositionTypeAbsolute);
        overlay.layout().setPositionType(YGPositionTypeAbsolute);

        AKBrush brush { true, false };
        brush.setBlendMode(SkBlendMode::kClear);

        AKPen pen;
        pen.setStrokeWidth(10);
        solidColor.setPen(pen);
        solidColor.setBrush(brush);
        solidColor.layout().setPosition(YGEdgeRight, 0.f);
        solidColor.layout().setPositionType(YGPositionTypeAbsolute);
        solidColor.layout().setWidth(100.f);
        solidColor.layout().setHeight(100.f);
        solidColor.layout().setMargin(YGEdgeAll, 10.f);

        solidColor2.setPen(pen);
        solidColor2.layout().setPosition(YGEdgeLeft, 0.f);
        solidColor2.layout().setPositionType(YGPositionTypeAbsolute);
        solidColor2.layout().setWidth(100.f);
        solidColor2.layout().setHeight(100.f);
        solidColor2.layout().setMargin(YGEdgeAll, 10.f);

        subSceneBackground.layout().setWidthPercent(100.f);
        subSceneBackground.layout().setHeightPercent(100.f);

        subScene.layout().setWidth(300.f);
        subScene.layout().setHeight(300.f);
        subScene.layout().setMargin(YGEdgeAll, 50.f);

    }
    LFactoryObject *createObjectRequest(LFactoryObject::Type objectType, const void *params) override;
    AKScene scene;
    AKContainer root;

    AKContainer background { YGFlexDirectionColumn, false, &root };
    AKContainer surfaces { YGFlexDirectionColumn, false, &root };
    AKContainer overlay { YGFlexDirectionColumn, false, &root };

    AKSubScene subScene { &overlay };
    AKRenderableRect subSceneBackground { SK_ColorWHITE, &subScene };
    AKRenderableRect solidColor { SK_ColorRED, &subSceneBackground };
    AKRenderableRect solidColor2 { SK_ColorYELLOW, &subSceneBackground };
};

class Surface final : public LSurface
{
public:
    using LSurface::LSurface;
    Compositor *comp() const noexcept { return static_cast<Compositor*>(compositor()); }

    AKImage node { &comp()->surfaces };

    void orderChanged() override
    {
        AKNode *prev { prevSurface() ? &static_cast<Surface*>(prevSurface())->node : nullptr };
        node.insertAfter(prev);
        repaintOutputs();
    }
};

class Output final : public LOutput
{
public:
    using LOutput::LOutput;

    Compositor *comp() const noexcept { return static_cast<Compositor*>(compositor()); }

    void updateGradient() noexcept
    {
        const SkPoint gradientPoints    [2] { SkPoint(0.f, 0.f), SkPoint(size().w(), size().h()) };
        const SkScalar gradientPositions[2] { 0.f, 1.f };
        const SkColor gradientColors    [2] { SK_ColorMAGENTA, SK_ColorGREEN };
        auto gradient = SkGradientShader::MakeLinear(
            gradientPoints,
            gradientColors,
            gradientPositions,
            2, SkTileMode::kRepeat);
        AKBrush gradientBrush;
        gradientBrush.setShader(gradient);
        backgroundGradient.setBrush(gradientBrush);
        backgroundGradient.layout().setPositionType(YGPositionTypeAbsolute);
        backgroundGradient.layout().setPosition(YGEdgeLeft, pos().x());
        backgroundGradient.layout().setPosition(YGEdgeTop, pos().y());
        backgroundGradient.layout().setWidth(size().w());
        backgroundGradient.layout().setHeight(size().h());
        backgroundGradient.setOpaqueRegion(AK_IRECT_INF);
        backgroundGradient.addDamage(AK_IRECT_INF);
    }

    void initializeGL() override
    {
        //setScale(3.1);
        enableFractionalOversampling(false);
        frame = 0;
        auto interface = GrGLMakeAssembledInterface(nullptr, (GrGLGetProc)*[](void *, const char *p) -> void * {
            return (void *)eglGetProcAddress(p);
        });

        contextOptions.fShaderCacheStrategy = GrContextOptions::ShaderCacheStrategy::kBackendBinary;
        contextOptions.fAvoidStencilBuffers = true;
        contextOptions.fPreferExternalImagesOverES3 = true;
        contextOptions.fDisableGpuYUVConversion = true;
        contextOptions.fReduceOpsTaskSplitting = GrContextOptions::Enable::kNo;
        contextOptions.fReducedShaderVariations = false;

        context = GrDirectContext::MakeGL(interface, contextOptions);

        if (!context.get())
        {
            LLog::fatal("Failed to create Skia context.");
            exit(1);
        }

        target = comp()->scene.createTarget();
        target->root = &comp()->root;
        updateGradient();
    }

    void paintGL() override
    {
        phase += 0.01f;

        comp()->subScene.layout().setWidth(300.f + SkScalarCos(phase) * 50.f);

        Int32 n;
        const LBox *boxes;
        SkRegion region, outDamage;
        LRegion damage;
        const GrGLFramebufferInfo fbInfo
        {
            .fFBOID = framebuffer()->id(),
            .fFormat = GL_RGB8_OES
        };

        const GrBackendRenderTarget backendTarget(
            currentMode()->sizeB().w(),
            currentMode()->sizeB().h(),
            0, 0,
            fbInfo);

        target->surface = SkSurfaces::WrapBackendRenderTarget(
            context.get(),
            backendTarget,
            fbInfo.fFBOID == 0 ? GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin : GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
            SkColorType::kRGB_888x_SkColorType,
            colorSpace,
            &skSurfaceProps);

        if (!target->surface)
        {
            LLog::fatal("No SkSurface.");
            exit(1);
        }

        for (Surface *s : (const std::list<Surface*>&)(compositor()->surfaces()))
        {
            s->node.setVisible(!s->cursorRole() && !s->minimized() && s->mapped());

            if (!s->node.visible())
                continue;

            const LPoint &pos { s->rolePos() };
            s->node.layout().setPositionType(YGPositionTypeAbsolute);
            s->node.layout().setPosition(YGEdgeLeft, pos.x());
            s->node.layout().setPosition(YGEdgeTop, pos.y());
            s->node.layout().setWidth(s->size().w());
            s->node.layout().setHeight(s->size().h());
            s->node.setImage(louvreTex2SkiaImage(s->texture(), context.get(), this));
            s->node.setImageSrcRect(SkRect::MakeXYWH(
                s->srcRect().x(), s->srcRect().y(),
                s->srcRect().w(), s->srcRect().h()));
            s->node.setImageScale(s->bufferScale());
            s->node.setImageTransform(static_cast<AKTransform>(s->bufferTransform()));


            boxes = s->damage().boxes(&n);
            region.setRects((const SkIRect*)boxes, n);
            s->node.addDamage(region);

            boxes = s->opaqueRegion().boxes(&n);
            region.setRects((const SkIRect*)boxes, n);
            s->node.setOpaqueRegion(region);
        }

        if (needsFullRepaint())
        {
            frame = 0;
            age = 0;
        }
        else if (frame < buffersCount() || buffersCount() == 1)
        {
            frame++;
            age = 0;
        }
        else
            age = buffersCount();

        //SkRegion inClip;
        //inClip.setRect(SkIRect::MakeXYWH(200 + 200 * SkScalarCos(phase), 200, 200, 200));
        //target->inClipRegion = &inClip;

        target->outDamageRegion = &outDamage;
        target->age = age;
        target->viewport = SkRect::MakeXYWH(pos().x(), pos().y(), size().w(), size().h());
        target->transform = static_cast<AKTransform>(transform());
        target->dstRect = SkIRect::MakeXYWH(0, 0, currentMode()->sizeB().w(), currentMode()->sizeB().h());


        //target->surface->getCanvas()->clear(SK_ColorBLUE);
        static_cast<Compositor*>(compositor())->scene.render(target);
        target->surface->flush();

        for (Surface *s : (const std::list<Surface*>&)(compositor()->surfaces()))
        {
            if (s->node.renderedOnLastTarget())
                s->requestNextFrame();

            for (LOutput *o : compositor()->outputs())
            {
                if (LRect(s->rolePos(), s->size()).intersects(o->rect()))
                    s->sendOutputEnterEvent(o);
                else
                    s->sendOutputLeaveEvent(o);
            }
        }

        outDamage.translate(pos().x(), pos().y());

        SkRegion::Iterator it(outDamage);
        while (!it.done())
        {
            damage.addRect(it.rect().x(), it.rect().y(), it.rect().width(), it.rect().height());
            it.next();
        }

        setBufferDamage(&damage);
    }

    void resizeGL() override
    {
        frame = 0;
        LPoint pos;
        for (LOutput *o : compositor()->outputs())
        {
            o->setPos(pos);
            pos.setX(pos.x() + o->size().w());
        }
        repaint();
        updateGradient();
    }

    void moveGL() override
    {
        frame = 0;
        repaint();
        updateGradient();
    }

    void uninitializeGL() override
    {
        static_cast<Compositor*>(compositor())->scene.destroyTarget(target);
    }

    AKRenderableRect backgroundGradient { &comp()->background };
    GrContextOptions contextOptions;
    sk_sp<GrDirectContext> context;
    AKTarget *target { nullptr };
    UInt32 age { 0 };
    UInt32 frame { 0 };
    float phase { 0.f };
};

LFactoryObject *Compositor::createObjectRequest(LFactoryObject::Type objectType, const void *params)
{
    if (objectType == LFactoryObject::Type::LOutput)
        return new Output(params);

    if (objectType == LFactoryObject::Type::LSurface)
        return new Surface(params);

    return nullptr;
}

int main(void)
{
    setenv("LOUVRE_WAYLAND_DISPLAY", "louvre", 0);

    LLauncher::startDaemon();

    Compositor compositor;

    if (!compositor.start())
        exit(1);

    while (compositor.state() != LCompositor::Uninitialized)
        compositor.processLoop(-1);

    return 0;
}
