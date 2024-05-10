#include <private/AKApplication.h>
#include <iostream>

#include "include/core/SkColorSpace.h"
#include "include/gpu/gl/GrGLAssembleInterface.h"

using namespace AK;

bool AKApplication::Private::initWayland() noexcept
{
    wlDisplay = wl_display_connect(NULL);

    if (!wlDisplay)
    {
        std::cerr << "Failed to connect to compositor.\n";
        return false;
    }

    static const wl_registry_listener registryListener
    {
        .global = [](void */*data*/, wl_registry *registry, UInt32 name, const char *interface, UInt32 version)
        {
            if (!app()->imp()->wlCompositor && strcmp(interface, wl_compositor_interface.name) == 0)
            {
                app()->imp()->wlCompositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 1);
            }
            else if (!app()->imp()->xdgWmBase && strcmp(interface, xdg_wm_base_interface.name) == 0)
            {
                static const xdg_wm_base_listener xdgWmBaseListener
                {
                    .ping = [](void */*data*/, xdg_wm_base *xdgWmBase, UInt32 serial)
                    {
                        xdg_wm_base_pong(xdgWmBase, serial);
                    }
                };

                app()->imp()->xdgWmBase = (xdg_wm_base*)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
                xdg_wm_base_add_listener(app()->imp()->xdgWmBase, &xdgWmBaseListener, nullptr);
            }
        },
        .global_remove = [](void */*data*/, wl_registry *registry, UInt32 name)
        {
            // TODO
        }
    };

    wlRegistry = wl_display_get_registry(wlDisplay);
    wl_registry_add_listener(wlRegistry, &registryListener, nullptr);
    wl_display_roundtrip(wlDisplay);

    if (!wlCompositor)
    {
        std::cerr << "Failed to get wl_compositor.\n";
        return false;
    }

    if (!xdgWmBase)
    {
        std::cerr << "Failed to get xdg_wm_base.\n";
        return false;
    }

    return true;
}

bool AKApplication::Private::initEGL() noexcept
{
    eglDisplay = eglGetDisplay(wlDisplay);

    if (eglDisplay == EGL_NO_DISPLAY)
    {
        std::cerr << "Failed to get EGLDisplay.\n";
        return false;
    }

    EGLint majorVersion, minorVersion;

    if (!eglInitialize(eglDisplay, &majorVersion, &minorVersion))
    {
        std::cerr << "Failed to initialize EGLDisplay.\n";
        return false;
    }

    std::cout << "Using EGL version " << majorVersion << "." << minorVersion << ".\n";

    if (!eglBindAPI(EGL_OPENGL_ES_API))
    {
        std::cerr << "Failed to bind OpenGL ES API.\n";
        return false;
    }

    EGLint numConfigs;

    if ((eglGetConfigs(eglDisplay, NULL, 0, &numConfigs) != EGL_TRUE) || (numConfigs == 0))
    {
        std::cerr << "Failed to get EGL configurations.\n";
        return false;
    }

    EGLint fbAttribs[]
    {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_NONE
    };

    if ((eglChooseConfig(eglDisplay, fbAttribs, &eglConfig, 1, &numConfigs) != EGL_TRUE) || (numConfigs != 1))
    {
        std::cerr << "Failed to choose EGL configuration.\n";
        return false;
    }

    EGLint contextAttribs[] { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };

    eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttribs);

    if (eglContext == EGL_NO_CONTEXT )
    {
        std::cerr << "Failed to create EGL context.\n";
        return false;
    }

    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, eglContext);

    return true;
}

bool AKApplication::Private::initSkia() noexcept
{
    colorSpace = SkColorSpace::MakeSRGB();

    auto interface = GrGLMakeAssembledInterface(nullptr, (GrGLGetProc)*[](void *, const char *p) -> void * {
        return (void *)eglGetProcAddress(p);
    });

    GrContextOptions options;
    options.fShaderCacheStrategy = GrContextOptions::ShaderCacheStrategy::kSkSL;
    options.fAvoidStencilBuffers = true;
    options.fPreferExternalImagesOverES3 = true;
    options.fDisableGpuYUVConversion = true;
    options.fReduceOpsTaskSplitting = GrContextOptions::Enable::kNo;
    options.fReducedShaderVariations = false;

    skiaContext = GrDirectContext::MakeGL(interface, options).release();

    if (!skiaContext)
    {
        std::cerr << "Failed to create Skia context.\n";
        return false;
    }

    return true;
}
