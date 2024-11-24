#include <LLauncher.h>
#include <LCompositor.h>
#include <LOutput.h>
#include <LSurface.h>
#include <LLog.h>

#include <AKScene.h>
#include <AKImage.h>
#include <AKLayout.h>
#include <AKSolidColor.h>

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

using namespace AK;
using namespace Louvre;

static sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeSRGB();
static SkSurfaceProps skSurfaceProps(0, kUnknown_SkPixelGeometry);

class Compositor final : public LCompositor
{
public:
    LFactoryObject *createObjectRequest(LFactoryObject::Type objectType, const void *params) override;
    AKScene scene;
};

class Surface final : public LSurface
{
public:
    using LSurface::LSurface;
    AKImage node { static_cast<Compositor*>(compositor())->scene.root() };
};

class Output final : public LOutput
{
public:
    using LOutput::LOutput;

    void initializeGL() override
    {
        setScale(1.5f);
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

        target = static_cast<Compositor*>(compositor())->scene.createTarget();
    }

    void paintGL() override
    {
        static Float32 phase { 0.f };
        phase += 0.1f;

        layout.styleSetFlexDirection(YGFlexDirectionRow);
        layout.styleSetWidth(size().w());
        layout.styleSetHeight(size().h());
        layout.styleSetGap(YGGutterAll, 50.f * (1.f + SkScalarSin(phase)));
        layout.styleSetPadding(YGEdgeAll, 200.f);

        for (Int32 i = 0; i < 4; i++)
        {
            solidColors[i].styleSetFlex(1.f);
            solidColors[i].styleSetWidthPercent(100);
            solidColors[i].styleSetHeightPercent(100);
        }

        const GrGLFramebufferInfo fbInfo
        {
            .fFBOID = framebuffer()->id(),
            .fFormat = GL_RGB8_OES
        };

        const GrBackendRenderTarget backendTarget(
            realBufferSize().w(),
            realBufferSize().h(),
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

        GrGLTextureInfo skTextureInfo;
        GrBackendTexture skTexture;

        for (Surface *s : (const std::list<Surface*>&)(compositor()->surfaces()))
        {

            const LPoint &pos { s->rolePos() };
            s->node.styleSetPositionType(YGPositionTypeAbsolute);
            s->node.styleSetPosition(YGEdgeLeft, pos.x());
            s->node.styleSetPosition(YGEdgeTop, pos.y());
            s->node.styleSetWidth(s->size().w());
            s->node.styleSetHeight(s->size().h());

            if (s->mapped())
            {
                skTextureInfo.fFormat = GL_BGRA8_EXT;
                skTextureInfo.fID = s->texture()->id(this);
                skTextureInfo.fTarget = s->texture()->target();

                skTexture = GrBackendTexture(
                    s->texture()->sizeB().w(),
                    s->texture()->sizeB().h(),
                    GrMipMapped::kYes,
                    skTextureInfo);

                auto const skImage = SkImages::BorrowTextureFrom(
                    target->surface->recordingContext()->asDirectContext(),
                    skTexture,
                    GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                    kBGRA_8888_SkColorType,
                    SkAlphaType::kPremul_SkAlphaType,
                    colorSpace,
                    nullptr,
                    nullptr);

                s->node.setImage(skImage);
                s->node.setSrcRect(SkRect::MakeXYWH(
                    s->srcRect().x(), s->srcRect().y(),
                    s->srcRect().w(), s->srcRect().h()));
                s->node.setScale(s->bufferScale());
                s->node.setTransform(static_cast<AKTransform>(s->bufferTransform()));

                Int32 n;
                const LBox *boxes { s->damage().boxes(&n) };
                SkRegion region;
                region.setRects((const SkIRect*)boxes, n);
                s->node.addDamage(region);

                boxes = s->opaqueRegion().boxes(&n);
                region.setRects((const SkIRect*)boxes, n);
                s->node.setOpaqueRegion(&region);
            }
        }

        if (frame < buffersCount() || buffersCount() == 1)
        {
            frame++;
            age = 0;
        }
        else
        {
            age = buffersCount();
        }

        target->age = age;
        target->pos.fX = pos().x();
        target->pos.fY = pos().y();
        target->scale = fractionalScale();
        target->transform = static_cast<AKTransform>(transform());
        static_cast<Compositor*>(compositor())->scene.render(target);
        target->surface->flush();

        for (Surface *s : (const std::list<Surface*>&)(compositor()->surfaces()))
        {
            if (s->node.insideLastTarget())
            {
                s->sendOutputEnterEvent(this);
                if (s->node.renderedOnLastTarget())
                    s->requestNextFrame();
            }
            else
            {
                s->sendOutputLeaveEvent(this);
            }
        }
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
    }

    void moveGL() override
    {
        frame = 0;
    }

    void uninitializeGL() override
    {

    }

    AKLayout layout { static_cast<Compositor*>(compositor())->scene.root() };

    AKSolidColor solidColors[4]
    {
        {SK_ColorRED,   &layout},
        {SK_ColorGREEN, &layout},
        {SK_ColorBLUE,  &layout},
        {SK_ColorWHITE, &layout}
    };

    GrContextOptions contextOptions;
    sk_sp<GrDirectContext> context;
    std::shared_ptr<AKTarget> target;
    UInt32 age { 0 };
    UInt32 frame { 0 };
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
