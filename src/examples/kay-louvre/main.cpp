#include "AK/nodes/AKPath.h"
#include "AK/nodes/AKSimpleText.h"
#include "include/core/SkMaskFilter.h"
#include <include/effects/SkBlurMaskFilter.h>

#include <LLauncher.h>
#include <LCompositor.h>
#include <LOutput.h>
#include <LOutputMode.h>
#include <LSurface.h>
#include <LCursor.h>
#include <LPointer.h>
#include <LLog.h>
#include <LSeat.h>
#include <include/core/SkPoint.h>
#include <LKeyboard.h>
#include <LAnimation.h>
#include <LOpenGL.h>
#include <LScreenshotRequest.h>
#include <LExclusiveZone.h>
#include <private/LOutputPrivate.h>

#include <AK/nodes/AKSubScene.h>
#include <AK/nodes/AKImage.h>
#include <AK/nodes/AKContainer.h>
#include <AK/nodes/AKSolidColor.h>
#include <AK/nodes/AKRoundContainer.h>
#include <AK/nodes/AKButton.h>
#include <AK/effects/AKBackgroundBoxShadowEffect.h>
#include <AK/effects/AKBackgroundImageShadowEffect.h>
#include <AK/effects/AKBackgroundBlurEffect.h>

#include <AK/AKScene.h>
#include <AK/AKSurface.h>

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
#include <include/effects/SkBlurMaskFilter.h>
#include <include/core/SkRRect.h>
#include <include/utils/SkParsePath.h>
#include <iostream>

using namespace AK;
using namespace AK;

enum NodeTypeFlags
{
    MENU_ITEM = 1 << 1
};

static sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeSRGB();
static SkSurfaceProps skSurfaceProps(0, kUnknown_SkPixelGeometry);
static const std::string logoSVG { "M9.05.435c-.58-.58-1.52-.58-2.1 0L.436 6.95c-.58.58-.58 1.519 0 2.098l6.516 6.516c.58.58 1.519.58 2.098 0l6.516-6.516c.58-.58.58-1.519 0-2.098zM8 .989c.127 0 .253.049.35.145l6.516 6.516a.495.495 0 0 1 0 .7L8.35 14.866a.5.5 0 0 1-.35.145z" };

// Creates an SkImage from an LTexture (its just a wrapper, there is no copy involved)
// Todo: properly handle other formats
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

/**
 * @brief Example Context Menu Component
 *
 * The menu only renders the container and shadow (see onBake()).
 * Menu items are composited by the scene and clipped by itemsContainer.
 */
class Menu : public AKBakeable
{
public:

    // Each node has its own changes flags
    // They're used to prevent re-doing stuff as demostrated below
    // By default all flags are true, and are cleared after
    // a scene render pass
    enum Changes
    {
        Chg_ShadowRadius = AKBakeable::Chg_Last,
        Chg_ShadowOffset,
        Chg_ShadowColor,
        Chg_BorderRadius,
        Chg_Brush,
        Chg_Pen,
        Chg_Last
    };

    Menu(AKNode *parent = nullptr) noexcept : AKBakeable(parent)
    {
        blur.on.targetLayoutUpdated.subscribe(this, [this](){
            const auto &chgs { changes() };
            bool blurNeedsUpdate { chgs.test(Chg_Size) };

            for (UInt32 flag = Chg_ShadowRadius; flag <= Chg_BorderRadius; flag++)
                blurNeedsUpdate |= chgs.test(flag);

            if (!blurNeedsUpdate)
                return;

            blur.effectRect.setXYWH(
                itemsContainer.layout().calculatedLeft(),
                itemsContainer.layout().calculatedTop(),
                itemsContainer.layout().calculatedWidth(),
                itemsContainer.layout().calculatedHeight());
            blur.clip.reset();
            blur.clip.addRRect(SkRRect::MakeRectXY(
                SkRect::MakeWH(blur.effectRect.size().width(), blur.effectRect.size().height()),
                m_borderRadius,
                m_borderRadius));
        });

        enableChildrenClipping(true);
        layout().setDisplay(YGDisplayNone);

        // Default styles
        m_brush.setAntiAlias(true);
        m_pen.setAntiAlias(true);
        m_brush.setColor(0x99FAFAFA); // ARGB
        m_pen.setStrokeWidth(0.25f);
        m_pen.setColor(0xAA000000);
        m_shadowBrush.setColor(0x66000000);

        layout().setPositionType(YGPositionTypeAbsolute);
        layout().setPosition(YGEdgeLeft, 100);
        layout().setPosition(YGEdgeTop, 100);

        // Fill menu
        itemsContainer.layout().setFlex(1.f);
        itemsContainer.layout().setPadding(YGEdgeAll, m_borderRadius);
    }

    void showAt(Int32 x, Int32 y) noexcept
    {
        layout().setPosition(YGEdgeLeft, x - m_shadowRadius + m_shadowOffset.x());
        layout().setPosition(YGEdgeTop, y - m_shadowRadius + m_shadowOffset.y());
        layout().setDisplay(YGDisplayFlex);
    }

    void hide() noexcept
    {
        layout().setDisplay(YGDisplayNone);
    }

    void setShadowColor(SkColor color) noexcept
    {
        if (m_shadowBrush.getColor() == color)
            return;

        m_shadowBrush.setColor(color);
        addChange(Chg_ShadowColor);
    }

    void setShadowRadius(SkScalar radius) noexcept
    {
        if (m_shadowRadius == radius)
            return;

        m_shadowRadius = radius;
        addChange(Chg_ShadowRadius);
    }

    void setShadowOffset(const SkPoint &offset) noexcept
    {
        if (m_shadowOffset == offset)
            return;

        m_shadowOffset = offset;
        addChange(Chg_ShadowOffset);
    }

    void setBrush(const AKBrush &brush) noexcept
    {
        if (m_brush == brush)
            return;

        m_brush = brush;
        addChange(Chg_Brush);
    }

    void setPen(const AKPen &pen) noexcept
    {
        if (m_pen == pen)
            return;

        m_pen = pen;
        addChange(Chg_Pen);
    }

    AKContainer itemsContainer { YGFlexDirectionColumn, true, this };
    AKBackgroundBlurEffect blur { AKBackgroundBlurEffect::Manual, { 16.f, 16.f },this };
protected:
    AKBrush m_shadowBrush; // Shadow style (color basically)
    AKBrush m_brush; // Fill style
    AKPen m_pen; // Stroke style
    SkScalar m_borderRadius = 8.f;
    SkScalar m_shadowRadius = 24.f;
    SkPoint m_shadowOffset { 0.f, 8.f };
    LAnimation m_fadeInAnimation, m_fadeOutAnimation;

    // Called right before the scene calculates the layout
    void onSceneBegin() override
    {
        const auto chgs { changes() };
        const bool needsShadowUpdate { chgs.test(Chg_ShadowRadius) };
        const bool needsPaddingUpdate { needsShadowUpdate || chgs.test(Chg_ShadowOffset) };

        // Add padding to prevent itemsContainer from overlapping the shadow
        if (needsPaddingUpdate)
        {
            layout().setPadding(YGEdgeLeft, m_shadowRadius - m_shadowOffset.x());
            layout().setPadding(YGEdgeTop, m_shadowRadius - m_shadowOffset.y());
            layout().setPadding(YGEdgeRight, m_shadowRadius + m_shadowOffset.x());
            layout().setPadding(YGEdgeBottom, m_shadowRadius + m_shadowOffset.x());
        }

        if (needsShadowUpdate)
            m_shadowBrush.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, m_shadowRadius/3.f));
    }

    // Always called by the scene except if the node is invisible or completly ocludded
    // Here the component renders into its own framebuffer.
    // Its the component's responsibility to avoid re-baking itself each time
    void onBake(OnBakeParams *params) override
    {
        const auto chgs { changes() };

        const bool needsRebake {
            !params->damage->isEmpty() || // If the scene explicitly adds damage it means the node's size changed
            chgs.test(Chg_ShadowRadius) ||
            chgs.test(Chg_ShadowColor) ||
            chgs.test(Chg_ShadowOffset) ||
            chgs.test(Chg_BorderRadius) ||
            chgs.test(Chg_Brush) ||
            chgs.test(Chg_Pen)
        };

        if (!needsRebake) // Nothing to do
            return;

        SkCanvas &c { *params->surface->surface()->getCanvas() };

        // To improve performance the framebuffer isn't automatically shrank
        // when the node is resized, but since menus aren't constatly resizing
        // calling shrink can free up some GPU memory
        params->surface->shrink();

        // Ensure previus content or artifacts of newly created framebuffers are cleared
        c.clear(SK_ColorTRANSPARENT);

        // Calculated layout params are relative to the parent node
        const SkRect rect {SkRect::MakeXYWH(
            itemsContainer.layout().calculatedLeft(),
            itemsContainer.layout().calculatedTop(),
            itemsContainer.layout().calculatedWidth(),
            itemsContainer.layout().calculatedHeight())};

        const SkRect shadowRect { rect.makeOffset(m_shadowOffset) };

        // Draw shadow
        c.drawRoundRect(shadowRect, m_borderRadius, m_borderRadius, m_shadowBrush);

        // Draw container fill
        m_brush.setBlendMode(SkBlendMode::kSrc);
        c.drawRoundRect(rect, m_borderRadius, m_borderRadius, m_brush);

        // Draw container stroke
        c.drawRoundRect(rect, m_borderRadius, m_borderRadius, m_pen);

        // Mark node as dirty
        params->damage->setRect(AK_IRECT_INF);

        // Set the opaque region (to optimize composition)
        if (SkColorGetA(m_brush.getColor()) == 255) // Opaque
        {
            // Make entire rect opaque
            params->opaque->setRect(rect.roundIn());

            // But exclude round corners

            // Top Left
            SkIRect borderRadiusRect = SkRect::MakeXYWH(rect.x(), rect.y(), m_borderRadius, m_borderRadius).roundOut();
            params->opaque->op(borderRadiusRect, SkRegion::Op::kDifference_Op);

            // Top Right
            borderRadiusRect.offset(rect.width() - borderRadiusRect.width(), 0);
            params->opaque->op(borderRadiusRect, SkRegion::Op::kDifference_Op);

            // Bottom Right
            borderRadiusRect.offset(0, rect.height() - borderRadiusRect.height());
            params->opaque->op(borderRadiusRect, SkRegion::Op::kDifference_Op);

            // Bottom Left
            borderRadiusRect.offsetTo(rect.x(), borderRadiusRect.y());
            params->opaque->op(borderRadiusRect, SkRegion::Op::kDifference_Op);
        }
        // Make fully translucent
        else
            params->opaque->setEmpty();
    }

    // Keep the default onRender() implementation
    // That's where the scene blits the component onto the screen
};

class MenuItem : public AKRoundContainer
{
public:
    MenuItem( const std::string &text, AKNode *parent = nullptr) noexcept :
        AKRoundContainer( { 4, 4, 4, 4 }, parent),
        m_text(text, &m_backgroundColor)
    {
        userFlags |= MENU_ITEM;
        layout().setDisplay(YGDisplayFlex);
        m_backgroundColor.setOpacity(0.f);
        m_backgroundColor.layout().setPadding(YGEdgeAll, 6.f);
        SkFont font;
        font.setEmbolden(true);
        m_text.setFont(font);
    }

    void onPointerEnter()
    {
        m_backgroundColor.setOpacity(1.f);
        m_text.setColorWithAlpha(SkColors::kWhite);
    }

    void onPointerLeave()
    {
        m_backgroundColor.setOpacity(0.f);
        m_text.setColorWithAlpha(SkColors::kBlack);
    }

    void onPress()
    {
        setOpacity(0.75f);
    }

    void onRelease()
    {
        setOpacity(1.f);
    }

protected:
    AKSolidColor m_backgroundColor { 0xFF2196F3, this };
    AKSimpleText m_text;
};

class Compositor final : public LCompositor
{
public:
    Compositor() noexcept
    {
        scene.setRoot(&root);
        surfaces.layout().setPositionType(YGPositionTypeAbsolute);
    }

    void initialized() override
    {
        LCompositor::initialized();
    }

    LFactoryObject *createObjectRequest(LFactoryObject::Type objectType, const void *params) override;
    AKScene scene;
    AKContainer root;
    AKContainer background { YGFlexDirectionColumn, false, &root };
    AKContainer surfaces { YGFlexDirectionColumn, false, &root };
    AKContainer overlay { YGFlexDirectionColumn, false, &root };
    Menu menu { &overlay };
    MenuItem item1 { "New Folder", &menu.itemsContainer };
    MenuItem item2 { "Open Terminal", &menu.itemsContainer };
    MenuItem item3 { "Change Wallpaper", &menu.itemsContainer };
    MenuItem item4 { "Properties", &menu.itemsContainer };
    MenuItem item5 { "Empty Trash", &menu.itemsContainer };
    MenuItem item6 { "Menu with very very very very very very very very very long text", &menu.itemsContainer };
    MenuItem item7 { "Extra menu item 1", &menu.itemsContainer };
    MenuItem item8 { "Extra menu item 2", &menu.itemsContainer };
    MenuItem item9 { "Extra menu item 3", &menu.itemsContainer };
    MenuItem item10 { "Extra menu item 4", &menu.itemsContainer };
    MenuItem item11 { "Extra menu item 5", &menu.itemsContainer };
    MenuItem item12 { "Extra menu item 6", &menu.itemsContainer };
    MenuItem item13 { "Extra menu item 7", &menu.itemsContainer };
    MenuItem item14 { "Extra menu item 8", &menu.itemsContainer };
    MenuItem item15 { "Extra menu item 9", &menu.itemsContainer };
    MenuItem item16 { "Extra menu item 10", &menu.itemsContainer };
    MenuItem item17 { "Extra menu item 11", &menu.itemsContainer };
    MenuItem item18 { "Extra menu item 12", &menu.itemsContainer };
};

class Surface final : public LSurface
{
public:
    Surface(const void *data) noexcept : LSurface(data) {
        node.layout().setPositionType(YGPositionTypeAbsolute);
    }
    Compositor *comp() const noexcept { return static_cast<Compositor*>(compositor()); }

    AKImage node { &comp()->surfaces };

    void orderChanged() override
    {
        AKNode *prev { prevSurface() ? &static_cast<Surface*>(prevSurface())->node : nullptr };
        node.insertAfter(prev);
    }
};

class Text final : public AKSimpleText
{
public:
    using AKSimpleText::AKSimpleText;
    AKBackgroundImageShadowEffect shadow { 6, {0,0}, 0x66000000, this };
};

class Output final : public LOutput
{
public:
    Output(const void *params) noexcept : LOutput(params)
    {
        background.layout().setDisplay(YGDisplayFlex);
        background.layout().setJustifyContent(YGJustifyCenter);
        background.layout().setAlignItems(YGAlignCenter);
        background.layout().setPositionType(YGPositionTypeAbsolute);
        background.layout().setGap(YGGutterAll, 8.f);

        topbar.layout().setPositionType(YGPositionTypeAbsolute);
        topbar.layout().setPosition(YGEdgeLeft, pos().x());
        topbar.layout().setPosition(YGEdgeTop, pos().x());
        topbarBackground.userFlags = 5;

        topbarBackground.layout().setGap(YGGutterAll, 16);
        topbarBackground.layout().setWidthPercent(100);
        topbarBackground.layout().setHeightPercent(100);
        topbarBackground.layout().setAlignItems(YGAlignCenter);
        topbarBackground.layout().setJustifyContent(YGJustifyFlexStart);
        topbarBackground.layout().setPadding(YGEdgeHorizontal, 8.f);
        topbarBackground.layout().setFlexDirection(YGFlexDirectionRow);

        SkPath path;
        SkParsePath::FromSVGString(logoSVG.c_str(), &path);
        logo.setPath(path);
        logo.setColorWithAlpha(SK_ColorWHITE);
        logo.setSizeMode(AKPath::ScalePath);
        logo.layout().setWidth(16);
        logo.layout().setHeight(16);
        logo.layout().setMargin(YGEdgeLeft, 8.f);
        logo.layout().setMargin(YGEdgeRight, 4.f);

        const std::vector<std::string> topbarMenuNames
        {
            "File",
            "Edit",
            "View",
            "Build",
            "Debug",
            "Analyze",
            "Tools",
            "Window",
            "Help"
        };

        SkFont font;
        font.setEmbolden(true);
        for (auto &name : topbarMenuNames)
        {
            topbarMenus.push_back(new Text(name, &topbarBackground));
            topbarMenus.back()->setColorWithAlpha(SK_ColorWHITE);
            topbarMenus.back()->setFont(font);
            topbarMenus.back()->setOpacity(0.75f);
        }

        topbarExclusiveZone.setOnRectChangeCallback([this](LExclusiveZone *zone){
            topbar.setParent(&comp()->overlay);
            topbar.layout().setPosition(YGEdgeLeft, pos().x() + zone->rect().x());
            topbar.layout().setPosition(YGEdgeTop, pos().y() + zone->rect().y());
            topbar.layout().setWidth(zone->rect().w());
            topbar.layout().setHeight(zone->rect().h());

            std::cout << "Topbar: Exclusive zone changed " << zone->rect().x() << "," << zone->rect().y() << "," << zone->rect().w() << "," << zone->rect().h() << std::endl;
        });
    }

    Compositor *comp() const noexcept { return static_cast<Compositor*>(compositor()); }

    void updateBackground() noexcept
    {
        if (compositor()->graphicBackendId() == LGraphicBackendDRM)
        {
            LSize bufferSize;

            if (is90Transform(transform()))
            {
                bufferSize.setW(currentMode()->sizeB().h());
                bufferSize.setH(currentMode()->sizeB().w());
            }
            else
                bufferSize = currentMode()->sizeB();

            if (wallpaper && wallpaper->sizeB() == bufferSize)
                return;

            if (wallpaper)
                delete wallpaper.get();

            wallpaper.reset(LOpenGL::loadTexture(compositor()->defaultAssetsPath() / "wallpaper.png"));

            if (!wallpaper)
                goto skipScale;

            LRect srcB;
            const Float32 w { Float32(bufferSize.w() * wallpaper->sizeB().h()) / Float32(bufferSize.h()) };

            /* Clip and scale the wallpaper texture */

            if (w >= wallpaper->sizeB().w())
            {
                srcB.setX(0);
                srcB.setW(wallpaper->sizeB().w());
                srcB.setH((wallpaper->sizeB().w() * bufferSize.h()) / bufferSize.w());
                srcB.setY((wallpaper->sizeB().h() - srcB.h()) / 2);
            }
            else
            {
                srcB.setY(0);
                srcB.setH(wallpaper->sizeB().h());
                srcB.setW((wallpaper->sizeB().h() * bufferSize.w()) / bufferSize.h());
                srcB.setX((wallpaper->sizeB().w() - srcB.w()) / 2);
            }
            LTexture *scaledWallpaper = wallpaper->copy(bufferSize, srcB);
            delete wallpaper.get();
            wallpaper.reset(scaledWallpaper);
        }
        else if (!wallpaper)
        {
            wallpaper.reset(LOpenGL::loadTexture(compositor()->defaultAssetsPath() / "wallpaper.png"));
        }

    skipScale:
        background.setImage(louvreTex2SkiaImage(wallpaper.get(), context.get(), this));
        background.opaqueRegion.setRect(AK_IRECT_INF);
        background.layout().setPosition(YGEdgeLeft, pos().x());
        background.layout().setPosition(YGEdgeTop, pos().y());
        background.layout().setWidth(size().w());
        background.layout().setHeight(size().h());

        if (wallpaper)
            background.setSrcRect(SkRect::MakeWH(wallpaper->sizeB().w(), wallpaper->sizeB().h()));
    }

    void initializeGL() override
    {
        disabledButton.setEnabled(false);
        customBackgroundButton.setBackgroundColor(0xFF247aff);
        customBackgroundButtonDisabled.setBackgroundColor(0xFF247aff);
        customBackgroundButtonDisabled.setEnabled(false);

        //setScale(1.5f);
        // Louvre creates an OpenGL context for each output
        // here we are wrapping it into a GrDirectContext.

        static auto interface = GrGLMakeAssembledInterface(nullptr, (GrGLGetProc)*[](void *, const char *p) -> void * {
            return (void *)eglGetProcAddress(p);
        });

        contextOptions.fShaderCacheStrategy = GrContextOptions::ShaderCacheStrategy::kBackendBinary;
        contextOptions.fAvoidStencilBuffers = true;
        contextOptions.fPreferExternalImagesOverES3 = true;
        contextOptions.fDisableGpuYUVConversion = true;
        contextOptions.fReducedShaderVariations = true;
        contextOptions.fSuppressPrints = true;
        contextOptions.fSuppressMipmapSupport = true;
        contextOptions.fSkipGLErrorChecks = GrContextOptions::Enable::kYes;
        contextOptions.fBufferMapThreshold = -1;
        contextOptions.fDisableDistanceFieldPaths = true;
        contextOptions.fAllowPathMaskCaching = true;
        contextOptions.fGlyphCacheTextureMaximumBytes = 2048 * 1024 * 4;
        contextOptions.fUseDrawInsteadOfClear = GrContextOptions::Enable::kYes;
        contextOptions.fReduceOpsTaskSplitting = GrContextOptions::Enable::kYes;
        contextOptions.fDisableDriverCorrectnessWorkarounds = true;
        contextOptions.fRuntimeProgramCacheSize = 1024;
        contextOptions.fInternalMultisampleCount = 0;
        contextOptions.fDisableTessellationPathRenderer = false;
        contextOptions.fAllowMSAAOnNewIntel = true;
        contextOptions.fAlwaysUseTexStorageWhenAvailable = true;

        context = GrDirectContext::MakeGL(interface, contextOptions);

        if (!context.get())
        {
            LLog::fatal("Failed to create Skia context.");
            exit(1);
        }

        // All outputs share the same scene but we still need to create a
        // specific target for each.
        // A target contains information about the viewport, destination
        // framebuffer, transform, etc, and is used by the scene and
        // nodes to keep track of damage, previous dimensions, and other properties.
        target = comp()->scene.createTarget();
        target->setClearColor(SK_ColorWHITE);
        target->on.markedDirty.subscribe(target, [this](AKTarget &){

            if (inPaintGL)
                return;
            repaint();
        });

        updateBackground();
        topbarExclusiveZone.setOutput(this);
    }

    void paintGL() override
    {
        inPaintGL = true;
        Int32 n;
        const LBox *boxes;
        SkRegion region, outDamage;
        LRegion damage;
        SkRegion cursorDamage;

        // Create an SkSurface for the current screen framebuffer
        // Depending on the backend this can change each frame, but luckly
        // since its just a wrapper re-creating it has no performance cost
        const GrGLFramebufferInfo fbInfo
        {
            .fFBOID = framebuffer()->id(),
            .fFormat = GL_RGB8_OES
        };

        const GrBackendRenderTarget backendTarget(
            realBufferSize().w(),
            realBufferSize().h(),
            //currentMode()->sizeB().w(),
            //currentMode()->sizeB().h(),
            0, 0,
            fbInfo);

        target->setSurface(SkSurfaces::WrapBackendRenderTarget(
            context.get(),
            backendTarget,
            fbInfo.fFBOID == 0 ? GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin : GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
            SkColorType::kRGB_888x_SkColorType,
            colorSpace,
            &skSurfaceProps));

        target->setImage(louvreTex2SkiaImage(
            usingFractionalScale() && fractionalOversamplingEnabled() ?
            imp()->fractionalFb.texture(0) : bufferTexture(currentBuffer()), context.get(), this));

        // We need to manually update each surface node
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

        // We can ask the scene which region was repainted
        if (hasBufferDamageSupport() || usingFractionalScale())
            target->outDamageRegion = &outDamage;

        // If hw cursor is disabled or during screen captures
        const LRegion &softwareCursorDamage = cursor()->damage(this);
        boxes = softwareCursorDamage.boxes(&n);
        cursorDamage.setRects((SkIRect*)boxes, n);
        target->inDamageRegion = &cursorDamage;

        // Required for damage tracking
        target->setAge(currentBufferAge());

        // Rect of the scene to capture relative to the root node
        target->setViewport(SkRect::MakeXYWH(pos().x(), pos().y(), size().w(), size().h()));

        // If the screen is rotated/flipped
        target->setTransform(static_cast<AKTransform>(transform()));

        // Rect within the screen fb to render the viewport to (in this case the entire screen)
        target->setDstRect(SkIRect::MakeXYWH(0, 0, backendTarget.width(), backendTarget.height()));
        target->setBakedComponentsScale(scale());

        //glScissor(0,0,100000,100000);
        //glClear(GL_COLOR_BUFFER_BIT);

        // Here the scene calculates the layout and performs all the rendering
        comp()->scene.render(target);

        // Allow visible apps to update
        for (Surface *s : (const std::list<Surface*>&)(compositor()->surfaces()))
        {
            // If not, it means the surface was ocluded
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

        // This allows optimizations in the Wayland backend, or the DRM backend in some hybrid GPU cases
        if (hasBufferDamageSupport() || usingFractionalScale())
        {
            outDamage.translate(pos().x(), pos().y());

            SkRegion::Iterator it(outDamage);
            while (!it.done())
            {
                damage.addRect(it.rect().x(), it.rect().y(), it.rect().width(), it.rect().height());
                it.next();
            }

            setBufferDamage(&damage);
        }

        for (auto *shReq : screenshotRequests())
            shReq->accept(true);

        inPaintGL = false;
    }

    void resizeGL() override
    {
        updateBackground();
    }

    void moveGL() override
    {
        updateBackground();
    }

    void uninitializeGL() override
    {
        comp()->scene.destroyTarget(target);

        if (wallpaper)
            delete wallpaper.get();
    }

    bool inPaintGL { false };
    AKImage background { &comp()->background };
    AKSimpleText instructions { "F1: Launch Weston Terminal - Right Click: Show Context Menu.", &background};
    AKSimpleText instructions2 { "Note: Blur only works if launched from a TTY (DRM backend)", &background};
    AKButton normalButton { "Normal Button", &background };
    AKButton disabledButton { "Disabled Button", &background };
    AKButton customBackgroundButton { "Colored Button", &background };
    AKButton customBackgroundButtonDisabled { "Colored Button Disabled", &background };
    GrContextOptions contextOptions;
    sk_sp<GrDirectContext> context;
    AKTarget *target { nullptr };
    LWeak<LTexture> wallpaper;

    AKSubScene topbar { };
    AKBackgroundBlurEffect topbarBlur { AKBackgroundBlurEffect::Automatic, {200.f, 200.f}, &topbar};
    AKSolidColor topbarBackground { 0x22FFFFFF, &topbar };
    AKBackgroundBoxShadowEffect topbarShadow {
        16, {0, 0}, 0x45000000, false, &topbar};
    AKPath logo { &topbarBackground };
    AKBackgroundImageShadowEffect logoShadow { 6, {0,0}, 0x66000000, &logo };
    std::vector<Text*>topbarMenus;
    LExclusiveZone topbarExclusiveZone { LEdgeTop, 28 };
};

class Pointer final : public LPointer
{
public:
    using LPointer::LPointer;

    AKWeak<MenuItem> focus;

    AKNode *nodeAt(const SkIPoint &globalPos, UInt64 filter) const noexcept
    {
        return findNode(&static_cast<Compositor*>(compositor())->root, globalPos, filter);
    }

    void pointerMoveEvent(const LPointerMoveEvent &event) override
    {
        LPointer::pointerMoveEvent(event);

        MenuItem *newFocus { (MenuItem*)nodeAt(SkIPoint(cursor()->pos().x(), cursor()->pos().y()), MENU_ITEM) };

        if (newFocus)
        {
            if (focus)
            {
                if (focus != newFocus)
                {
                    focus->onRelease();
                    focus->onPointerLeave();
                    focus = newFocus;
                    focus->onPointerEnter();
                }
            }
            else
            {
                focus = newFocus;
                focus->onPointerEnter();
            }
        }
        else
        {
            if (focus)
            {
                focus->onRelease();
                focus->onPointerLeave();
                focus.reset();
            }
        }
    }

    void pointerButtonEvent(const LPointerButtonEvent &event) override
    {
        LPointer::pointerButtonEvent(event);

        if (event.button() == BTN_RIGHT && event.state() == LPointerButtonEvent::Pressed)
        {
            static_cast<Compositor*>(compositor())->menu.showAt(cursor()->pos().x(), cursor()->pos().y());
            return;
        }

        if (event.button() == BTN_LEFT)
        {
            if (!focus)
            {
                if (event.state() == LPointerButtonEvent::Released)
                    static_cast<Compositor*>(compositor())->menu.hide();
                return;
            }

            if (event.state() == LPointerButtonEvent::Pressed)
                focus->onPress();
            else
                focus->onRelease();
        }
    }
private:
    static AKNode *findNode(AKNode *node, const SkIPoint &globalPos, UInt64 filter) noexcept
    {
        for (auto it = node->children().rbegin(); it != node->children().rend(); it++)
            if (AKNode *child = findNode(*it, globalPos, filter))
                return child;

        if ((node->userFlags & filter) && node->globalRect().contains(globalPos.x(), globalPos.y()))
            return node;

        return nullptr;
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
    setenv("SRM_RENDER_MODE_ITSELF_FB_COUNT", "3", 1);

    LLauncher::startDaemon();
    Compositor compositor;
    assert("Compositor failed to start" && compositor.start());

    while (compositor.state() != LCompositor::Uninitialized)
        compositor.processLoop(-1);

    return 0;
}
