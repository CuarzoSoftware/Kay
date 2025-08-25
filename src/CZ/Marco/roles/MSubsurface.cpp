#include <CZ/Marco/private/MSubsurfacePrivate.h>
#include <CZ/Marco/private/MSurfacePrivate.h>
#include <CZ/Marco/roles/MPopup.h>
#include <CZ/Marco/roles/MToplevel.h>
#include <CZ/Marco/MApplication.h>

using namespace CZ;

MSubsurface::MSubsurface(MSurface *parent) noexcept :
    MSurface(MSurface::Role::SubSurface)
{
    assert("wl_subcompositor not supported by the compositor" && app()->wayland().subCompositor);
    m_imp = std::make_unique<Imp>(*this);
    setParent(parent);
}

MSubsurface::~MSubsurface()
{
    setParent(nullptr);
}

MSurface *MSubsurface::parent() const noexcept
{
    return imp()->parent;
}

bool MSubsurface::setParent(MSurface *surface) noexcept
{
    if (imp()->parent == surface)
        return true;

    // Check if it is a descendant
    if (surface && surface->role() == MSurface::Role::SubSurface)
    {
        MSubsurface *check { (MSubsurface*)surface };

        while (check)
        {
            if (check == this || check->parent() == this)
                return false;

            if (check->parent() && check->parent()->role() == MSurface::Role::SubSurface)
                check = (MSubsurface*)check->parent();
            else
                break;
        }
    }

    if (surface)
    {
        if (imp()->parent)
        {
            imp()->parent->imp()->subSurfaces.erase(imp()->parentLink);
            imp()->parent.reset();

            if (imp()->wlSubSurface)
            {
                wl_subsurface_destroy(imp()->wlSubSurface);
                imp()->wlSubSurface = nullptr;
            }
        }

        imp()->parent.reset(surface);
        imp()->parent->imp()->subSurfaces.push_back(this);
        imp()->parentLink = std::prev(imp()->parent->imp()->subSurfaces.end());
        imp()->wlSubSurface = wl_subcompositor_get_subsurface(app()->wayland().subCompositor, wlSurface(), parent()->wlSurface());
        update();
        surface->update(true);
    }
    else
    {
        imp()->parent->imp()->subSurfaces.erase(imp()->parentLink);
        imp()->parent.reset();

        wl_subsurface_destroy(imp()->wlSubSurface);
        imp()->wlSubSurface = nullptr;
        MSurface::imp()->setMapped(false);
    }

    return true;
}

const std::list<MSubsurface*>::iterator &MSubsurface::parentLink() const noexcept
{
    return imp()->parentLink;
}

bool MSubsurface::placeAbove(MSubsurface *subSurface) noexcept
{
    if (!parent() || subSurface == this)
        return false;

    if (subSurface)
    {
        if (subSurface->parent() != parent())
            return false;

        if (std::next(subSurface->imp()->parentLink) == imp()->parentLink)
            return true;

        parent()->imp()->subSurfaces.erase(imp()->parentLink);

        if (subSurface == parent()->subSurfaces().back())
        {
            parent()->imp()->subSurfaces.push_back(this);
            imp()->parentLink = std::prev(parent()->imp()->subSurfaces.end());
        }
        else
            imp()->parentLink = parent()->imp()->subSurfaces.insert(std::next(subSurface->imp()->parentLink), this);

        wl_subsurface_place_above(imp()->wlSubSurface, subSurface->wlSurface());
    }
    else
    {
        if (parent()->subSurfaces().front() == this)
            return true;

        parent()->imp()->subSurfaces.erase(imp()->parentLink);
        parent()->imp()->subSurfaces.push_front(this);
        imp()->parentLink = parent()->imp()->subSurfaces.begin();
        wl_subsurface_place_above(imp()->wlSubSurface, parent()->wlSurface());
    }

    update();
    parent()->update(true);
    return true;
}

bool MSubsurface::placeBelow(MSubsurface *subSurface) noexcept
{
    if (!subSurface || !parent() || subSurface->parent() != parent())
        return false;

    if (std::prev(subSurface->imp()->parentLink) == imp()->parentLink)
        return true;

    parent()->imp()->subSurfaces.erase(imp()->parentLink);
    imp()->parentLink = parent()->imp()->subSurfaces.insert(subSurface->imp()->parentLink, this);
    wl_subsurface_place_below(imp()->wlSubSurface, subSurface->wlSurface());
    update();
    parent()->update(true);
    return true;
}

const SkIPoint &MSubsurface::pos() const noexcept
{
    return imp()->pos;
}

bool MSubsurface::isChildOfRole(Role role) const noexcept
{
    MSurface *p { parent() };

    while (p)
    {
        if (p->role() == role)
            return true;

        switch (p->role()) {
        case Role::LayerSurface:
            return false;
            break;
        case Role::Popup:
            p = static_cast<MPopup*>(p)->parent();
            break;
        case Role::Toplevel:
            p = static_cast<MToplevel*>(p)->parentToplevel();
            break;
        default:
            return false;
        }
    }

    return false;
}

void MSubsurface::setPos(const SkIPoint &pos) noexcept
{
    if (pos == imp()->pos)
        return;

    imp()->pos = pos;

    if (parent() && mapped())
    {
        wl_subsurface_set_position(imp()->wlSubSurface, pos.x(), pos.y());
        wl_surface_commit(wlSurface());

        // Protocol: The position is always applied when the parent commits
        parent()->update(true);
        return;
    }

    // From onUpdate() the parent is requested to commit later
    update(true);
    imp()->posChanged = true;
}

MSubsurface::Imp *MSubsurface::imp() const noexcept
{
    return m_imp.get();
}

void MSubsurface::onUpdate() noexcept
{
    MSurface::onUpdate();

    if (!parent())
        return;

    CZWeak<MSubsurface> ref { this };

    if (MSurface::imp()->flags.has(MSurface::Imp::UserMapped))
    {
        if (!mapped())
        {
            parent()->update(true);
            MSurface::imp()->setMapped(true);
        }
    }
    else
    {
        if (mapped())
        {
            parent()->update(true);
            wl_surface_attach(wlSurface(), NULL, 0, 0);
            wl_surface_commit(wlSurface());
            MSurface::imp()->setMapped(false);
        }
    }

    if (!ref || !mapped()) return;

    if (imp()->posChanged)
    {
        imp()->posChanged = false;
        wl_subsurface_set_position(imp()->wlSubSurface, pos().x(), pos().y());
        update(true);
        parent()->update(true);
    }

    if (MSurface::imp()->tmpFlags.has(MSurface::Imp::ScaleChanged))
        update(true);

    render();
}

void MSubsurface::render() noexcept
{
    scene().root()->layout().calculate();

    if (wlCallback() && !MSurface::imp()->flags.has(MSurface::Imp::ForceUpdate))
        return;

    bool repaint { false };

    parent()->update(true);
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
