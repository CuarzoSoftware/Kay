#include <include/gpu/ganesh/gl/GrGLBackendSurface.h>
#include <include/core/SkMaskFilter.h>
#include <include/effects/SkBlurMaskFilter.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/gpu/ganesh/SkImageGanesh.h>
#include <include/core/SkImage.h>
#include <include/core/SkSurface.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkColorSpace.h>
#include <include/effects/SkImageFilters.h>
#include <include/effects/SkGradientShader.h>
#include <include/utils/SkParsePath.h>
#include <include/effects/SkBlurMaskFilter.h>
#include <include/core/SkRRect.h>
#include <include/utils/SkParsePath.h>
#include <include/gpu/ganesh/GrDirectContext.h>

#include <LPainter.h>
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
#include <LKeyboardKeyEvent.h>

#include <AK/nodes/AKSubScene.h>
#include <AK/nodes/AKImageFrame.h>
#include <AK/nodes/AKContainer.h>
#include <AK/nodes/AKSolidColor.h>
#include <AK/nodes/AKRoundContainer.h>
#include <AK/nodes/AKButton.h>
#include <AK/nodes/AKPath.h>
#include <AK/nodes/AKTextField.h>

#include <AK/effects/AKBackgroundBoxShadowEffect.h>
#include <AK/effects/AKBackgroundImageShadowEffect.h>
#include <AK/effects/AKBackgroundBlurEffect.h>

#include <AK/events/AKPointerMoveEvent.h>
#include <AK/events/AKPointerButtonEvent.h>
#include <AK/events/AKKeyboardKeyEvent.h>
#include <AK/events/AKBakeEvent.h>
#include <AK/input/AKKeyboard.h>

#include <AK/AKApplication.h>
#include <AK/AKScene.h>
#include <AK/AKSurface.h>
#include <AK/AKTheme.h>
#include <AK/AKGLContext.h>
#include <AK/AKTimer.h>
#include <AK/AKLog.h>

#include <cassert>

using namespace Louvre;
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
static sk_sp<SkImage> louvreTex2SkiaImage(LTexture *texture, LOutput *o)
{
    if (!texture)
        return nullptr;

    GrGLTextureInfo skTextureInfo;
    GrBackendTexture skTexture;
    skTextureInfo.fFormat = GL_BGRA8_EXT;
    skTextureInfo.fID = texture->id(o);
    skTextureInfo.fTarget = texture->target();

    skTexture = GrBackendTextures::MakeGL(
        texture->sizeB().w(),
        texture->sizeB().h(),
        skgpu::Mipmapped::kNo,
        skTextureInfo);

    return SkImages::BorrowTextureFrom(
        akApp()->glContext()->skContext().get(),
        skTexture,
        GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
        kBGRA_8888_SkColorType,
        texture->premultipliedAlpha() ? SkAlphaType::kPremul_SkAlphaType : SkAlphaType::kUnpremul_SkAlphaType,
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
        CHShadowRadius = AKBakeable::CHLast,
        CHShadowOffset,
        CHShadowColor,
        CHBorderRadius,
        CHBrush,
        CHPen,
        CHLast
    };

    Menu(AKNode *parent = nullptr) noexcept : AKBakeable(parent)
    {
        blur.on.targetLayoutUpdated.subscribe(this, [this](){
            const auto &chgs { changes() };
            bool blurNeedsUpdate { chgs.test(CHSize) };

            for (UInt32 flag = CHShadowRadius; flag <= CHBorderRadius; flag++)
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
        setVisible(true);
    }

    void hide() noexcept
    {
        setVisible(false);
    }

    void setShadowColor(SkColor color) noexcept
    {
        if (m_shadowBrush.getColor() == color)
            return;

        m_shadowBrush.setColor(color);
        addChange(CHShadowColor);
    }

    void setShadowRadius(SkScalar radius) noexcept
    {
        if (m_shadowRadius == radius)
            return;

        m_shadowRadius = radius;
        addChange(CHShadowRadius);
        updateShadow();
    }

    void setShadowOffset(const SkPoint &offset) noexcept
    {
        if (m_shadowOffset == offset)
            return;

        m_shadowOffset = offset;
        addChange(CHShadowOffset);
        updateShadow();
    }

    void setBrush(const AKBrush &brush) noexcept
    {
        if (m_brush == brush)
            return;

        m_brush = brush;
        addChange(CHBrush);
    }

    void setPen(const AKPen &pen) noexcept
    {
        if (m_pen == pen)
            return;

        m_pen = pen;
        addChange(CHPen);
    }

    AKContainer itemsContainer { YGFlexDirectionColumn, true, this };
    AKBackgroundBlurEffect blur { AKBackgroundBlurEffect::Manual, { 16.f, 16.f }, this };
protected:
    AKBrush m_shadowBrush; // Shadow style (color basically)
    AKBrush m_brush; // Fill style
    AKPen m_pen; // Stroke style
    SkScalar m_borderRadius = 8.f;
    SkScalar m_shadowRadius = 24.f;
    SkPoint m_shadowOffset { 0.f, 8.f };
    LAnimation m_fadeInAnimation, m_fadeOutAnimation;

    // Called right before the scene calculates the layout
    void updateShadow() noexcept
    {
        const auto chgs { changes() };
        const bool needsShadowUpdate { chgs.test(CHShadowRadius) };
        const bool needsPaddingUpdate { needsShadowUpdate || chgs.test(CHShadowOffset) };

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
    void bakeEvent(const AKBakeEvent &event) override
    {
        const auto chgs { changes() };

        const bool needsRebake {
            !event.damage.isEmpty() || // If the scene explicitly adds damage it means the node's size changed
            chgs.testAnyOf(CHShadowRadius, CHShadowColor, CHShadowOffset, CHBorderRadius, CHBrush, CHPen)
        };

        if (!needsRebake) // Nothing to do
            return;

        SkCanvas &c { event.canvas() };

        // To improve performance the framebuffer isn't automatically shrank
        // when the node is resized, but since menus aren't constatly resizing
        // calling shrink can free up some GPU memory
        event.surface.shrink();

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
        event.damage.setRect(AK_IRECT_INF);

        // Set the opaque region (to optimize composition)
        if (SkColorGetA(m_brush.getColor()) == 255) // Opaque
        {
            // Make entire rect opaque
            event.opaque.setRect(rect.roundIn());

            // But exclude round corners

            // Top Left
            SkIRect borderRadiusRect = SkRect::MakeXYWH(rect.x(), rect.y(), m_borderRadius, m_borderRadius).roundOut();
            event.opaque.op(borderRadiusRect, SkRegion::Op::kDifference_Op);

            // Top Right
            borderRadiusRect.offset(rect.width() - borderRadiusRect.width(), 0);
            event.opaque.op(borderRadiusRect, SkRegion::Op::kDifference_Op);

            // Bottom Right
            borderRadiusRect.offset(0, rect.height() - borderRadiusRect.height());
            event.opaque.op(borderRadiusRect, SkRegion::Op::kDifference_Op);

            // Bottom Left
            borderRadiusRect.offsetTo(rect.x(), borderRadiusRect.y());
            event.opaque.op(borderRadiusRect, SkRegion::Op::kDifference_Op);
        }
        // Make fully translucent
        else
            event.opaque.setEmpty();
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

        /*
        SkFont font = AKTheme::DefaultFont;
        font.setEmbolden(true);
        m_text.setFont(font);*/
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
    AKText m_text { "" };
};

class Compositor final : public LCompositor
{
public:

    void initialized() override
    {
        kay = std::make_unique<Kay>();

        addFdListener(kay->app.fd(), nullptr, [](auto, auto, auto) -> int {
            akApp()->processLoop(0);
            return 0;
        });

        LCompositor::initialized();
        LLauncher::launch(std::string("swaybg -m fill -i ") + std::string(defaultAssetsPath() / "wallpaper.png"));
    }

    void uninitialized() override
    {
        LCompositor::uninitialized();
        while (!outputs().empty())
            removeOutput(outputs().back());
        kay.reset();
    }

    LFactoryObject *createObjectRequest(LFactoryObject::Type objectType, const void *params) override;

    struct Kay {
        Kay() noexcept
        {
            scene.setRoot(&root);
            surfaces.layout().setPositionType(YGPositionTypeAbsolute);
        }
        AKApplication app;
        AKScene scene;
        AKContainer root;
        AKContainer wallpaper { YGFlexDirectionColumn, false, &root };
        AKContainer background { YGFlexDirectionColumn, false, &root };
        AKContainer surfaces { YGFlexDirectionColumn, false, &root };
        AKContainer overlay { YGFlexDirectionColumn, false, &root};
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
    std::unique_ptr<Kay> kay;
};

class Surface final : public LSurface
{
public:
    Surface(const void *data) noexcept : LSurface(data) {
        node.enableAutoDamage(false);
        node.setSrcRectMode(AKRenderableImage::SrcRectMode::Custom);
        node.layout().setPositionType(YGPositionTypeAbsolute);
    }
    Compositor *comp() const noexcept { return static_cast<Compositor*>(compositor()); }

    AKRenderableImage node { &comp()->kay->surfaces };

    GLuint prevTexSerial { 0 };

    void layerChanged() override
    {
        // Check if the surface has a toplevel role
        if (LToplevelRole *tl = toplevel())
        {
            // Add the activated flag
            tl->configureState(tl->pendingConfiguration().state | LToplevelRole::Activated);

            // Now tl->pendingConfiguration().state contains Activated and the configuration
            // will be sent later to the client.
            // To know when the surface becomes active check LToplevelRole::atomsChanged()
        }

        LSurface::layerChanged();
        if (layer() == LLayerBackground)
            node.setParent(&comp()->kay->wallpaper);
    }

    void orderChanged() override
    {
        AKNode *prev { prevSurface() ? &static_cast<Surface*>(prevSurface())->node : nullptr };
        node.insertAfter(prev);
    }

    void opaqueRegionChanged() override
    {
        LSurface::opaqueRegionChanged();
        Int32 n;
        const LBox *boxes = opaqueRegion().boxes(&n);
        node.opaqueRegion.setRects((const SkIRect*)boxes, n);
    }

    void damageChanged() override
    {
        LSurface::damageChanged();
        Int32 n;
        const LBox *boxes = damage().boxes(&n);
        SkRegion region;
        region.setRects((const SkIRect*)boxes, n);
        node.addDamage(region);
    }

    void bufferTransformChanged() override
    {
        LSurface::bufferTransformChanged();
        node.setSrcTransform(static_cast<AKTransform>(bufferTransform()));
    }

    void bufferScaleChanged() override
    {
        LSurface::bufferScaleChanged();
        node.setCustomSrcRectScale(bufferScale());
    }

    void srcRectChanged() override
    {
        node.setCustomSrcRect(SkRect::MakeXYWH(
            srcRect().x(), srcRect().y(),
            srcRect().w(), srcRect().h()));
    }
};

class Text final : public AKText
{
public:
    using AKText::AKText;
    AKBackgroundImageShadowEffect shadow { 6, {0,0}, 0x66000000, this };
};

static std::mutex m;

class Output final : public LOutput
{
public:
    Output(const void *params) noexcept : LOutput(params) {}
    Compositor *comp() const noexcept { return static_cast<Compositor*>(compositor()); }

    void initializeGL() override
    {
        m.lock();
        kay = std::make_unique<Kay>(this);
        m.unlock();
    }

    void paintGL() override
    {
        m.lock();
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

        const GrBackendRenderTarget backendTarget = GrBackendRenderTargets::MakeGL(
            realBufferSize().w(),
            realBufferSize().h(),
            0, 0,
            fbInfo);

        kay->target->setSurface(SkSurfaces::WrapBackendRenderTarget(
            akApp()->glContext()->skContext().get(),
            backendTarget,
            fbInfo.fFBOID == 0 ? GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin : GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
            SkColorType::kRGB_888x_SkColorType,
            colorSpace,
            &skSurfaceProps));

        kay->target->setImage(louvreTex2SkiaImage(
            usingFractionalScale() && fractionalOversamplingEnabled() ?
            oversamplingTexture() : bufferTexture(currentBuffer()), this));

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

            const GLuint newTexSerial { s->texture()->serial() };
            if (s->prevTexSerial != newTexSerial || !s->node.image())
            {
                s->node.setImage(louvreTex2SkiaImage(s->texture(), this));
                s->prevTexSerial = newTexSerial;
            }
        }

        // We can ask the scene which region was repainted
        if (hasBufferDamageSupport() || usingFractionalScale())
            kay->target->outDamageRegion = &outDamage;
        else
            kay->target->outDamageRegion = nullptr;

        // If hw cursor is disabled or during screen captures
        const LRegion &softwareCursorDamage = cursor()->damage(this);
        boxes = softwareCursorDamage.boxes(&n);

        if (n > 0)
        {
            cursorDamage.setRects((SkIRect*)boxes, n);
            kay->target->inDamageRegion = &cursorDamage;
        }
        else
        {
            kay->target->inDamageRegion = nullptr;
        }

        // Required for damage tracking
        kay->target->setAge(currentBufferAge());

        // Rect of the scene to capture relative to the root node
        kay->target->setViewport(SkRect::MakeXYWH(pos().x(), pos().y(), size().w(), size().h()));

        // If the screen is rotated/flipped
        kay->target->setTransform(static_cast<AKTransform>(transform()));

        // Rect within the screen fb to render the viewport to (in this case the entire screen)
        kay->target->setDstRect(SkIRect::MakeXYWH(0, 0, backendTarget.width(), backendTarget.height()));
        kay->target->setBakedComponentsScale(scale());

        //glScissor(0,0,100000,100000);
        //glClear(GL_COLOR_BUFFER_BIT);

        inPaintGL = false;
        // Here the scene calculates the layout and performs all the rendering
        comp()->kay->scene.render(kay->target);

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
            if (!usingFractionalScale())
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

        m.unlock();
    }

    void resizeGL() override
    {
        if (kay)
            kay->resizeGL();
    }

    void moveGL() override
    {
        if (kay)
            kay->moveGL();
    }

    void uninitializeGL() override
    {
        kay.reset();
        akApp()->freeGLContext();
    }

    bool inPaintGL { false };

    struct Kay
    {
        Kay(Output *output) noexcept : output(output)
        {
            // setScale(1.5f);
            // Louvre creates an OpenGL context for each output
            // here we are wrapping it into a GrDirectContext.

            // All outputs share the same scene but we still need to create a
            // specific target for each.
            // A target contains information about the viewport, destination
            // framebuffer, transform, etc, and is used by the scene and
            // nodes to keep track of damage, previous dimensions, and other properties.
            target = comp()->kay->scene.createTarget();
            target->setClearColor(SK_ColorBLACK);
            target->setViewport(SkRect::MakeXYWH(output->pos().x(), output->pos().y(), output->size().w(), output->size().h()));
            target->setBakedComponentsScale(output->scale());

            target->on.markedDirty.subscribe(target, [output](AKTarget &){

                if (output->inPaintGL)
                    return;
                output->repaint();
            });

            background.layout().setJustifyContent(YGJustifyCenter);
            background.layout().setAlignItems(YGAlignCenter);
            background.layout().setPositionType(YGPositionTypeAbsolute);
            background.layout().setGap(YGGutterAll, 8.f);

            topbar.layout().setPositionType(YGPositionTypeAbsolute);
            topbar.layout().setPosition(YGEdgeLeft, output->pos().x());
            topbar.layout().setPosition(YGEdgeTop, output->pos().x());
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

            for (auto &name : topbarMenuNames)
            {
                topbarMenus.push_back(new Text(name, &topbarBackground));
                topbarMenus.back()->enableCustomTextureColor(true);
                topbarMenus.back()->setColorWithAlpha(SK_ColorWHITE);
                topbarMenus.back()->setOpacity(0.75f);
            }

            topbarExclusiveZone.setOnRectChangeCallback([this](LExclusiveZone *zone){
                topbar.layout().setWidth(zone->rect().w());
                topbar.layout().setHeight(zone->rect().h());
            });

            resizeGL();
            moveGL();
            disabledButton.setEnabled(false);
            customBackgroundButton.setBackgroundColor(AKTheme::SystemBlue);
            customBackgroundButtonDisabled.setBackgroundColor(AKTheme::SystemBlue);
            customBackgroundButtonDisabled.setEnabled(false);
            exitButton.setBackgroundColor(AKTheme::SystemRed);

            exitButton.on.clicked.subscribe(&exitButton, [](){
                compositor()->finish();
            });

            scalex1.setBackgroundColor(AKTheme::SystemGreen);
            scalex1.on.clicked.subscribe(&scalex1, [output](){
                output->setScale(1.f);
            });

            scalex15.setBackgroundColor(AKTheme::SystemOrange);
            scalex15.on.clicked.subscribe(&scalex15, [output](){
                output->setScale(1.5f);
            });

            scalex2.setBackgroundColor(AKTheme::SystemYellow);
            scalex2.on.clicked.subscribe(&scalex2, [output](){
                output->setScale(2.f);
            });

            topbarExclusiveZone.setOutput(output);

            assetsTexture = LOpenGL::loadTexture(compositor()->defaultAssetsPath()/"weston-terminal.png");
            if (assetsTexture)
                assetsTexture->setPremultipliedAlpha(false);
            assetsImage = louvreTex2SkiaImage(assetsTexture, output);
            assetsView.setImage(assetsImage);
            assetsView.layout().setWidth(100);
            assetsView.layout().setHeight(64);
            assetsView.setSizeMode(AKImageFrame::SizeMode::Contain);

            imgTransform.on.clicked.subscribe(&imgTransform, [this](){
                static const char* transformName[] {
                    "Normal",
                    "Rotated90",
                    "Rotated180",
                    "Rotated270",
                    "Flipped",
                    "Flipped90",
                    "Flipped180",
                    "Flipped270"
                };

                if (assetsView.srcTransform() == AKTransform::Flipped270)
                    assetsView.setSrcTransform(AKTransform::Normal);
                else
                    assetsView.setSrcTransform((AKTransform)(Int32(assetsView.srcTransform())+1));

                imgTransform.setText(transformName[Int32(assetsView.srcTransform())]);
            });

            imgAlignment.on.clicked.subscribe(&imgAlignment, [this](){

                static Int32 current { 0 };

                static const AKAlignment alignments[] {
                    AKAlignCenter,
                    AKAlignTop,
                    AKAlignRight,
                    AKAlignBottom,
                    AKAlignLeft
                };

                static const char* alignmentName[] {
                    "Center",
                    "Top",
                    "Right",
                    "Bottom",
                    "Left"
                };

                if (current == 4)
                    current = 0;
                else
                    current++;

                assetsView.setAlignment(alignments[current]);
                imgAlignment.setText(alignmentName[current]);
            });

            imgSizeMode.on.clicked.subscribe(&imgSizeMode, [this](){
                static const char* sizeModeName[] {
                    "Contain",
                    "Cover",
                    "Fill",
                };

                if (assetsView.sizeMode() == AKImageFrame::SizeMode::Fill)
                    assetsView.setSizeMode(AKImageFrame::SizeMode::Contain);
                else
                    assetsView.setSizeMode((AKImageFrame::SizeMode)(Int32(assetsView.sizeMode())+1));

                imgSizeMode.setText(sizeModeName[Int32(assetsView.sizeMode())]);
            });

            auto textStyle = instructions.textStyle();
            textStyle.setFontSize(16);
            textStyle.setFontStyle(SkFontStyle::Bold());
            instructions.setTextStyle(textStyle);

            AKWeak buttonRef { &customBackgroundButton };
            AKTimer::OneShoot(1000, [buttonRef](AKTimer *timer){
                if (buttonRef)
                {
                    buttonRef->setBackgroundColor(0xFF000000 | rand());
                    timer->start(timer->timeoutMs());
                }
            });

        }
        ~Kay()
        {
            comp()->kay->scene.destroyTarget(target);
        }

        Compositor *comp() const noexcept { return static_cast<Compositor*>(compositor()); }

        void moveGL() noexcept
        {
            background.layout().setPosition(YGEdgeLeft, output->pos().x());
            background.layout().setPosition(YGEdgeTop, output->pos().y());
            topbar.layout().setPosition(YGEdgeLeft, output->pos().x() + topbarExclusiveZone.rect().x());
            topbar.layout().setPosition(YGEdgeTop, output->pos().y() + topbarExclusiveZone.rect().y());
        }

        void resizeGL() noexcept
        {
            Int32 x { 0 };
            for (LOutput *o : comp()->outputs())
            {
                o->setPos(LPoint(x, 0));
                x += o->size().w();
                o->moveGL();
            }

            background.layout().setWidth(output->size().w());
            background.layout().setHeight(output->size().h());
        }

        LWeak<Output> output;
        AKContainer background { YGFlexDirectionColumn, false, &comp()->kay->background };
        AKText instructions {
            "F1: Launch Weston Terminal\nRight Click: Show Context Menu.\nNote: Blur only works if launched from a TTY (DRM backend)",
            &background };

        AKContainer buttonsGroup1 { YGFlexDirectionRow, false, &background };
        AKButton normalButton { "Normal Button", &buttonsGroup1};
        AKButton disabledButton { "Disabled Button", &buttonsGroup1 };
        AKButton customBackgroundButton { "Colored Button", &buttonsGroup1 };
        AKButton customBackgroundButtonDisabled { "Colored Button Disabled", &buttonsGroup1 };

        AKContainer buttonsGroup2 { YGFlexDirectionRow, false, &background };
        AKButton scalex1 { "Screen@x1", &buttonsGroup2 };
        AKButton scalex15 { "Screen@x1.5", &buttonsGroup2 };
        AKButton scalex2 { "Screen@x2", &buttonsGroup2 };
        AKButton exitButton { "Exit", &buttonsGroup2 };

        AKContainer buttonsGroup3 { YGFlexDirectionRow, false, &background };
        AKButton imgSizeMode { "SizeMode: Contain", &buttonsGroup3 };
        AKButton imgAlignment { "Alignment: Center", &buttonsGroup3 };
        AKButton imgTransform { "Transform: Normal", &buttonsGroup3 };

        AKTextField textField { &background };

        LWeak<LTexture> assetsTexture;
        sk_sp<SkImage> assetsImage;
        AKImageFrame assetsView { &background };
        AKTarget *target { nullptr };

        AKSubScene topbar { &comp()->kay->overlay };
        AKBackgroundBlurEffect topbarBlur { AKBackgroundBlurEffect::Automatic, {200.f, 200.f}, &topbar};
        AKSolidColor topbarBackground { 0x22FFFFFF, &topbar };
        AKBackgroundBoxShadowEffect topbarShadow {
            16, {0, 0}, 0x45000000, false, &topbar};
        AKPath logo { &topbarBackground };
        AKBackgroundImageShadowEffect logoShadow { 6, {0,0}, 0x66000000, &logo };
        std::vector<Text*>topbarMenus;
        LExclusiveZone topbarExclusiveZone { LEdgeTop, 28 };
    };

    std::unique_ptr<Kay> kay;
};

class Pointer final : public LPointer
{
public:
    using LPointer::LPointer;
    AKPointerMoveEvent moveEvent;
    AKPointerButtonEvent buttonEvent;
    AKWeak<MenuItem> focus;

    AKNode *nodeAt(const SkIPoint &globalPos, UInt64 filter) const noexcept
    {
        return findNode(&static_cast<Compositor*>(compositor())->kay->root, globalPos, filter);
    }

    void pointerMoveEvent(const LPointerMoveEvent &event) override
    {
        LPointer::pointerMoveEvent(event);

        moveEvent.setX(cursor()->pos().x());
        moveEvent.setY(cursor()->pos().y());
        akApp()->sendEvent(moveEvent, static_cast<Compositor*>(compositor())->kay->scene);

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

        buttonEvent.setButton((AKPointerButtonEvent::Button)event.button());
        buttonEvent.setState((AKPointerButtonEvent::State)event.state());
        buttonEvent.setSerial(event.serial());
        buttonEvent.setMs(event.ms());
        buttonEvent.setUs(event.us());
        akApp()->sendEvent(buttonEvent, static_cast<Compositor*>(compositor())->kay->scene);

        if (event.button() == BTN_RIGHT && event.state() == LPointerButtonEvent::Pressed)
        {
            static_cast<Compositor*>(compositor())->kay->menu.showAt(cursor()->pos().x(), cursor()->pos().y());
            return;
        }

        if (event.button() == BTN_LEFT)
        {
            if (!focus)
            {
                if (event.state() == LPointerButtonEvent::Released)
                    static_cast<Compositor*>(compositor())->kay->menu.hide();
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

class Keyboard : public LKeyboard
{
public:
    using LKeyboard::LKeyboard;
    AKKeyboardKeyEvent keyboardKeyEvent;

    void keyEvent(const LKeyboardKeyEvent &event) override
    {
        LKeyboard::keyEvent(event);
        akKeyboard().updateKeyState(event.keyCode(), event.state());
        keyboardKeyEvent.setKeyCode(event.keyCode());
        keyboardKeyEvent.setState((AKKeyboardKeyEvent::State)event.state());
        keyboardKeyEvent.setMs(event.ms());
        keyboardKeyEvent.setSerial(event.serial());
        akApp()->sendEvent(keyboardKeyEvent, static_cast<Compositor*>(compositor())->kay->scene);
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

    if (objectType == LFactoryObject::Type::LKeyboard)
        return new Keyboard(params);

    return nullptr;
}

int main(void)
{
    setenv("LOUVRE_DEBUG", "3", 0);
    setenv("KAY_DEBUG", "3", 0);
    setenv("LOUVRE_WAYLAND_DISPLAY", "louvre", 0);
    setenv("SRM_RENDER_MODE_ITSELF_FB_COUNT", "3", 0);
    setenv("SRM_FORCE_GL_ALLOCATION", "1", 0);

    LLauncher::startDaemon();
    Compositor compositor;
    assert("Compositor failed to start" && compositor.start());

    while (compositor.state() != LCompositor::Uninitialized)
        compositor.processLoop(-1);

    return 0;
}
