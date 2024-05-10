#include "include/core/SkCanvas.h"
#include <private/AKApplication.h>
#include <private/AKFrame.h>
#include <AKSurface.h>
#include <wayland-client.h>
#include <iostream>
#include <string.h>

using namespace AK;

static AKApplication *s_app { nullptr };

AKApplication *AK::app()
{
    return s_app;
}

AKApplication::AKApplication() : m_imp(std::make_unique<Private>())
{
    /* TODO: add clean up on failure */

    if (s_app)
    {
        std::cerr << "Only a single AKApplication instance can exist.\n";
        return;
    }

    s_app = this;

    if (!imp()->initWayland())
        return;

    if (!imp()->initEGL())
        return;

    if (!imp()->initSkia())
        return;

    imp()->initialized = true;
}

AKApplication::~AKApplication()
{
    if (s_app == this)
        s_app = nullptr;
}

int AKApplication::run()
{
    if (!imp()->initialized)
        return EXIT_FAILURE;

    if (imp()->running)
        return EXIT_SUCCESS;

    imp()->running = true;

    while (imp()->running)
    {
        for (AKSurface *surface : imp()->surfaces)
            surface->widget()->render();

        wl_display_dispatch(imp()->wlDisplay);
        wl_display_flush(imp()->wlDisplay);
    }

    // TODO: add clean up

    return EXIT_SUCCESS;
}
