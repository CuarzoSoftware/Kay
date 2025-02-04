#include <include/gpu/GrBackendSurface.h>
#include <include/gpu/GrDirectContext.h>
#include <include/core/SkSurface.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/core/SkColorSpace.h>
#include <include/utils/SkParsePath.h>

#include <wayland-client.h>
#include <wayland-egl.h>
#include <xdg-shell-client-protocol.h>
#include <xdg-decoration-unstable-v1-client-protocol.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <cassert>
#include <cstring>
#include <linux/input-event-codes.h>

#include <AK/AKApplication.h>
#include <AK/AKScene.h>
#include <AK/AKGLContext.h>
#include <AK/effects/AKBackgroundBoxShadowEffect.h>
#include <AK/effects/AKBackgroundImageShadowEffect.h>
#include <AK/nodes/AKRoundContainer.h>
#include <AK/nodes/AKContainer.h>
#include <AK/nodes/AKSolidColor.h>
#include <AK/nodes/AKSimpleText.h>
#include <AK/nodes/AKPath.h>

using namespace AK;

static SkSurfaceProps skSurfaceProps(0, kUnknown_SkPixelGeometry);

static const std::string message { "Hello World!" };
static const std::string starSVG { "M3.612 15.443c-.386.198-.824-.149-.746-.592l.83-4.73L.173 6.765c-.329-.314-.158-.888.283-.95l4.898-.696L7.538.792c.197-.39.73-.39.927 0l2.184 4.327 4.898.696c.441.062.612.636.282.95l-3.522 3.356.83 4.73c.078.443-.36.79-.746.592L8 13.187l-4.389 2.256z" };
static const std::string happySVG { "M8 16A8 8 0 1 0 8 0a8 8 0 0 0 0 16M7 6.5C7 7.328 6.552 8 6 8s-1-.672-1-1.5S5.448 5 6 5s1 .672 1 1.5M4.285 9.567a.5.5 0 0 1 .683.183A3.5 3.5 0 0 0 8 11.5a3.5 3.5 0 0 0 3.032-1.75.5.5 0 1 1 .866.5A4.5 4.5 0 0 1 8 12.5a4.5 4.5 0 0 1-3.898-2.25.5.5 0 0 1 .183-.683M10 8c-.552 0-1-.672-1-1.5S9.448 5 10 5s1 .672 1 1.5S10.552 8 10 8" };
static const std::string heartSVG { "M4 1c2.21 0 4 1.755 4 3.92C8 2.755 9.79 1 12 1s4 1.755 4 3.92c0 3.263-3.234 4.414-7.608 9.608a.513.513 0 0 1-.784 0C3.234 9.334 0 8.183 0 4.92 0 2.755 1.79 1 4 1" };
static struct
{
    AKApplication ak;
    wl_display *wlDisplay;
    wl_registry *wlRegistry;
    wl_compositor *wlCompositor { nullptr };
    xdg_wm_base *xdgWmBase { nullptr };
    zxdg_decoration_manager_v1 *xdgDecorationManager { nullptr };
    wl_seat *wlSeat { nullptr };
    wl_pointer *wlPointer { nullptr };
    wl_keyboard *wlKeyboard { nullptr };

    EGLDisplay eglDisplay;
    EGLConfig eglConfig;
    EGLContext eglContext;

    // TODO: Check extension
    PFNEGLSWAPBUFFERSWITHDAMAGEKHRPROC eglSwapBuffersWithDamageKHR;
} app;

class Topbar : public AKSolidColor
{
public:
    Topbar(AKNode *parent) noexcept : AKSolidColor(0xFFEEEEEE, parent)
    {
        SkFont font = title.font();
        font.setEmbolden(true);
        font.setSize(32);
        title.setFont(font);
        title.layout().setMargin(YGEdgeAll, 16);
        enableChildrenClipping(true);
        layout().setHeightAuto();
        layout().setWidthPercent(100);
        layout().setDisplay(YGDisplayFlex);
        layout().setJustifyContent(YGJustifyCenter);
        layout().setAlignItems(YGAlignCenter);
    }

    AKBackgroundBoxShadowEffect shadow {
        32.f, { 0, 0 }, SkColorSetARGB(250, 0, 0, 0),
        true, this };

    AKSimpleText title { "Hello Kay!", this };
    AKBackgroundImageShadowEffect titleShadow { 8, {4, 4}, 0x88000000, &title };
};

struct Window
{
    Window() noexcept;
    void update() noexcept;

    AKScene scene;
    AKContainer root;
    Topbar topbar { &root };
    AKContainer bottom { YGFlexDirectionRow, true, &root };
    AKRoundContainer roundContainer { AKBorderRadius::Make(16), &bottom };
    AKSolidColor redBackground { 0xFE444444, &roundContainer };
    AKPath heart { SkPath(), &redBackground };
    AKPath star { SkPath(), &redBackground };
    AKPath happy { SkPath(), &redBackground };

    AKBackgroundBoxShadowEffect roundContainerShadow {10,
                                            {0,0}, SK_ColorBLACK,
                                            false, &roundContainer};

    AKTarget *target { nullptr };

    wl_callback *wlCallback { nullptr };
    wl_surface *wlSurface;
    xdg_surface *xdgSurface;
    xdg_toplevel *xdgToplevel;
    zxdg_toplevel_decoration_v1 *xdgToplevelDecoration;
    wl_egl_window *wlEGLWindow { nullptr };
    EGLSurface eglSurface;
    sk_sp<SkSurface> skSurface;

    SkISize size;
    SkISize bufferSize;
    int32_t scale { 1 };
    bool needsNewSurface { true };
};

static wl_surface_listener wlSurfaceLis
{
    .enter = [](auto, auto, auto){},
    .leave = [](auto, auto, auto){},
    .preferred_buffer_scale = [](void *data, wl_surface */*wlSurface*/, int32_t factor)
    {
        Window &window { *static_cast<Window*>(data) };

        if (factor == window.scale)
            return;

        window.scale = factor;
        window.needsNewSurface = true;
    },
    .preferred_buffer_transform = [](auto, auto, auto){}
};

static xdg_surface_listener xdgSurfaceLis
{
    .configure = [](void *data, xdg_surface *xdgSurface, uint32_t serial)
    {
        Window &window { *static_cast<Window*>(data) };
        xdg_surface_ack_configure(xdgSurface, serial);
        window.update();
    }
};

static xdg_toplevel_listener xdgToplevelLis
{
    .configure = [](void *data, xdg_toplevel */*xdgToplevel*/, int32_t width, int32_t height, wl_array */*states*/)
    {
        Window &window { *static_cast<Window*>(data) };
        if (width < 256) width = 256;
        if (height < 256) height = 256;
        if (window.size.width() != width || window.size.height() != height)
        {
            window.size = { width, height };
            window.needsNewSurface = true;
        }
    },
    .close = [](auto, auto){ exit(0); },
    .configure_bounds = [](auto, auto, auto, auto){},
    .wm_capabilities = [](auto, auto, auto){}
};

Window::Window() noexcept
{
    //roundContainerShadow.setBorderRadius({10, 0, 0, 0});
    roundContainerShadow.setOffset(0, 4);
    roundContainerShadow.setBorderRadius(roundContainer.borderRadius().corners());
    root.layout().setDirection(YGDirectionRTL);
    bottom.layout().setFlex(1);
    bottom.layout().setWidthAuto();
    bottom.layout().setHeightAuto();
    bottom.layout().setPadding(YGEdgeAll, 48.f);
    roundContainer.layout().setWidthPercent(100);
    roundContainer.layout().setHeightPercent(100);
    redBackground.layout().setWidthPercent(100);
    redBackground.layout().setHeightPercent(100);
    redBackground.layout().setDisplay(YGDisplayFlex);
    redBackground.layout().setFlexDirection(YGFlexDirectionRow);
    redBackground.layout().setAlignItems(YGAlignCenter);
    redBackground.layout().setJustifyContent(YGJustifySpaceEvenly);

    SkPath path;
    const SkScalar size { 32.f };
    SkParsePath::FromSVGString(starSVG.c_str(), &path);
    star.setPath(path);
    star.layout().setWidth(size);
    star.layout().setHeight(size);
    star.setColorWithAlpha(SkColors::kWhite);
    star.enableCustomBlendFunc(true);
    star.setCustomBlendFunc({
        .sRGBFactor = GL_ZERO,
        .dRGBFactor = GL_ONE_MINUS_SRC_ALPHA,
        .sAlphaFactor = GL_ZERO,
        .dAlphaFactor = GL_ONE_MINUS_SRC_ALPHA,
    });

    SkParsePath::FromSVGString(happySVG.c_str(), &path);
    happy.setPath(path);
    happy.layout().setWidth(size);
    happy.layout().setHeight(size);
    happy.setColorWithAlpha(SkColors::kGreen);

    SkParsePath::FromSVGString(heartSVG.c_str(), &path);
    heart.setPath(path);
    heart.layout().setWidth(size);
    heart.layout().setHeight(size);
    heart.setColorWithAlpha(SkColors::kRed);

    wlSurface = wl_compositor_create_surface(app.wlCompositor);
    wl_surface_add_listener(wlSurface, &wlSurfaceLis, this);
    xdgSurface = xdg_wm_base_get_xdg_surface(app.xdgWmBase, wlSurface);
    xdg_surface_add_listener(xdgSurface, &xdgSurfaceLis, this);
    xdgToplevel = xdg_surface_get_toplevel(xdgSurface);
    xdg_toplevel_add_listener(xdgToplevel, &xdgToplevelLis, this);

    if (app.xdgDecorationManager)
        xdgToplevelDecoration = zxdg_decoration_manager_v1_get_toplevel_decoration(app.xdgDecorationManager, xdgToplevel);

    wl_surface_commit(wlSurface);
}

static wl_callback_listener wlCallbackLis
{
    .done = [](void *data, wl_callback *wlCallback, uint32_t)
    {
        Window &window { *static_cast<Window*>(data) };
        window.wlCallback = nullptr;
        window.update();
        wl_callback_destroy(wlCallback);
    }
};

void Window::update() noexcept
{
    bufferSize = { size.width() * scale, size.height() * scale };

    if (!wlEGLWindow)
    {
        wlEGLWindow = wl_egl_window_create(wlSurface, bufferSize.width(), bufferSize.height());
        eglSurface = eglCreateWindowSurface(app.eglDisplay, app.eglConfig, wlEGLWindow, NULL);
        assert("Failed to create EGLSurface" && eglSurface != EGL_NO_SURFACE);
        eglMakeCurrent(app.eglDisplay, eglSurface, eglSurface, app.eglContext);
        eglSwapInterval(app.eglDisplay, 0);
    }

    if (needsNewSurface)
    {
        needsNewSurface = false;
        wl_egl_window_resize(wlEGLWindow, bufferSize.width(), bufferSize.height(), 0, 0);

        const GrGLFramebufferInfo fbInfo
        {
            .fFBOID = 0,
            .fFormat = GL_RGBA8_OES
        };

        const GrBackendRenderTarget backendTarget(
            bufferSize.width(),
            bufferSize.height(),
            0, 0,
            fbInfo);

        skSurface = SkSurfaces::WrapBackendRenderTarget(
            AKApp()->glContext()->skContext().get(),
            backendTarget,
            GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,
            SkColorType::kRGBA_8888_SkColorType,
            SkColorSpace::MakeSRGB(),
            &skSurfaceProps);

        assert("Failed to create SkSurface" && skSurface.get());
    }

    EGLint bufferAge;
    eglQuerySurface(app.eglDisplay, eglSurface, EGL_BUFFER_AGE_EXT, &bufferAge);

    root.layout().setWidth(size.width());
    root.layout().setHeight(size.height());
    static float phase = 0;
    phase += 0.01f;
    //topbar.layout().setHeight(0.5f * size.height() * SkScalarAbs(SkScalarCos(phase)));
    topbar.title.setColorWithAlpha({SkScalarAbs(SkScalarCos(phase)), SkScalarAbs(SkScalarSin(phase)), 1.f, 1.f});
    topbar.title.setText(message.substr(0, SkScalarAbs(SkScalarSin(phase)) * (message.size() +  1)));

    if (!target)
        target = scene.createTarget();

    SkRegion damage, opaque;
    target->setClearColor(SkColorSetARGB(255, 255, 50, 255 * SkScalarAbs(SkScalarCos(phase * 5.f))));
    target->outDamageRegion = &damage;
    target->outOpaqueRegion = &opaque;
    target->setSurface(skSurface);
    target->setTransform(AKTransform::Normal);
    target->setViewport(SkRect::MakeSize(SkSize::Make(size)));
    target->setDstRect(SkIRect::MakeSize(bufferSize));
    target->setAge(bufferAge);
    target->setBakedComponentsScale(scale);
    //std::cout << "Buffer age: " << bufferAge << std::endl;
    scene.setRoot(&root);
    scene.render(target);

    wl_region *wlOpaqueRegion = wl_compositor_create_region(app.wlCompositor);
    SkRegion::Iterator it(opaque);
    while (!it.done())
    {
        wl_region_add(wlOpaqueRegion, it.rect().x(), it.rect().y(), it.rect().width(), it.rect().height());
        it.next();
    }
    wl_surface_set_opaque_region(wlSurface, wlOpaqueRegion);
    wl_region_destroy(wlOpaqueRegion);

    wl_region *wlInputRegion = wl_compositor_create_region(app.wlCompositor);
    wl_region_add(wlInputRegion, 0, 0, 100000, 100000);
    wl_surface_set_input_region(wlSurface, wlInputRegion);
    wl_region_destroy(wlInputRegion);

    wl_surface_set_buffer_scale(wlSurface, scale);

    if (!wlCallback)
    {
        //std::cout << "wl_surface::frame" << std::endl;
        wlCallback = wl_surface_frame(wlSurface);
        wl_callback_add_listener(wlCallback, &wlCallbackLis ,this);
    }

    if (damage.computeRegionComplexity() > 0)
    {
        EGLint *damageRects { new EGLint[damage.computeRegionComplexity() * 4] };
        EGLint *rectsIt = damageRects;
        SkRegion::Iterator damageIt(damage);
        while (!damageIt.done())
        {
            //std::cout << "DAMAGE " << damageIt.rect().x() << "," << damageIt.rect().y() << "," << damageIt.rect().width() << "," << damageIt.rect().height() << std::endl;
            *rectsIt = damageIt.rect().x() * scale;
            rectsIt++;
            *rectsIt = (size.height() - damageIt.rect().height() - damageIt.rect().y()) * scale;
            rectsIt++;
            *rectsIt = damageIt.rect().width() * scale;
            rectsIt++;
            *rectsIt = damageIt.rect().height() * scale;
            rectsIt++;
            damageIt.next();
        }

        assert(app.eglSwapBuffersWithDamageKHR(app.eglDisplay, eglSurface, damageRects, damage.computeRegionComplexity()) == EGL_TRUE);
        delete []damageRects;
    }
    else
        wl_surface_commit(wlSurface);
}

static xdg_wm_base_listener xdgWmBaseLis
{
    .ping = [](void */*data*/, xdg_wm_base *xdgWmBase, uint32_t serial) { xdg_wm_base_pong(xdgWmBase, serial); }
};

static wl_keyboard_listener wlKeyboardLis
{
    .keymap = [](auto, auto, auto, auto, auto){},
    .enter = [](auto, auto, auto, auto, auto){},
    .leave = [](auto, auto, auto, auto){},
    .key = [](void */*data*/, wl_keyboard */*wlKeyboard*/, uint32_t /*serial*/, uint32_t /*time*/, uint32_t /*key*/, uint32_t /*state*/)
    {
        exit(0);
    },
    .modifiers = [](auto, auto, auto, auto, auto, auto, auto){},
    .repeat_info = [](auto, auto, auto, auto){}
};

static wl_seat_listener wlSeatLis
{
    .capabilities = [](void */*data*/, struct wl_seat *wlSeat, uint32_t capabilities)
    {
        if (!app.wlPointer && (capabilities & WL_SEAT_CAPABILITY_POINTER))
            app.wlPointer = wl_seat_get_pointer(wlSeat);
        else if (!app.wlKeyboard && (capabilities & WL_SEAT_CAPABILITY_KEYBOARD))
        {
            app.wlKeyboard = wl_seat_get_keyboard(wlSeat);
            wl_keyboard_add_listener(app.wlKeyboard, &wlKeyboardLis, NULL);
        }
    },
    .name = [](auto, auto, auto){}
};

static wl_registry_listener wlRegistryLis
{
    .global = [](void */*data*/, wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version)
    {
        if (version >= 6 && !app.wlCompositor && strcmp(interface, wl_compositor_interface.name) == 0)
            app.wlCompositor = (wl_compositor*)wl_registry_bind(wl_registry, name, &wl_compositor_interface, 6);
        if (!app.wlSeat && strcmp(interface, wl_seat_interface.name) == 0)
        {
            app.wlSeat = (wl_seat*)wl_registry_bind(wl_registry, name, &wl_seat_interface, 1);
            wl_seat_add_listener(app.wlSeat, &wlSeatLis, NULL);
        }
        else if (!app.xdgWmBase && strcmp(interface, xdg_wm_base_interface.name) == 0)
        {
            app.xdgWmBase = (xdg_wm_base*)wl_registry_bind(wl_registry, name, &xdg_wm_base_interface, 1);
            xdg_wm_base_add_listener(app.xdgWmBase, &xdgWmBaseLis, NULL);
        }
        else if (!app.xdgDecorationManager && strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0)
        {
            app.xdgDecorationManager = (zxdg_decoration_manager_v1*)wl_registry_bind(wl_registry, name, &zxdg_decoration_manager_v1_interface, 1);
        }
    },
    .global_remove = [](auto, auto, auto){}
};

static void initEGL() noexcept
{
    app.eglDisplay = eglGetDisplay(app.wlDisplay);

    assert("Failed to create EGLDisplay" && app.eglDisplay != EGL_NO_DISPLAY);
    assert("Failed to initialize EGLDisplay." && eglInitialize(app.eglDisplay, NULL, NULL) == EGL_TRUE);
    assert("Failed to bind GL_OPENGL_ES_API." && eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);

    EGLint numConfigs;
    assert("Failed to get EGL configurations." &&
           eglGetConfigs(app.eglDisplay, NULL, 0, &numConfigs) == EGL_TRUE && numConfigs > 0);

    const EGLint fbAttribs[]
    {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_NONE
    };

    assert("Failed to choose EGL configuration." &&
           eglChooseConfig(app.eglDisplay, fbAttribs, &app.eglConfig, 1, &numConfigs) == EGL_TRUE && numConfigs == 1);

    const EGLint contextAttribs[] { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };
    app.eglContext = eglCreateContext(app.eglDisplay, app.eglConfig, EGL_NO_CONTEXT, contextAttribs);
    assert("Failed to create EGL context." && app.eglContext != EGL_NO_CONTEXT);
    eglMakeCurrent(app.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, app.eglContext);
    app.eglSwapBuffersWithDamageKHR = (PFNEGLSWAPBUFFERSWITHDAMAGEKHRPROC)eglGetProcAddress("eglSwapBuffersWithDamageKHR");
}

int main(void)
{
    app.wlDisplay = wl_display_connect(NULL);
    assert("wl_display_connect failed" && app.wlDisplay);
    app.wlRegistry = wl_display_get_registry(app.wlDisplay);
    wl_registry_add_listener(app.wlRegistry, &wlRegistryLis, NULL);
    wl_display_roundtrip(app.wlDisplay);
    assert("Failed to get wl_compositor v6" && app.wlCompositor);
    assert("Failed to get xdg_wm_base" && app.wlCompositor);
    initEGL();

    Window window;
    while (wl_display_dispatch(app.wlDisplay) != -1) {}
    return 0;
}
