#include <AK/AKGLContext.h>
#include <AK/AKColors.h>
#include <AK/AKLog.h>

#include <Marco/private/MSurfacePrivate.h>
#include <Marco/MApplication.h>
#include <Marco/roles/MSubsurface.h>
#include <Marco/nodes/MVibrancyView.h>

using namespace AK;

MSurface::MSurface(Role role) noexcept : AK::AKSolidColor(SK_ColorWHITE)
{
    m_imp = std::make_unique<Imp>(*this);
    imp()->role = role;
    imp()->createSurface();

    imp()->appLink = app()->m_surfaces.size();
    app()->m_surfaces.push_back(this);
    setParent(rootNode());
    enableChildrenClipping(true);
    imp()->target.reset(scene().createTarget());
    scene().setRoot(rootNode());

    target()->on.markedDirty.subscribe(this, [this](AK::AKSceneTarget&){
        update();
    });

    app()->onScreenUnplugged.subscribe(this, [this](MScreen &screen){
        Imp::wl_surface_leave(this, wlSurface(), screen.wlOutput());
    });
}

MSurface::~MSurface()
{
    while (!subSurfaces().empty())
        subSurfaces().back()->setParent(nullptr);

    app()->m_surfaces[imp()->appLink] = app()->m_surfaces.back();
    app()->m_surfaces.pop_back();
    scene().destroyTarget(target());
    imp()->resizeBuffer({0, 0});

    if (wlCallback())
    {
        wl_callback_destroy(wlCallback());
        imp()->wlCallback = nullptr;
    }

    if (imp()->invisibleRegion)
    {
        invisible_region_destroy(imp()->invisibleRegion);
        imp()->invisibleRegion = nullptr;
    }

    if (wlSurface())
    {
        wl_surface_destroy(wlSurface());
        imp()->wlSurface = nullptr;
    }
}

MSurface::Role MSurface::role() noexcept
{
    return imp()->role;
}

Int32 MSurface::scale() noexcept
{
    return imp()->scale;
}

const SkISize &MSurface::surfaceSize() const noexcept
{
    if (imp()->viewportSize.width() >= 0)
        return imp()->viewportSize;

    return imp()->size;
}

const SkISize &MSurface::bufferSize() const noexcept
{
    return imp()->bufferSize;
}

const std::set<MScreen *> &MSurface::screens() const noexcept
{
    return imp()->screens;
}

void MSurface::setMapped(bool mapped) noexcept
{
    if (imp()->flags.check(Imp::UserMapped) == mapped)
        return;

    imp()->flags.setFlag(Imp::UserMapped, mapped);
    update();
}

bool MSurface::mapped() const noexcept
{
    return imp()->flags.check(Imp::Mapped);
}

void MSurface::update(bool force) noexcept
{
    imp()->flags.add(Imp::PendingUpdate);

    if (force)
        imp()->flags.add(Imp::ForceUpdate);

    app()->update();
}

SkISize MSurface::minContentSize() noexcept
{
    const YGValue width { layout().width() };
    const YGValue height { layout().height() };
    layout().setWidthAuto();
    layout().setHeightAuto();
    rootNode()->layout().calculate();
    const SkISize contentSize { SkISize::Make(layout().calculatedWidth(), layout().calculatedHeight()) };
    layout().setWidthYGValue(width);
    layout().setHeightYGValue(height);
    rootNode()->layout().calculate();
    return contentSize;
}

const std::list<MSubsurface *> &MSurface::subSurfaces() const noexcept
{
    return imp()->subSurfaces;
}

AKScene &MSurface::scene() const noexcept
{
    return imp()->scene;
}

AKSceneTarget *MSurface::target() const noexcept
{
    return imp()->target.get();
}

AKNode *MSurface::rootNode() const noexcept
{
    return &imp()->root;
}

wl_surface *MSurface::wlSurface() const noexcept
{
    return imp()->wlSurface;
}

wl_callback *MSurface::wlCallback() const noexcept
{
    return imp()->wlCallback;
}

wp_viewport *MSurface::wlViewport() const noexcept
{
    return imp()->wlViewport;
}

invisible_region *MSurface::wlInvisibleRegion() const noexcept
{
    return imp()->invisibleRegion;
}

wl_egl_window *MSurface::eglWindow() const noexcept
{
    return imp()->eglWindow;
}

sk_sp<SkSurface> MSurface::skSurface() const noexcept
{
    return imp()->skSurface;
}

EGLSurface MSurface::eglSurface() const noexcept
{
    return imp()->eglSurface;
}

AKVibrancyState MSurface::vibrancyState() const noexcept
{
    return imp()->currentVibrancyState;
}

AKVibrancyStyle MSurface::vibrancyStyle() const noexcept
{
    return imp()->currentVibrancyStyle;
}

void MSurface::vibrancyEvent(const AKVibrancyEvent &event)
{
    AKSafeEventQueue queue;
    AKNode::Iterator it { bottommostLeftChild() };

    while (!it.done())
    {
        if (it.node() != this)
            queue.addEvent((const AKEvent&)event, (AKObject&)(*it.node()));

        it.next();
    }

    queue.dispatch();
    onVibrancyChanged.notify(event);
}

void MSurface::onUpdate() noexcept
{
    if (imp()->tmpFlags.check(Imp::PreferredScaleChanged))
    {
        imp()->scale = imp()->preferredBufferScale;
        imp()->tmpFlags.add(Imp::ScaleChanged);
    }
    else if (imp()->preferredBufferScale == -1 && imp()->tmpFlags.check(Imp::ScreensChanged))
    {
        Int32 maxScale { 1 };

        for (const auto &screen : imp()->screens)
            if (screen->props().scale > maxScale)
                maxScale = screen->props().scale;

        if (maxScale != scale())
        {
            imp()->scale = maxScale;
            imp()->tmpFlags.add(Imp::ScaleChanged);
        }
    }
}

bool MSurface::event(const AKEvent &event)
{
    if (event.type() == AKEvent::Vibrancy)
    {
        vibrancyEvent((const AKVibrancyEvent&)event);
        return true;
    }

    return AKSolidColor::event(event);
}

MSurface::Imp *MSurface::imp() const noexcept
{
    return m_imp.get();
}
