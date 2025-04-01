#include <Marco/private/MLayerSurfacePrivate.h>
#include <Marco/private/MSurfacePrivate.h>
#include <Marco/MApplication.h>

using namespace AK;

MLayerSurface::MLayerSurface(Layer layer, AKBitset<AKEdge> anchor, Int32 exclusiveZone, MScreen *screen, const std::string &scope) noexcept :
    MSurface(Role::LayerSurface)
{
    assert("zwlr_layer_shell not supported by the compositor" && app()->wayland().layerShell);
    m_imp = std::make_unique<Imp>(*this);
    imp()->layer = layer;
    imp()->screen = screen;
    imp()->scope = scope;

    imp()->layerSurface = zwlr_layer_shell_v1_get_layer_surface(
        app()->wayland().layerShell,
        wlSurface(),
        screen ? screen->wlOutput() : nullptr,
        layer,
        scope.c_str());

    zwlr_layer_surface_v1_add_listener(imp()->layerSurface, &Imp::layerSurfaceListener, this);

    setAnchor(anchor);
    setExclusiveZone(exclusiveZone);
}

MLayerSurface::~MLayerSurface() noexcept
{
    if (imp()->layerSurface)
    {
        zwlr_layer_surface_v1_destroy(imp()->layerSurface);
        imp()->layerSurface = nullptr;
    }
}

bool MLayerSurface::setScreen(MScreen *screen) noexcept
{
    return false;
}

MScreen *MLayerSurface::screen() const noexcept
{
    return imp()->screen;
}

bool MLayerSurface::setAnchor(AKBitset<AKEdge> edges) noexcept
{
    if (imp()->anchor.get() == edges.get())
        return false;

    imp()->anchor = edges;
    update();
    return true;
}

AKBitset<AKEdge> MLayerSurface::anchor() const noexcept
{
    return imp()->anchor;
}

bool MLayerSurface::setExclusiveZone(Int32 size) noexcept
{
    if (imp()->exclusiveZone == size)
        return false;

    imp()->exclusiveZone = size;
    update();
    return true;
}

Int32 MLayerSurface::exclusiveZone() const noexcept
{
    return imp()->exclusiveZone;
}

bool MLayerSurface::setMargin(const SkIRect &margin) noexcept
{
    if (imp()->margin == margin)
        return false;

    imp()->margin = margin;
    update();
    return true;
}

const SkIRect &MLayerSurface::margin() const noexcept
{
    return imp()->margin;
}

bool MLayerSurface::setKeyboardInteractivity(KeyboardInteractivity mode) noexcept
{
    if (imp()->keyboardInteractivity == mode || (app()->wayland().layerShell.version() < 4 && mode == OnDemand))
        return false;

    imp()->keyboardInteractivity = mode;
    update();
    return true;
}

MLayerSurface::KeyboardInteractivity MLayerSurface::keyboardInteractivity() const noexcept
{
    return imp()->keyboardInteractivity;
}

bool MLayerSurface::setLayer(Layer layer) noexcept
{
    if (app()->wayland().layerShell.version() < 2 || imp()->layer == layer)
        return false;

    imp()->layer = layer;
    update();
    return true;
}

MLayerSurface::Layer MLayerSurface::layer() const noexcept
{
    return imp()->layer;
}

bool MLayerSurface::setExclusiveEdge(AKEdge edge) noexcept
{
    if (app()->wayland().layerShell.version() < 5 || imp()->exclusiveEdge == edge)
        return false;

    imp()->exclusiveEdge = edge;
    update();
    return true;
}

AKEdge MLayerSurface::exclusiveEdge() const noexcept
{
    return imp()->exclusiveEdge;
}

bool MLayerSurface::setScope(const std::string &scope) noexcept
{
    return false;
}

const std::string &MLayerSurface::scope() const noexcept
{
    return imp()->scope;
}

const SkISize &MLayerSurface::suggestedSize() const noexcept
{
    return imp()->suggestedSize;
}

MLayerSurface::Imp *MLayerSurface::imp() const noexcept
{
    return m_imp.get();
}

void MLayerSurface::suggestedSizeChanged()
{
    return;
    if (suggestedSize().width() == 0)
    {
        if (globalRect().width() == 0)
            layout().setWidthAuto();
    }
    else
        layout().setWidth(suggestedSize().width());

    if (suggestedSize().height() == 0)
    {
        if (globalRect().height() == 0)
            layout().setHeightAuto();
    }
    else
        layout().setHeight(suggestedSize().height());
}

void MLayerSurface::render() noexcept
{
    scene().root()->layout().calculate();

    if (wlCallback() && !MSurface::imp()->flags.check(MSurface::Imp::ForceUpdate))
        return;

    MSurface::imp()->flags.remove(MSurface::Imp::ForceUpdate);

    EGLint bufferAge { -1 };

    SkISize newSize {
        SkScalarFloorToInt(layout().calculatedWidth() + layout().calculatedMargin(YGEdgeLeft) + layout().calculatedMargin(YGEdgeRight)),
        SkScalarFloorToInt(layout().calculatedHeight() + layout().calculatedMargin(YGEdgeTop) + layout().calculatedMargin(YGEdgeBottom))
    };

    if (newSize.fWidth < 8) newSize.fWidth = 8;
    if (newSize.fHeight < 8) newSize.fHeight = 8;

    bool sizeChanged = false;

    if (MSurface::imp()->viewportSize != newSize)
    {
        MSurface::imp()->viewportSize = newSize;
        bufferAge = 0;
        sizeChanged = true;

        zwlr_layer_surface_v1_set_size(
            imp()->layerSurface,
            newSize.width(),
            newSize.height());
    }

    SkISize eglWindowSize { newSize };

    sizeChanged |= MSurface::imp()->resizeBuffer(eglWindowSize);

    if (sizeChanged)
    {
        wp_viewport_set_source(wlViewport(),
                               wl_fixed_from_int(0),
                               wl_fixed_from_int(eglWindowSize.height() - newSize.height()),
                               wl_fixed_from_int(newSize.width()),
                               wl_fixed_from_int(newSize.height()));
    }

    eglMakeCurrent(app()->graphics().eglDisplay, eglSurface(), eglSurface(), app()->graphics().eglContext);
    eglSwapInterval(app()->graphics().eglDisplay, 0);

    target()->setDstRect({ 0, 0, surfaceSize().width() * scale(), surfaceSize().height() * scale() });
    target()->setViewport(SkRect::MakeWH(surfaceSize().width(), surfaceSize().height()));
    target()->setSurface(skSurface());

    if (bufferAge != 0)
        eglQuerySurface(app()->graphics().eglDisplay, eglSurface(), EGL_BUFFER_AGE_EXT, &bufferAge);
    target()->setBakedComponentsScale(scale());
    target()->setRenderCalculatesLayout(false);
    target()->setAge(bufferAge);
    SkRegion skDamage, skOpaque;
    target()->outDamageRegion = &skDamage;
    target()->outOpaqueRegion = &skOpaque;
    scene().render(target());
    wl_surface_set_buffer_scale(wlSurface(), scale());
    zwlr_layer_surface_v1_set_margin(imp()->layerSurface,
        margin().fTop, margin().fRight, margin().fBottom, margin().fLeft);

    if (app()->wayland().layerShell.version() >= 2)
    {
        zwlr_layer_surface_v1_set_layer(imp()->layerSurface, layer());

        if (app()->wayland().layerShell.version() >= 5)
            zwlr_layer_surface_v1_set_exclusive_edge(imp()->layerSurface, exclusiveEdge());
    }

    zwlr_layer_surface_v1_set_anchor(imp()->layerSurface, anchor().get());
    zwlr_layer_surface_v1_set_exclusive_zone(imp()->layerSurface, exclusiveZone());
    zwlr_layer_surface_v1_set_keyboard_interactivity(imp()->layerSurface, keyboardInteractivity());

    /* Input region */
    if (sizeChanged)
    {
        SkIRect inputRect { globalRect() };
        inputRect.outset(6, 6);
        wl_region *wlInputRegion = wl_compositor_create_region(app()->wayland().compositor);
        wl_region_add(wlInputRegion, inputRect.x(), inputRect.y(), inputRect.width(), inputRect.height());
        wl_surface_set_input_region(wlSurface(), wlInputRegion);
        wl_region_destroy(wlInputRegion);
    }

    /* In some compositors using fractional scaling (without oversampling), the opaque region
     * can leak, causing borders to appear black. This inset prevents that. */
    SkIRect opaqueClip { globalRect() };
    opaqueClip.inset(1, 1);
    skOpaque.op(opaqueClip, SkRegion::Op::kIntersect_Op);
    wl_region *wlOpaqueRegion = wl_compositor_create_region(app()->wayland().compositor);
    SkRegion::Iterator opaqueIt(skOpaque);
    while (!opaqueIt.done())
    {
        wl_region_add(wlOpaqueRegion, opaqueIt.rect().x(), opaqueIt.rect().y(), opaqueIt.rect().width(), opaqueIt.rect().height());
        opaqueIt.next();
    }
    wl_surface_set_opaque_region(wlSurface(), wlOpaqueRegion);
    wl_region_destroy(wlOpaqueRegion);

    const bool noDamage { skDamage.computeRegionComplexity() == 0 };

    if (noDamage)
        skDamage.setRect(SkIRect(-10, -10, 1, 1));

    EGLint *damageRects { new EGLint[skDamage.computeRegionComplexity() * 4] };
    EGLint *rectsIt = damageRects;
    SkRegion::Iterator damageIt(skDamage);
    while (!damageIt.done())
    {
        *rectsIt = damageIt.rect().x() * scale();
        rectsIt++;
        *rectsIt = bufferSize().height() - ((eglWindowSize.height() - newSize.height()) + damageIt.rect().height() + damageIt.rect().y()) * scale();
        rectsIt++;
        *rectsIt = damageIt.rect().width() * scale();
        rectsIt++;
        *rectsIt = damageIt.rect().height() * scale();
        rectsIt++;
        damageIt.next();
    }

    MSurface::imp()->createCallback();
    assert(app()->graphics().eglSwapBuffersWithDamageKHR(app()->graphics().eglDisplay, eglSurface(), damageRects, skDamage.computeRegionComplexity()) == EGL_TRUE);
    delete []damageRects;
}

void MLayerSurface::onUpdate() noexcept
{
    MSurface::onUpdate();

    if (MSurface::imp()->flags.check(MSurface::Imp::PendingConfigureAck))
    {
        MSurface::imp()->flags.remove(MSurface::Imp::PendingConfigureAck);
        zwlr_layer_surface_v1_ack_configure(imp()->layerSurface, imp()->configureSerial);
    }

    if (visible())
    {
        if (MSurface::imp()->flags.check(MSurface::Imp::PendingNullCommit))
        {
            MSurface::imp()->flags.add(MSurface::Imp::PendingFirstConfigure);
            MSurface::imp()->flags.remove(MSurface::Imp::PendingNullCommit);
            wl_surface_attach(wlSurface(), nullptr, 0, 0);
            wl_surface_commit(wlSurface());
            update();
            return;
        }
    }
    else
    {
        if (!MSurface::imp()->flags.check(MSurface::Imp::PendingNullCommit))
        {
            MSurface::imp()->flags.add(MSurface::Imp::PendingNullCommit);
            wl_surface_attach(wlSurface(), nullptr, 0, 0);
            wl_surface_commit(wlSurface());
        }

        return;
    }

    if (MSurface::imp()->flags.check(MSurface::Imp::PendingFirstConfigure | MSurface::Imp::PendingNullCommit))
        return;

    if (MSurface::MSurface::imp()->tmpFlags.check(MSurface::Imp::ScaleChanged))
        MSurface::imp()->flags.add(MSurface::Imp::ForceUpdate);

    render();
}
