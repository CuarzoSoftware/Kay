#include <Marco/private/MToplevelPrivate.h>
#include <Marco/private/MSurfacePrivate.h>
#include <Marco/roles/MPopup.h>
#include <Marco/MApplication.h>
#include <Marco/MTheme.h>

#include <AK/events/AKLayoutEvent.h>
#include <AK/events/AKWindowStateEvent.h>
#include <AK/events/AKPointerButtonEvent.h>
#include <AK/AKLog.h>

#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/core/SkColorSpace.h>
#include <include/utils/SkParsePath.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

using namespace AK;

MToplevel::MToplevel() noexcept : MSurface(Role::Toplevel)
{
    m_imp = std::make_unique<Imp>(*this);
    root()->installEventFilter(this);

    if (app()->wayland().xdgWmBase.version() < 5)
        imp()->pendingWMCaps = imp()->currentWMCaps = { WindowMenuCap | MinimizeCap | MaximizeCap | FullscreenCap };

    imp()->xdgSurface = xdg_wm_base_get_xdg_surface(app()->wayland().xdgWmBase, wlSurface());
    xdg_surface_add_listener(imp()->xdgSurface, &Imp::xdgSurfaceListener, this);
    imp()->xdgToplevel = xdg_surface_get_toplevel(imp()->xdgSurface);
    xdg_toplevel_add_listener(imp()->xdgToplevel, &Imp::xdgToplevelListener, this);
    xdg_toplevel_set_app_id(imp()->xdgToplevel, app()->appId().c_str());

    app()->onAppIdChanged.subscribe(this, [this](){
        xdg_toplevel_set_app_id(imp()->xdgToplevel, app()->appId().c_str());
    });

    onMappedChanged.subscribe(this, [this](){

        if (!mapped())
            return;

        if (!childToplevels().empty())
        {
            MSurface::imp()->flags.add(MSurface::Imp::PendingChildren | MSurface::Imp::ForceUpdate);
            update();

            for (const auto &child : childToplevels())
            {
                child->MSurface::imp()->flags.add(MSurface::Imp::PendingParent | MSurface::Imp::ForceUpdate);
                child->update();
            }
        }

        if (parentToplevel())
        {
            MSurface::imp()->flags.add(MSurface::Imp::PendingParent | MSurface::Imp::ForceUpdate);
            update();
        }
    });

    /* CSD */

    for (int i = 0; i < 4; i++)
    {
        imp()->borderRadius[i].setParent(rootNode());
        imp()->borderRadius[i].layout().setPositionType(YGPositionTypeAbsolute);
        imp()->borderRadius[i].layout().setWidth(app()->theme()->CSDBorderRadius);
        imp()->borderRadius[i].layout().setHeight(app()->theme()->CSDBorderRadius);
        imp()->borderRadius[i].enableCustomBlendFunc(true);
        imp()->borderRadius[i].enableAutoDamage(false);
        imp()->borderRadius[i].setCustomBlendFunc({
            .sRGBFactor = GL_ZERO,
            .dRGBFactor = GL_SRC_ALPHA,
            .sAlphaFactor = GL_ZERO,
            .dAlphaFactor = GL_SRC_ALPHA,
        });

        /*
        imp()->borderRadius[i].opaqueRegion.setEmpty();
        imp()->borderRadius[i].reactiveRegion.setRect(
            SkIRect::MakeWH(app()->theme()->CSDBorderRadius, app()->theme()->CSDBorderRadius));*/
    }

    // TODO: update only when activated changes

    // TL
    imp()->borderRadius[0].setSrcTransform(AKTransform::Normal);
    imp()->borderRadius[0].layout().setPosition(YGEdgeLeft, 0);
    imp()->borderRadius[0].layout().setPosition(YGEdgeTop, 0);

    // TR
    imp()->borderRadius[1].setSrcTransform(AKTransform::Rotated90);
    imp()->borderRadius[1].layout().setPosition(YGEdgeRight, 0);
    imp()->borderRadius[1].layout().setPosition(YGEdgeTop, 0);

    // BR
    imp()->borderRadius[2].setSrcTransform(AKTransform::Rotated180);
    imp()->borderRadius[2].layout().setPosition(YGEdgeRight, 0);
    imp()->borderRadius[2].layout().setPosition(YGEdgeBottom, 0);

    // BL
    imp()->borderRadius[3].setSrcTransform(AKTransform::Rotated270);
    imp()->borderRadius[3].layout().setPosition(YGEdgeLeft, 0);
    imp()->borderRadius[3].layout().setPosition(YGEdgeBottom, 0);

    imp()->shadow.setParent(rootNode());
}

MToplevel::~MToplevel() noexcept
{
    while (!imp()->childPopups.empty())
        (*imp()->childPopups.begin())->setParent(nullptr);

    setParentToplevel(nullptr);

    if (imp()->xdgDecoration)
    {
        zxdg_toplevel_decoration_v1_destroy(imp()->xdgDecoration);
        imp()->xdgDecoration = nullptr;
    }

    xdg_toplevel_destroy(imp()->xdgToplevel);
    xdg_surface_destroy(imp()->xdgSurface);
}

void MToplevel::setMaximized(bool maximized) noexcept
{
    if (!wmCapabilities().check(MaximizeCap))
        return;

    if (maximized)
        xdg_toplevel_set_maximized(imp()->xdgToplevel);
    else
        xdg_toplevel_unset_maximized(imp()->xdgToplevel);
}

bool MToplevel::maximized() const noexcept
{
    return states().check(AKMaximized);
}

void MToplevel::setFullscreen(bool fullscreen, MScreen *screen) noexcept
{
    if (!wmCapabilities().check(FullscreenCap))
        return;

    if (fullscreen)
        xdg_toplevel_set_fullscreen(imp()->xdgToplevel, screen ? screen->wlOutput() : nullptr);
    else
        xdg_toplevel_unset_fullscreen(imp()->xdgToplevel);
}

bool MToplevel::fullscreen() const noexcept
{
    return states().check(AKFullscreen);
}

void MToplevel::setMinimized() noexcept
{
    if (!wmCapabilities().check(MinimizeCap))
        return;

    xdg_toplevel_set_minimized(imp()->xdgToplevel);
}

void MToplevel::setMinSize(const SkISize &size) noexcept
{
    imp()->minSize = size;

    if (imp()->minSize.fWidth < 0) imp()->minSize.fWidth = 0;
    if (imp()->minSize.fHeight < 0) imp()->minSize.fHeight = 0;

    xdg_toplevel_set_min_size(imp()->xdgToplevel, imp()->minSize.fWidth, imp()->minSize.fHeight);

    layout().setMinWidth(imp()->minSize.fWidth == 0 ? YGUndefined : imp()->minSize.fWidth);
    layout().setMinHeight(imp()->minSize.fHeight == 0 ? YGUndefined : imp()->minSize.fHeight);
}

const SkISize &MToplevel::minSize() const noexcept
{
    return imp()->minSize;
}

void MToplevel::setMaxSize(const SkISize &size) noexcept
{
    imp()->maxSize = size;

    if (imp()->maxSize.fWidth < 0) imp()->maxSize.fWidth = 0;
    if (imp()->maxSize.fHeight < 0) imp()->maxSize.fHeight = 0;

    xdg_toplevel_set_max_size(imp()->xdgToplevel, imp()->maxSize.fWidth, imp()->maxSize.fHeight);

    layout().setMinWidth(imp()->maxSize.fWidth == 0 ? YGUndefined : imp()->maxSize.fWidth);
    layout().setMinHeight(imp()->maxSize.fHeight == 0 ? YGUndefined : imp()->maxSize.fHeight);
}

const SkISize &MToplevel::maxSize() const noexcept
{
    return imp()->maxSize;
}

const SkISize &MToplevel::suggestedSize() const noexcept
{
    return imp()->currentSuggestedSize;
}

const SkISize &MToplevel::suggestedBounds() const noexcept
{
    return imp()->currentSuggestedBounds;
}

void MToplevel::suggestedSizeChanged()
{
    if (suggestedSize().width() == 0)
    {
        if (layout().width().value == 0.f || layout().width().value == YGUndefined)
            layout().setWidthAuto();
    }
    else
        layout().setWidth(suggestedSize().width());

    if (suggestedSize().height() == 0)
    {
        if (layout().height().value == 0.f || layout().height().value == YGUndefined)
            layout().setHeightAuto();
    }
    else
        layout().setHeight(suggestedSize().height());

    onSuggestedSizeChanged.notify();
}

void MToplevel::suggestedBoundsChanged()
{
    onSuggestedSizeChanged.notify();
}

AKBitset<AKWindowState> MToplevel::states() const noexcept
{
    return imp()->currentStates;
}

void MToplevel::setTitle(const std::string &title)
{
    if (this->title() == title)
        return;

    xdg_toplevel_set_title(imp()->xdgToplevel, title.c_str());
    imp()->title = title;
    onTitleChanged.notify();
}

const std::string &MToplevel::title() const noexcept
{
    return imp()->title;
}

const SkIRect &MToplevel::builtinDecorationMargins() const noexcept
{
    return imp()->shadowMargins;
}

const SkIRect &MToplevel::decorationMargins() const noexcept
{
    return imp()->userDecorationMargins;
}

void MToplevel::setDecorationMargins(const SkIRect &margins) noexcept
{
    SkIRect m { margins };

    if (m.fLeft < 0) m.fLeft = 0;
    if (m.fTop < 0) m.fTop = 0;
    if (m.fRight < 0) m.fRight = 0;
    if (m.fBottom < 0) m.fBottom = 0;

    if (imp()->userDecorationMargins == margins)
        return;

    imp()->userDecorationMargins = margins;

    if (decorationMode() == ClientSide && !builtinDecorationsEnabled())
        update();

    decorationMarginsChanged();
}

AKBitset<MToplevel::WMCapabilities> MToplevel::wmCapabilities() const noexcept
{
    return imp()->currentWMCaps;
}

bool MToplevel::showWindowMenu(const AKInputEvent &event, const SkIPoint &pos) noexcept
{
    if (!wmCapabilities().check(WindowMenuCap))
        return false;

    xdg_toplevel_show_window_menu(imp()->xdgToplevel, app()->wayland().seat, event.serial(), pos.x(), pos.y());
    return true;
}

MToplevel::DecorationMode MToplevel::decorationMode() const noexcept
{
    return imp()->currentDecorationMode;
}

void MToplevel::setDecorationMode(DecorationMode mode) noexcept
{
    if (mode == ClientSide)
    {
        if (imp()->xdgDecoration)
        {
            zxdg_toplevel_decoration_v1_destroy(imp()->xdgDecoration);
            imp()->xdgDecoration = nullptr;
            update();
        }

        imp()->pendingDecorationMode = ClientSide;

        if (imp()->pendingDecorationMode != imp()->currentDecorationMode)
        {
            imp()->currentDecorationMode = ClientSide;
            decorationModeChanged();
        }
    }
    else
    {
        if (!app()->wayland().xdgDecorationManager)
            return;

        const bool wasMapped { mapped() };

        if (!imp()->xdgDecoration)
        {
            if (wasMapped)
                imp()->unmap();

            imp()->xdgDecoration = zxdg_decoration_manager_v1_get_toplevel_decoration(app()->wayland().xdgDecorationManager, imp()->xdgToplevel);
            zxdg_toplevel_decoration_v1_add_listener(imp()->xdgDecoration, &imp()->xdgDecorationListener, this);
            zxdg_toplevel_decoration_v1_set_mode(imp()->xdgDecoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);

            if (wasMapped)
                setMapped(true);
        }
        else
        {
            zxdg_toplevel_decoration_v1_set_mode(imp()->xdgDecoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
        }

        update();
    }
}

bool MToplevel::builtinDecorationsEnabled() const noexcept
{
    return MSurface::imp()->flags.check(MSurface::Imp::BuiltinDecorations);
}

void MToplevel::enableBuiltinDecorations(bool enabled) noexcept
{
    if (builtinDecorationsEnabled() == enabled)
        return;

    MSurface::imp()->flags.setFlag(MSurface::Imp::BuiltinDecorations, enabled);

    if (decorationMode() == ClientSide)
        update(true);
}

bool MToplevel::setParentToplevel(MToplevel *parent) noexcept
{
    if (parent == parentToplevel())
        return true;

    if (parent)
    {
        MToplevel *tl = parent;

        while (tl)
        {
            if (tl == this) return false;
            tl = tl->parentToplevel();
        }

        if (imp()->parentToplevel)
            imp()->parentToplevel->imp()->childToplevels.erase(this);

        // TODO: handle later
        imp()->parentToplevel.reset(parent);
        parent->imp()->childToplevels.insert(this);
        parent->MSurface::imp()->flags.add(MSurface::Imp::PendingChildren);
        MSurface::imp()->flags.add(MSurface::Imp::PendingParent);
        update();
    }
    else
    {
        if (imp()->parentToplevel)
            imp()->parentToplevel->imp()->childToplevels.erase(this);

        imp()->parentToplevel.reset();
        MSurface::imp()->flags.add(MSurface::Imp::PendingParent);
        update();
    }

    return true;
}

MToplevel *MToplevel::parentToplevel() const noexcept
{
    return imp()->parentToplevel;
}

const std::unordered_set<AK::MToplevel *> &MToplevel::childToplevels() const noexcept
{
    return imp()->childToplevels;
}

const std::unordered_set<MPopup *> &MToplevel::childPopups() const noexcept
{
    return imp()->childPopups;
}

void MToplevel::wmCapabilitiesChanged()
{
    onWMCapabilitiesChanged.notify();
}

void MToplevel::decorationModeChanged()
{
    onDecorationModeChanged.notify();
}

void MToplevel::decorationMarginsChanged()
{

}

void MToplevel::windowStateEvent(const AKWindowStateEvent &event)
{
    MSurface::windowStateEvent(event);
    onStatesChanged.notify(event);
    event.accept();
}

void MToplevel::pointerButtonEvent(const AKPointerButtonEvent &event)
{
    if (event.state() == AKPointerButtonEvent::Pressed)
        for (auto &popup : childPopups())
            popup->setMapped(false);

    MSurface::pointerButtonEvent(event);
}

bool MToplevel::eventFilter(const AKEvent &event, AKObject &object)
{
    if (rootNode() == &object)
    {
        if (event.type() == AKEvent::PointerButton)
            imp()->handleRootPointerButtonEvent(static_cast<const AKPointerButtonEvent&>(event));
        else if (event.type() == AKEvent::PointerMove)
            imp()->handleRootPointerMoveEvent(static_cast<const AKPointerMoveEvent&>(event));
    }

   return MSurface::eventFilter(event, object);
}

bool MToplevel::event(const AKEvent &e)
{
    if (e.type() == AKEvent::WindowClose)
    {
        onBeforeClose.notify((const AKWindowCloseEvent &)e);
        return e.isAccepted();
    }

    return MSurface::event(e);
}

void MToplevel::onUpdate() noexcept
{
    auto &flags = MSurface::imp()->flags;
    using SF = MSurface::Imp::Flags;
    auto &tmpFlags = MSurface::imp()->tmpFlags;
    using STF = MSurface::Imp::TmpFlags;

    MSurface::onUpdate();

    if (!flags.check(SF::UserMapped))
    {
        if (flags.check(SF::PendingNullCommit))
            return;

        if (mapped())
            imp()->unmap();

        return;
    }

    if (flags.check(SF::PendingNullCommit))
    {
        if (fullscreen())
            xdg_toplevel_set_fullscreen(imp()->xdgToplevel, nullptr);

        if (maximized())
            xdg_toplevel_set_maximized(imp()->xdgToplevel);

        xdg_toplevel_set_app_id(imp()->xdgToplevel, app()->appId().c_str());
        xdg_toplevel_set_title(imp()->xdgToplevel, imp()->title.c_str());

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
        xdg_surface_ack_configure(imp()->xdgSurface, imp()->configureSerial);
    }

    if (flags.check(SF::PendingFirstConfigure | SF::PendingNullCommit))
        return;

    if (tmpFlags.check(STF::ScaleChanged))
        flags.add(SF::ForceUpdate);

    if (states().check(AKMaximized | AKFullscreen) || decorationMode() == ServerSide || !builtinDecorationsEnabled())
    {
        if (decorationMode() == ServerSide || builtinDecorationsEnabled())
            imp()->setShadowMargins({ 0, 0, 0, 0 });
        else
            imp()->setShadowMargins(imp()->userDecorationMargins);

        imp()->shadow.setVisible(false);
        for (int i = 0; i < 4; i++)
            imp()->borderRadius[i].setVisible(false);
    }
    else
    {
        imp()->shadow.setVisible(true);
        for (int i = 0; i < 4; i++)
            imp()->borderRadius[i].setVisible(true);

        if (activated())
        {
            imp()->setShadowMargins({
                app()->theme()->CSDShadowActiveRadius,
                app()->theme()->CSDShadowActiveRadius - app()->theme()->CSDShadowActiveOffsetY,
                app()->theme()->CSDShadowActiveRadius,
                app()->theme()->CSDShadowActiveRadius + app()->theme()->CSDShadowActiveOffsetY,
            });
        }
        else
        {
            imp()->setShadowMargins({
                app()->theme()->CSDShadowInactiveRadius,
                app()->theme()->CSDShadowInactiveRadius - app()->theme()->CSDShadowInactiveOffsetY,
                app()->theme()->CSDShadowInactiveRadius,
                app()->theme()->CSDShadowInactiveRadius + app()->theme()->CSDShadowInactiveOffsetY,
            });
        }

        imp()->borderRadius[0].layout().setPosition(YGEdgeLeft, imp()->shadowMargins.fLeft);
        imp()->borderRadius[0].layout().setPosition(YGEdgeTop, imp()->shadowMargins.fTop);
        imp()->borderRadius[1].layout().setPosition(YGEdgeRight, imp()->shadowMargins.fRight);
        imp()->borderRadius[1].layout().setPosition(YGEdgeTop, imp()->shadowMargins.fTop);
        imp()->borderRadius[2].layout().setPosition(YGEdgeRight, imp()->shadowMargins.fRight);
        imp()->borderRadius[2].layout().setPosition(YGEdgeBottom, imp()->shadowMargins.fBottom);
        imp()->borderRadius[3].layout().setPosition(YGEdgeLeft, imp()->shadowMargins.fLeft);
        imp()->borderRadius[3].layout().setPosition(YGEdgeBottom, imp()->shadowMargins.fBottom);
    }

    layout().setPosition(YGEdgeLeft, 0.f);
    layout().setPosition(YGEdgeTop, 0.f);
    layout().setMargin(YGEdgeLeft, imp()->shadowMargins.fLeft);
    layout().setMargin(YGEdgeTop, imp()->shadowMargins.fTop);
    layout().setMargin(YGEdgeRight, imp()->shadowMargins.fRight);
    layout().setMargin(YGEdgeBottom, imp()->shadowMargins.fBottom);

    render();
}

std::vector<std::string> splitString(const std::string& input, size_t chunkSize) {
    std::vector<std::string> chunks;

    for (size_t i = 0; i < input.size(); i += chunkSize) {
        // Create substrings of chunkSize
        chunks.push_back(input.substr(i, chunkSize));
    }

    return chunks;
}

void MToplevel::render() noexcept
{
    scene().root()->layout().calculate();

    if (wlCallback() && !MSurface::imp()->flags.check(MSurface::Imp::ForceUpdate))
        return;

    bool repaint { false };

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

    if (states().check(AKResizing))
    {
        eglWindowSize = bufferSize();
        eglWindowSize.fWidth /= scale();
        eglWindowSize.fHeight /= scale();

        if (eglWindowSize.width() < newSize.width())
            eglWindowSize.fWidth = newSize.width() * 1.75f;

        if (eglWindowSize.height() < newSize.height())
            eglWindowSize.fHeight = newSize.height() * 1.75f;
    }

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
            imp()->shadowMargins.fLeft,
            imp()->shadowMargins.fTop,
            layout().calculatedWidth(),
            layout().calculatedHeight());

        if (MSurface::imp()->backgroundBlur && app()->wayland().svgPathManager)
        {
            SkPath path = SkPath::RRect(
                SkRect::MakeXYWH(
                    imp()->shadowMargins.fLeft,
                    imp()->shadowMargins.fTop,
                    layout().calculatedWidth(),
                    layout().calculatedHeight()),
                MTheme::CSDBorderRadius,
                MTheme::CSDBorderRadius);

            auto str = std::string(SkParsePath::ToSVGString(path).c_str());
            svg_path *svg { svg_path_manager_get_svg_path(app()->wayland().svgPathManager) };

            size_t sent { 0 };
            size_t toWrite;
            char *start;
            char c;
            while (sent < str.size())
            {
                start = (char*)&str.c_str()[sent];
                toWrite = std::min(str.size() - sent, 1024UL);
                c = start[toWrite];
                start[toWrite] = '\0';
                svg_path_concat_commands(svg, start);
                start[toWrite] = c;
                sent += toWrite;
            }

            svg_path_done(svg);
            background_blur_set_path(MSurface::imp()->backgroundBlur, svg);
            svg_path_destroy(svg);
        }
    }

    repaint |= target()->isDirty() || target()->bakedComponentsScale() != scale();

    if (!repaint)
    {
        imp()->applyPendingParent();
        imp()->applyPendingChildren();
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

    /* CSD */
    if (decorationMode() == ClientSide && builtinDecorationsEnabled())
        for (int i = 0; i < 4; i++)
            imp()->borderRadius[i].setImage(app()->theme()->csdBorderRadiusMask(scale()));

    /*
    glScissor(0, 0, 1000000, 100000);
    glViewport(0, 0, 1000000, 100000);
    glClear(GL_COLOR_BUFFER_BIT);*/

    scene().render(target());

    if (decorationMode() == ClientSide && builtinDecorationsEnabled())
        for (int i = 0; i < 4; i++)
            target()->outOpaqueRegion->op(imp()->borderRadius[i].globalRect(), SkRegion::Op::kDifference_Op);

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

    if (decorationMode() == ClientSide && builtinDecorationsEnabled())
    {
        SkIRect opaqueClip { globalRect() };
        opaqueClip.inset(1, 1);
        skOpaque.op(opaqueClip, SkRegion::Op::kIntersect_Op);
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

    /*
    if (states().check(Resizing) && callbackMsDiff < 4)
    {
        wl_display_flush(Marco::app()->wayland().display);
        usleep((4 - callbackMsDiff) * 1000);
        AKLog::debug("Sleeping %d ms", (4 - callbackMsDiff));
    }*/

    //eglSwapBuffers(app()->graphics().eglDisplay, m_eglSurface);

    imp()->applyPendingParent();
    imp()->applyPendingChildren();
}

MToplevel::Imp *MToplevel::imp() const noexcept
{
    return m_imp.get();
}
