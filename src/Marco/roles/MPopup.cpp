#include <AK/events/AKInputEvent.h>
#include <AK/AKLog.h>
#include <Marco/private/MPopupPrivate.h>
#include <Marco/private/MToplevelPrivate.h>
#include <Marco/private/MLayerSurfacePrivate.h>

using namespace AK;

MPopup::MPopup() noexcept : MSurface(MSurface::Role::Popup)
{
    m_imp = std::make_unique<Imp>(*this);
}

MPopup::~MPopup()
{
    setParent(nullptr);
    imp()->destroy();

    while (!imp()->childPopups.empty())
        (*imp()->childPopups.begin())->setParent(nullptr);

    if (imp()->xdgSurface)
    {
        xdg_surface_destroy(imp()->xdgSurface);
        imp()->xdgSurface = nullptr;
    }
}

bool MPopup::setParent(MSurface *parent) noexcept
{
    if (parent == this->parent())
        return true;

    if (parent)
    {
        if (parent->role() != Role::Toplevel && parent->role() != Role::Popup && parent->role() != Role::LayerSurface)
        {
            AKLog::error("[MPopup::setParent] Popups can only be child of MToplevel, MPopup or MLayerSurface roles.");
            return false;
        }

        // Check circular dependency
        if (parent->role() == Role::Popup)
        {
            MPopup *popupParent { static_cast<MPopup*>(parent) };

            while (popupParent)
            {
                if (popupParent == this)
                    return false;

                if (popupParent->parent() && popupParent->parent()->role() == Role::Popup)
                    popupParent = static_cast<MPopup*>(popupParent->parent());
                else
                    break;
            }
        }

        imp()->setParent(parent);
    }
    else
    {
        imp()->unsetParent();
    }

    imp()->paramsChanged = true;
    update();
    return true;
}

MSurface *MPopup::parent() const noexcept
{
    return imp()->parent;
}

const std::unordered_set<MPopup *> &MPopup::childPopups() const noexcept
{
    return imp()->childPopups;
}

void MPopup::setAnchorRect(const SkIRect &rect) noexcept
{
    if (anchorRect() == rect)
        return;

    imp()->paramsChanged = true;
    imp()->anchorRect = rect;
    update();
}

const SkIRect &MPopup::anchorRect() const noexcept
{
    return imp()->anchorRect;
}

void MPopup::setAnchor(Anchor anchor) noexcept
{
    if (this->anchor() == anchor)
        return;

    imp()->paramsChanged = true;
    imp()->anchor = anchor;
    update();
}

MPopup::Anchor MPopup::anchor() const noexcept
{
    return imp()->anchor;
}

void MPopup::setGravity(Gravity gravity) noexcept
{
    if (this->gravity() == gravity)
        return;

    imp()->paramsChanged = true;
    imp()->gravity = gravity;
    update();
}

MPopup::Gravity MPopup::gravity() const noexcept
{
    return imp()->gravity;
}

void MPopup::setConstraintAdjustment(AKBitset<ConstraintAdjustment> adjustment) noexcept
{
    adjustment &= ConstraintAdjustment::All;

    if (adjustment.get() == constraintAdjustment().get())
        return;

    imp()->paramsChanged = true;
    imp()->constraintAdjustment.set(adjustment.get());
    update();
}

AKBitset<MPopup::ConstraintAdjustment> MPopup::constraintAdjustment() const noexcept
{
    return imp()->constraintAdjustment;
}

void MPopup::setOffset(const SkIPoint &offset) noexcept
{
    if (this->offset() == offset)
        return;

    imp()->paramsChanged = true;
    imp()->offset = offset;
    update();
}

const SkIPoint &MPopup::offset() const noexcept
{
    return imp()->offset;
}

void MPopup::setGrab(const AKInputEvent *event) noexcept
{
    imp()->paramsChanged = true;
    if (event)
        imp()->grab.reset((AKInputEvent*)event->copy());
    else
        imp()->grab.reset();
    update();
}

AKInputEvent *MPopup::grab() const noexcept
{
    return imp()->grab.get();
}

const SkIRect &MPopup::assignedRect() const noexcept
{
    return imp()->currentAssignedRect;
}

MPopup::Imp *MPopup::imp() const noexcept
{
    return m_imp.get();
}

void MPopup::closeEvent(const AKWindowCloseEvent &event)
{
    onClose.notify(event);
}

void MPopup::decorationMarginsChanged()
{
    onDecorationMarginsChanged.notify();
}

void MPopup::assignedRectChanged()
{
    onAssignedRectChanged.notify();
    layout().setWidth(assignedRect().width());
    layout().setHeight(assignedRect().height());
}

bool MPopup::event(const AKEvent &e)
{
    if (e.type() == AKEvent::WindowClose)
    {
        closeEvent((const AKWindowCloseEvent&)e);
        return true;
    }

    return MSurface::event(e);
}

void MPopup::onUpdate() noexcept
{
    auto &flags = MSurface::imp()->flags;
    using SF = MSurface::Imp::Flags;
    auto &tmpFlags = MSurface::imp()->tmpFlags;
    using STF = MSurface::Imp::TmpFlags;

    MSurface::onUpdate();

    if (imp()->paramsChanged)
    {
        imp()->paramsChanged = false;
        imp()->destroy();
    }

    if (!flags.check(SF::UserMapped) || !parent() || !parent()->mapped())
    {
        return imp()->destroy();
    }

    if (flags.check(SF::PendingNullCommit))
    {
        flags.add(SF::PendingFirstConfigure);
        flags.remove(SF::PendingNullCommit);
        imp()->create();
        update();
        return;
    }

    if (!imp()->xdgPopup)
        return;

    if (flags.check(SF::PendingConfigureAck))
    {
        flags.remove(SF::PendingConfigureAck);
        xdg_surface_ack_configure(imp()->xdgSurface, imp()->configureSerial);
    }

    if (flags.check(SF::PendingFirstConfigure | SF::PendingNullCommit))
    {
        return;
    }

    MSurface::imp()->setMapped(true);

    if (tmpFlags.check(STF::ScaleChanged))
        flags.add(SF::ForceUpdate);

    layout().setPosition(YGEdgeLeft, 0.f);
    layout().setPosition(YGEdgeTop, 0.f);
    layout().setMargin(YGEdgeAll, 0.f);
    render();
}

void MPopup::render() noexcept
{
    scene().root()->layout().calculate();

    if (wlCallback() && !MSurface::imp()->flags.check(MSurface::Imp::ForceUpdate))
        return;

    bool repaint { !MSurface::imp()->flags.check(MSurface::Imp::HasBufferAttached) };

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
    }

    SkISize eglWindowSize { newSize };

    sizeChanged |= MSurface::imp()->resizeBuffer(eglWindowSize);

    if (sizeChanged)
    {
        repaint = true;
        app()->update();

        wp_viewport_set_source(wlViewport(),
                               wl_fixed_from_int(0),
                               wl_fixed_from_int(eglWindowSize.height() - newSize.height()),
                               wl_fixed_from_int(newSize.width()),
                               wl_fixed_from_int(newSize.height()));

        xdg_surface_set_window_geometry(
            imp()->xdgSurface,
            layout().calculatedMargin(YGEdgeLeft),
            layout().calculatedMargin(YGEdgeTop),
            layout().calculatedWidth(),
            layout().calculatedHeight());
    }

    repaint |= target()->isDirty() || target()->bakedComponentsScale() != scale();

    if (!repaint)
    {
        //imp()->applyPendingParent();
        //imp()->applyPendingChildren();
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
    MSurface::imp()->flags.add(MSurface::Imp::HasBufferAttached);
}
