#include <SRMCore.h>
#include <SRMDevice.h>
#include <SRMConnector.h>
#include <SRMConnectorMode.h>
#include <SRMListener.h>
#include <SRMList.h>
#include <SRMLog.h>

#include <CZ/AK/AKTheme.h>

#include <Application.h>
#include <Screen.h>

#include <fcntl.h>

static std::mutex mutex;

static int openRestricted(const char *path, int flags, void *userData)
{
    SRM_UNUSED(userData);
    return open(path, flags);
}

static void closeRestricted(int fd, void *userData)
{
    SRM_UNUSED(userData);
    close(fd);
}

static SRMInterface srmInterface
{
    .openRestricted = &openRestricted,
    .closeRestricted = &closeRestricted
};

static void initializeGL(SRMConnector *connector, void *)
{
    std::lock_guard<std::mutex> lock(mutex);
    Screen *screen { new Screen(connector) };
    srmConnectorSetUserData(connector, screen);
    srmConnectorRepaint(connector);
}

static void paintGL(SRMConnector *connector, void *)
{
    std::lock_guard<std::mutex> lock(mutex);
    Screen *screen { static_cast<Screen*>(srmConnectorGetUserData(connector)) };
    screen->updateTarget();
    app()->scene.render(screen->target);
}

static void resizeGL(SRMConnector *connector, void *)
{
    std::lock_guard<std::mutex> lock(mutex);
    Screen *screen { static_cast<Screen*>(srmConnectorGetUserData(connector)) };
    screen->updateDimensions();
    srmConnectorRepaint(connector);
}
static void pageFlipped(SRMConnector *, void *) {}

static void uninitializeGL(SRMConnector *connector, void *)
{
    std::lock_guard<std::mutex> lock(mutex);
    Screen *screen { static_cast<Screen*>(srmConnectorGetUserData(connector)) };
    delete screen;
}

static SRMConnectorInterface connectorInterface
{
    .initializeGL = &initializeGL,
    .paintGL = &paintGL,
    .pageFlipped = &pageFlipped,
    .resizeGL = &resizeGL,
    .uninitializeGL = &uninitializeGL
};

static void initConnector(SRMConnector *connector)
{
    if (!srmConnectorInitialize(connector, &connectorInterface, nullptr))
    {
        SRMError("[srm-skia] Failed to initialize connector %s.",
                 srmConnectorGetModel(connector));
    }
}

static void connectorPluggedEventHandler(SRMListener *, SRMConnector *connector)
{
    initConnector(connector);
}

Application::Application() noexcept : AKApp()
{
    scene.setRoot(&root);
    core = srmCoreCreate(&srmInterface, this);

    if (!core)
    {
        SRMFatal("[Application] Failed to initialize SRM core.");
        exit(0);
    }

    app()->addEventSource(srmCoreGetMonitorFD(core), EPOLLIN, [](int /*fd*/, UInt32 /*events*/){
        srmCoreProcessMonitor(app()->core, 0);
    });

    srmCoreAddConnectorPluggedEventListener(core, &connectorPluggedEventHandler, NULL);

    SRMListForeach (deviceIt, srmCoreGetDevices(core))
    {
        SRMDevice *device = (SRMDevice*)srmListItemGetData(deviceIt);

        SRMListForeach (connectorIt, srmDeviceGetConnectors(device))
        {
            SRMConnector *connector = (SRMConnector*)srmListItemGetData(connectorIt);

            if (srmConnectorIsConnected(connector))
                initConnector(connector);
        }
    }
}

Application::~Application() noexcept
{
    if (core)
        srmCoreDestroy(core);
}

// Arrange screens from left to right
void Application::arrangeScreens() noexcept
{
    Float32 x { 0.f };
    SRMListForeach (deviceIt, srmCoreGetDevices(app()->core))
    {
        SRMDevice *device = (SRMDevice*)srmListItemGetData(deviceIt);

        SRMListForeach (connectorIt, srmDeviceGetConnectors(device))
        {
            SRMConnector *connector = (SRMConnector*)srmListItemGetData(connectorIt);

            if (srmConnectorGetState(connector) != SRM_CONNECTOR_STATE_UNINITIALIZED)
            {
                Screen *screen { static_cast<Screen*>(srmConnectorGetUserData(connector)) };
                screen->layout().setPosition(YGEdgeLeft, x);
                x += screen->layout().width().value;
            }
        }
    }
}

void Application::finish() noexcept
{
    SRMListForeach (deviceIt, srmCoreGetDevices(app()->core))
    {
        SRMDevice *device = (SRMDevice*)srmListItemGetData(deviceIt);

        SRMListForeach (connectorIt, srmDeviceGetConnectors(device))
        {
            SRMConnector *connector = (SRMConnector*)srmListItemGetData(connectorIt);

            if (srmConnectorGetState(connector) != SRM_CONNECTOR_STATE_UNINITIALIZED)
            {
                Screen *screen { static_cast<Screen*>(srmConnectorGetUserData(connector)) };
                screen->emoji.setVisible(true);
                screen->logo.setVisible(false);
                screen->progressBar.setVisible(false);
            }
        }
    }
}
