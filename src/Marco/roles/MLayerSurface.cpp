#include <Marco/private/MLayerSurfacePrivate.h>
#include <Marco/private/MSurfacePrivate.h>
#include <Marco/MApplication.h>
#include <AK/AKLog.h>

using namespace AK;

MLayerSurface::MLayerSurface() noexcept :
    MSurface(Role::LayerSurface)
{
    assert("zwlr_layer_shell not supported by the compositor" && app()->wayland().layerShell);
    m_imp = std::make_unique<Imp>(*this);
}

MLayerSurface::~MLayerSurface() noexcept
{
    if (imp()->layerSurface)
    {
        zwlr_layer_surface_v1_destroy(imp()->layerSurface);
        imp()->layerSurface = nullptr;
    }
}

void MLayerSurface::requestAvailableWidth() noexcept
{
    if (imp()->requestAvailableWidth)
        return;

    imp()->requestAvailableWidth = true;
    update(true);
}

void MLayerSurface::requestAvailableHeight() noexcept
{
    if (imp()->requestAvailableHeight)
        return;

    imp()->requestAvailableHeight = true;
    update(true);
}

bool MLayerSurface::setScreen(MScreen *screen) noexcept
{
    if (screen == imp()->pendingScreen)
        return false;

    imp()->pendingScreen = screen;
    update();
    return true;
}

MScreen *MLayerSurface::screen() const noexcept
{
    return imp()->pendingScreen;
}

bool MLayerSurface::setAnchor(AKBitset<AKEdge> edges) noexcept
{
    if (imp()->pendingAnchor.get() == edges.get())
        return false;

    imp()->pendingAnchor = edges;
    update(true);
    return true;
}

AKBitset<AKEdge> MLayerSurface::anchor() const noexcept
{
    return imp()->pendingAnchor;
}

bool MLayerSurface::setExclusiveZone(Int32 size) noexcept
{
    if (size < -1)
    {
        AKLog::warning("[MLayerSurface::setExclusiveZone] Invalid value %d. Using -1 instead.", size);
        size = -1;
    }

    if (imp()->pendingExclusiveZone == size)
        return false;

    imp()->pendingExclusiveZone = size;
    update(true);
    return true;
}

Int32 MLayerSurface::exclusiveZone() const noexcept
{
    return imp()->pendingExclusiveZone;
}

bool MLayerSurface::setMargin(const SkIRect &margin) noexcept
{
    if (imp()->pendingMargin == margin)
        return false;

    imp()->pendingMargin = margin;
    update(true);
    return true;
}

const SkIRect &MLayerSurface::margin() const noexcept
{
    return imp()->pendingMargin;
}

bool MLayerSurface::setKeyboardInteractivity(KeyboardInteractivity mode) noexcept
{
    if (imp()->pendingKeyboardInteractivity == mode)
        return false;

    if ((app()->wayland().layerShell.version() < 4 && mode == OnDemand))
    {
        AKLog::warning("[MLayerSurface::setKeyboardInteractivity] OnDemand option not supported by the compositor. Keeping current value.");
        return false;
    }

    imp()->pendingKeyboardInteractivity = mode;
    update(true);
    return true;
}

MLayerSurface::KeyboardInteractivity MLayerSurface::keyboardInteractivity() const noexcept
{
    return imp()->pendingKeyboardInteractivity;
}

bool MLayerSurface::setLayer(Layer layer) noexcept
{
    if (imp()->pendingLayer == layer)
        return false;

    if (app()->wayland().layerShell.version() < 2 && imp()->layerSurface)
        imp()->reset();

    imp()->pendingLayer = layer;
    update(true);
    return true;
}

MLayerSurface::Layer MLayerSurface::layer() const noexcept
{
    return imp()->pendingLayer;
}

bool MLayerSurface::setExclusiveEdge(AKEdge edge) noexcept
{
    if (imp()->pendingExclusiveEdge == edge)
        return false;

    if (app()->wayland().layerShell.version() < 5)
    {
        AKLog::warning("[MLayerSurface::setExclusiveEdge] Request not supported by the compositor. Ignoring it.");
        return false;
    }

    imp()->pendingExclusiveEdge = edge;
    update(true);
    return true;
}

AKEdge MLayerSurface::exclusiveEdge() const noexcept
{
    return imp()->pendingExclusiveEdge;
}

bool MLayerSurface::setScope(const std::string &scope) noexcept
{
    if (imp()->pendingScope == scope)
        return false;

    imp()->pendingScope = scope;
    update();
    return true;
}

const std::string &MLayerSurface::scope() const noexcept
{
    return imp()->pendingScope;
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
    if (suggestedSize().width() != 0)
        layout().setWidth(suggestedSize().width());

    if (suggestedSize().height() != 0)
        layout().setHeight(suggestedSize().height());
}

void MLayerSurface::render() noexcept
{
    scene().root()->layout().calculate();

    if (wlCallback() && !MSurface::imp()->flags.check(MSurface::Imp::ForceUpdate))
        return;

    MSurface::imp()->flags.remove(MSurface::Imp::ForceUpdate);

    bool repaint { false };
    bool anchorChanged = false;

    /* MARGIN CHANGE */

    if (imp()->pendingMargin != imp()->currentMargin)
    {
        imp()->currentMargin = imp()->pendingMargin;

        zwlr_layer_surface_v1_set_margin(imp()->layerSurface,
                                         margin().fTop, margin().fRight, margin().fBottom, margin().fLeft);
    }

    if (app()->wayland().layerShell.version() >= 2)
    {
        /* LAYER CHANGE */

        if (imp()->pendingLayer != imp()->currentLayer)
        {
            imp()->currentLayer = imp()->pendingLayer;
            zwlr_layer_surface_v1_set_layer(imp()->layerSurface, layer());
        }

        if (app()->wayland().layerShell.version() >= 5)
        {
            /* EXCLUSIVE EDGE CHANGE */

            if (imp()->pendingExclusiveEdge != imp()->currentExclusiveEdge)
            {
                imp()->currentExclusiveEdge = imp()->pendingExclusiveEdge;
                zwlr_layer_surface_v1_set_exclusive_edge(imp()->layerSurface,
                    anchor().check(exclusiveEdge()) ? exclusiveEdge() : AKEdgeNone);
            }
        }
    }

    /* EXCLUSIVE ZONE CHANGE */

    if (imp()->pendingExclusiveZone != imp()->currentExclusiveZone)
    {
        imp()->currentExclusiveZone = imp()->pendingExclusiveZone;
        zwlr_layer_surface_v1_set_exclusive_zone(imp()->layerSurface, exclusiveZone());
    }

    /* KEYBOARD INT CHANGE */

    if (imp()->pendingKeyboardInteractivity != imp()->currentKeyboardInteractivity)
    {
        imp()->currentKeyboardInteractivity = imp()->currentKeyboardInteractivity;
        zwlr_layer_surface_v1_set_keyboard_interactivity(imp()->layerSurface, keyboardInteractivity());
    }

    /* ANCHOR CHANGE */

    if (imp()->pendingAnchor.get() != imp()->currentAnchor.get())
    {
        imp()->currentAnchor = imp()->pendingAnchor;
        zwlr_layer_surface_v1_set_anchor(imp()->layerSurface, anchor().get());
        anchorChanged = true;
    }

    EGLint bufferAge { -1 };

    SkISize newSize {
        SkScalarFloorToInt(layout().calculatedWidth()),
        SkScalarFloorToInt(layout().calculatedHeight())
    };

    if (newSize.fWidth < 8) newSize.fWidth = 8;
    if (newSize.fHeight < 8) newSize.fHeight = 8;

    bool sizeChanged = false;

    if (MSurface::imp()->viewportSize != newSize)
    {
        MSurface::imp()->viewportSize = newSize;
        bufferAge = 0;
        sizeChanged = true;
    }

    SkISize eglWindowSize { newSize };

    sizeChanged |= MSurface::imp()->resizeBuffer(eglWindowSize);

    if (sizeChanged)
    {
        repaint = true;
        wp_viewport_set_source(wlViewport(),
                               wl_fixed_from_int(0),
                               wl_fixed_from_int(eglWindowSize.height() - newSize.height()),
                               wl_fixed_from_int(newSize.width()),
                               wl_fixed_from_int(newSize.height()));
    }

    if (sizeChanged || anchorChanged)
    {
        /* PREVENT 0 SIZE PROTOCOL ERROR */

        const Int32 finalW = imp()->requestAvailableWidth && anchor().checkAll(AKEdgeLeft | AKEdgeRight) ? 0 : newSize.width();
        const Int32 finalH = imp()->requestAvailableHeight && anchor().checkAll(AKEdgeTop | AKEdgeBottom) ? 0 : newSize.height();

        imp()->requestAvailableWidth = false;
        imp()->requestAvailableHeight = false;

        zwlr_layer_surface_v1_set_size(imp()->layerSurface, finalW, finalH);
    }

    repaint |= target()->isDirty() || target()->bakedComponentsScale() != scale();

    if (!repaint)
    {
        wl_surface_commit(wlSurface());
        return;
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
    auto &flags = MSurface::imp()->flags;
    using SF = MSurface::Imp::Flags;
    auto &tmpFlags = MSurface::imp()->tmpFlags;
    using STF = MSurface::Imp::TmpFlags;

    MSurface::onUpdate();

    /* SCREEN CHANGE */

    if (imp()->currentScreen != imp()->pendingScreen)
    {
        imp()->currentScreen = imp()->pendingScreen;
        imp()->reset();
    }

    /* SCOPE CHANGE */

    if (imp()->currentScope != imp()->pendingScope)
    {
        imp()->currentScope = imp()->pendingScope;
        imp()->reset();
    }

    if (!flags.check(SF::UserMapped))
    {
        if (flags.check(SF::PendingNullCommit))
            return;

        imp()->reset();
        return;
    }

    if (!imp()->layerSurface)
        imp()->createRole();

    if (flags.check(SF::PendingNullCommit))
    {
        flags.add(SF::PendingFirstConfigure);
        flags.remove(SF::PendingNullCommit);
        wl_surface_attach(wlSurface(), nullptr, 0, 0);
        wl_surface_commit(wlSurface());
        update();
        return;
    }

    if (flags.check(SF::PendingConfigureAck))
    {
        flags.remove(SF::PendingConfigureAck);
        zwlr_layer_surface_v1_ack_configure(imp()->layerSurface, imp()->configureSerial);
    }

    if (flags.check(SF::PendingFirstConfigure | SF::PendingNullCommit))
        return;

    if (tmpFlags.check(STF::ScaleChanged))
        flags.add(SF::ForceUpdate);

    render();
}
