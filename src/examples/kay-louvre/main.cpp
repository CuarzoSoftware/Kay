#include <LLauncher.h>
#include <LCompositor.h>
#include <LOutput.h>
#include <LOutputMode.h>
#include <LSurface.h>
#include <LCursor.h>
#include <LLog.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LKeyboard.h>

#include <AK/nodes/AKSubScene.h>
#include <AK/nodes/AKImage.h>
#include <AK/nodes/AKContainer.h>
#include <AK/nodes/AKSolidColor.h>
#include <AK/nodes/AKRoundContainer.h>
#include <AK/effects/AKBackgroundShadowEffect.h>

#include <AK/AKScene.h>

#include <cassert>
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
#include <include/utils/SkParsePath.h>
#include <iostream>

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
        GrMipMapped::kNo,
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

class Button : public AKRoundContainer
{
public:
    Button(AKNode *parent) noexcept : AKRoundContainer(AKBorderRadius::Make(0), parent)
    {
        shadowEffect.enableShadowClipping(true);
        layout().setWidth(100);
        layout().setHeight(100);
        background.layout().setWidthPercent(100);
        background.layout().setHeightPercent(100);
    }

    AKSolidColor background { SkColorSetARGB(255, rand()%255, rand()%255, rand()%255), this };
    AKBackgroundShadowEffect shadowEffect { AKBackgroundShadowEffect::Box,
        50.f, {-10, 10}, SkColorSetARGB(255, rand()%255, rand()%255, rand()%255), true, this };
    float rad { 0.f };
};

class Compositor final : public LCompositor
{
public:
    Compositor() noexcept
    {
        scene.setClearColor(SK_ColorWHITE);
        surfaces.layout().setPositionType(YGPositionTypeAbsolute);
    }

    LFactoryObject *createObjectRequest(LFactoryObject::Type objectType, const void *params) override;
    AKScene scene;
    AKContainer root;
    AKContainer background { YGFlexDirectionColumn, false, &root };
    AKContainer surfaces { YGFlexDirectionColumn, false, &root };
    AKContainer overlay { YGFlexDirectionColumn, false, &root };
};

class Surface final : public LSurface
{
public:
    Surface(const void *data) noexcept : LSurface(data) {
        node.layout().setPositionType(YGPositionTypeAbsolute);
        node.layout().setDisplay(YGDisplayNone);
    }
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
    Output(const void *params) noexcept : LOutput(params)
    {
        enableFractionalOversampling(false);
        background.layout().setFlexWrap(YGWrapWrap);
        background.layout().setPadding(YGEdgeAll, 40.f);
        background.layout().setGap(YGGutterAll, 40.f);
        background.layout().setFlexDirection(YGFlexDirectionRow);
        background.layout().setPositionType(YGPositionTypeAbsolute);
    }

    Compositor *comp() const noexcept { return static_cast<Compositor*>(compositor()); }

    void updateBackground() noexcept
    {
        background.layout().setPosition(YGEdgeLeft, pos().x());
        background.layout().setPosition(YGEdgeTop, pos().y());
        background.layout().setWidth(size().w());
        background.layout().setHeight(size().h());
    }

    void initializeGL() override
    {
        frame = 0;
        static auto interface = GrGLMakeAssembledInterface(nullptr, (GrGLGetProc)*[](void *, const char *p) -> void * {
            return (void *)eglGetProcAddress(p);
        });

        contextOptions.fShaderCacheStrategy = GrContextOptions::ShaderCacheStrategy::kBackendBinary;
        contextOptions.fAvoidStencilBuffers = true;
        contextOptions.fPreferExternalImagesOverES3 = true;
        contextOptions.fDisableGpuYUVConversion = true;
        contextOptions.fReducedShaderVariations = false;
        contextOptions.fSuppressPrints = true;
        contextOptions.fSuppressMipmapSupport = true;
        contextOptions.fSkipGLErrorChecks = GrContextOptions::Enable::kYes;
        contextOptions.fBufferMapThreshold = -1;
        contextOptions.fDisableDistanceFieldPaths = true;
        contextOptions.fAllowPathMaskCaching = false;
        contextOptions.fGlyphCacheTextureMaximumBytes = 2048 * 1024 * 4;
        contextOptions.fUseDrawInsteadOfClear = GrContextOptions::Enable::kYes;
        contextOptions.fReduceOpsTaskSplitting = GrContextOptions::Enable::kYes;
        contextOptions.fDisableDriverCorrectnessWorkarounds = true;
        contextOptions.fRuntimeProgramCacheSize = 256;
        contextOptions.fInternalMultisampleCount = 4;
        contextOptions.fDisableTessellationPathRenderer = false;
        contextOptions.fAllowMSAAOnNewIntel = true;
        contextOptions.fAlwaysUseTexStorageWhenAvailable = false;

        context = GrDirectContext::MakeGL(interface, contextOptions);

        if (!context.get())
        {
            LLog::fatal("Failed to create Skia context.");
            exit(1);
        }

        target = comp()->scene.createTarget();
        target->root = &comp()->root;
        updateBackground();

        for (int i = 0; i < 20; i++)
        {
            buttons.push_back(new Button(&background));
        }
    }

    void paintGL() override
    {
        phase += 0.1f;

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

        /*
        for (auto *btn : buttons)
        {
            btn->shadowEffect.setShadowRadius(50.f + SkScalarCos(phase) * 50.f);
            btn->shadowEffect.setShadowOffset(SkScalarCos(phase) * 20.f, SkScalarSin(phase) * 20.f);
        }*/

        for (Surface *s : (const std::list<Surface*>&)(compositor()->surfaces()))
        {
            s->node.setVisible(!s->cursorRole() && !s->minimized() && s->mapped());

            if (!s->node.visible())
                continue;

            const LPoint &pos { s->rolePos() };

            s->node.layout().setPosition(YGEdgeLeft, pos.x());
            s->node.layout().setPosition(YGEdgeTop, pos.y());
            s->node.layout().setWidth(s->size().w());
            s->node.layout().setHeight(s->size().h());

            s->node.setImage(louvreTex2SkiaImage(s->texture(), context.get(), this));
            s->node.setSrcRect(SkRect::MakeXYWH(
                s->srcRect().x(), s->srcRect().y(),
                s->srcRect().w(), s->srcRect().h()));
            s->node.setScale(s->bufferScale());
            s->node.setTransform(static_cast<AKTransform>(s->bufferTransform()));

            boxes = s->damage().boxes(&n);
            region.setRects((const SkIRect*)boxes, n);
            s->node.addDamage(region);

            boxes = s->opaqueRegion().boxes(&n);
            s->node.opaqueRegion.setRects((const SkIRect*)boxes, n);
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

        target->outDamageRegion = &outDamage;
        target->age = age;
        target->scale = 1.f;
        target->viewport = SkRect::MakeXYWH(pos().x(), pos().y(), size().w(), size().h());
        target->transform = static_cast<AKTransform>(transform());
        target->dstRect = SkIRect::MakeXYWH(0, 0, currentMode()->sizeB().w(), currentMode()->sizeB().h());

        //glScissor(0, 0, 10000000, 1000000);
        //glClear(GL_COLOR_BUFFER_BIT);
        static_cast<Compositor*>(compositor())->scene.render(target);

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
        updateBackground();
    }

    void moveGL() override
    {
        frame = 0;
        repaint();
        updateBackground();
    }

    void uninitializeGL() override
    {
        static_cast<Compositor*>(compositor())->scene.destroyTarget(target);
    }

    AKContainer background { YGFlexDirectionRow, true, &comp()->background };
    GrContextOptions contextOptions;
    sk_sp<GrDirectContext> context;
    AKTarget *target { nullptr };
    UInt32 age { 0 };
    UInt32 frame { 0 };
    float phase { 0.f };
    std::vector<Button*> buttons;
};

class Pointer final : public LPointer
{
public:
    using LPointer::LPointer;

    void pointerMoveEvent(const LPointerMoveEvent &event) override
    {
        LPointer::pointerMoveEvent(event);
        cursor()->repaintOutputs(false);
    }

    void pointerButtonEvent(const LPointerButtonEvent &event) override
    {
        LPointer::pointerButtonEvent(event);

        if (event.button() == BTN_LEFT && event.state() == LPointerButtonEvent::Released)
        {
            if (seat()->keyboard()->isKeyCodePressed(KEY_S))
            {
                for (LOutput *o : compositor()->outputs())
                    o->setScale(o->scale() == 1 ? 2.f : 1.f);
            }
        }
    }
};

LFactoryObject *Compositor::createObjectRequest(LFactoryObject::Type objectType, const void *params)
{
    if (objectType == LFactoryObject::Type::LOutput)
        return new Output(params);

    if (objectType == LFactoryObject::Type::LSurface)
        return new Surface(params);

    if (objectType == LFactoryObject::Type::LPointer)
        return new Pointer(params);

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
