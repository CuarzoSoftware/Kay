#include <SRMCore.h>
#include <SRMDevice.h>
#include <SRMConnector.h>
#include <SRMConnectorMode.h>
#include <SRMListener.h>

#include <SRMList.h>
#include <SRMLog.h>

#include <GLES2/gl2.h>

#include <fcntl.h>
#include <unistd.h>

#include <include/gpu/gl/GrGLInterface.h>
#include <include/gpu/gl/GrGLTypes.h>
#include <include/gpu/GrDirectContext.h>
#include <include/gpu/GrBackendSurface.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/core/SkImage.h>
#include <include/core/SkSurface.h>
#include <include/core/SkCanvas.h>
#include <include/gpu/gl/GrGLAssembleInterface.h>
#include <include/core/SkColorSpace.h>
#include <AK/AKScene.h>
#include <AK/nodes/AKContainer.h>
#include <AK/nodes/AKRenderableRect.h>

using namespace AK;

static sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeSRGB();
static SkSurfaceProps skSurfaceProps(0, kUnknown_SkPixelGeometry);
static AKScene scene;
static AKContainer root;

struct ConnectorData
{
    GrContextOptions contextOptions;
    sk_sp<GrDirectContext> context;
    AKTarget *target { nullptr };
    AKRenderableRect background { SK_ColorWHITE, &root };
    AKRenderableRect childLeft  { SK_ColorRED, &background };
    AKRenderableRect childRight { SK_ColorBLUE, &background };
};

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

static void initializeGL(SRMConnector *connector, void *userData)
{
    ConnectorData *data = (ConnectorData*)userData;

    auto interface = GrGLMakeAssembledInterface(nullptr, (GrGLGetProc)*[](void *, const char *p) -> void * {
        return (void *)eglGetProcAddress(p);
    });

    data->contextOptions.fShaderCacheStrategy = GrContextOptions::ShaderCacheStrategy::kSkSL;
    data->contextOptions.fAvoidStencilBuffers = true;
    data->contextOptions.fPreferExternalImagesOverES3 = true;
    data->contextOptions.fDisableGpuYUVConversion = true;
    data->contextOptions.fReduceOpsTaskSplitting = GrContextOptions::Enable::kNo;
    data->contextOptions.fReducedShaderVariations = false;

    data->context = GrDirectContext::MakeGL(interface, data->contextOptions);

    if (!data->context.get())
    {
        SRMFatal("Failed to create context.");
        exit(1);
    }

    data->target = scene.createTarget();
    srmConnectorRepaint(connector);
}

static void paintGL(SRMConnector *connector, void *userData)
{
    ConnectorData *data = (ConnectorData*)userData;
    SRMConnectorMode *mode = srmConnectorGetCurrentMode(connector);

    const GrGLFramebufferInfo fbInfo
    {
        .fFBOID = srmConnectorGetFramebufferID(connector),
        .fFormat = GL_RGB8_OES
    };

    const GrBackendRenderTarget target(
        srmConnectorModeGetWidth(mode),
        srmConnectorModeGetHeight(mode),
        0, 0,
        fbInfo);

    data->target->surface = SkSurfaces::WrapBackendRenderTarget(
        data->context.get(),
        target,
        GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
        SkColorType::kRGB_888x_SkColorType,
        colorSpace,
        &skSurfaceProps);

    if (!data->target->surface)
    {
        SRMFatal("No SkSurface.");
        exit(1);
    }

    data->background.layout().setPadding(YGEdgeAll, 100.f);
    data->background.layout().setGap(YGGutterAll, 100.f);
    data->background.layout().setFlexDirection(YGFlexDirectionRow);
    data->background.layout().setWidth(target.width());
    data->background.layout().setHeight(target.height());

    data->childLeft.layout().setFlex(1.f);
    data->childRight.layout().setFlex(1.f);

    data->target->viewport = { 0.f, 0.f, (float)target.width(), (float)target.height() };
    data->target->scale = 1.f;
    data->target->transform = AKTransform::Rotated180;
    scene.render(data->target);
    data->target->surface->flush();
    srmConnectorRepaint(connector);
}

static void resizeGL(SRMConnector */*connector*/, void */*userData*/) {}
static void pageFlipped(SRMConnector */*connector*/, void */*userData*/) {}

static void uninitializeGL(SRMConnector */*connector*/, void *userData)
{
    ConnectorData *data = (ConnectorData*)userData;
    delete data;
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
    ConnectorData *data { new ConnectorData };

    if (!srmConnectorInitialize(connector, &connectorInterface, data))
    {
        SRMError("[srm-skia] Failed to initialize connector %s.",
                 srmConnectorGetModel(connector));

        delete data;
    }
}

static void connectorPluggedEventHandler(SRMListener */*listener*/, SRMConnector *connector)
{
    initConnector(connector);
}

static void connectorUnpluggedEventHandler(SRMListener *listener, SRMConnector *connector)
{
    SRM_UNUSED(listener);
    SRM_UNUSED(connector);
}

int main(void)
{
    setenv("SRM_DEBUG", "4", 0);

    SRMCore *core = srmCoreCreate(&srmInterface, NULL);

    if (!core)
    {
        SRMFatal("[srm-skia] Failed to initialize SRM core.");
        return 1;
    }

    srmCoreAddConnectorPluggedEventListener(core, &connectorPluggedEventHandler, NULL);
    srmCoreAddConnectorUnpluggedEventListener(core, &connectorUnpluggedEventHandler, NULL);

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

    while (srmCoreProcessMonitor(core, -1) >= 0) {}
    srmCoreDestroy(core);
    return 0;
}
