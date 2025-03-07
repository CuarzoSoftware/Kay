#include <include/gpu/ganesh/gl/GrGLAssembleInterface.h>
#include <include/gpu/ganesh/gl/GrGLDirectContext.h>
#include <include/gpu/ganesh/gl/GrGLBackendSurface.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/gpu/ganesh/GrBackendSurface.h>
#include <include/gpu/ganesh/GrDirectContext.h>
#include <include/core/SkColorSpace.h>

#include <AK/AKGLContext.h>
#include <AK/AKLog.h>

#include <SRMCore.h>
#include <SRMDevice.h>
#include <SRMConnector.h>
#include <SRMConnectorMode.h>
#include <SRMListener.h>
#include <SRMList.h>

#include <Application.h>
#include <Screen.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

Screen::Screen(SRMConnector *connector) noexcept :
    AKContainer(YGFlexDirectionColumn, false, &app()->root),
    connector(connector)
{
    srmConnectorSetUserData(connector, this);
    target = app()->scene.createTarget();
    target->setClearColor(SK_ColorBLACK);
    target->on.markedDirty.subscribe(this, [connector](AKTarget&){
        srmConnectorRepaint(connector);
    });

    auto style = emoji.textStyle();
    SkPaint p;
    p.setAntiAlias(true);
    p.setColor(SK_ColorWHITE);
    style.setForegroundColor(p);
    style.setFontSize(200);
    emoji.setTextStyle(style);
    emoji.setVisible(false);

    layout().setPositionType(YGPositionTypeAbsolute);
    layout().setJustifyContent(YGJustifyCenter);
    layout().setAlignItems(YGAlignCenter);
    logo.setImage(AKImageLoader::loadFile("/usr/local/share/Kay/assets/logo.png"));
    logo.layout().setWidth(128);
    logo.layout().setHeight(128);
    logo.setSizeMode(AKImageFrame::SizeMode::Contain);

    updateDimensions();
    app()->arrangeScreens();
}

Screen::~Screen() noexcept
{
    app()->scene.destroyTarget(target);
}

static Float64 getDPI(Float64 widthMM, Float64 heightMM, Int32 widthPix, Int32 heightPix)
{
    constexpr Float64 mmToInch { 0.0393701 };
    const Float64 widthInch = widthMM * mmToInch;
    const Float64 heightInch = heightMM * mmToInch;

    const Float64 diagonalInch = std::sqrt(widthInch * widthInch + heightInch * heightInch);
    const Float64 diagonalPix = std::sqrt(widthPix * widthPix + heightPix * heightPix);

    return diagonalInch == 0.0 ? 0.f : diagonalPix / diagonalInch;
}

void Screen::updateDimensions() noexcept
{
    SRMConnectorMode *mode { srmConnectorGetCurrentMode(connector) };

    const Float64 dpi { getDPI(
        srmConnectorGetmmWidth(connector),
        srmConnectorGetmmHeight(connector),
        srmConnectorModeGetWidth(mode),
        srmConnectorModeGetHeight(mode))
    };

    target->setBakedComponentsScale(dpi >= 200.f ? 2 : 1);
    layout().setPosition(YGEdgeTop, 0.f);
    layout().setWidth(srmConnectorModeGetWidth(mode) / target->bakedComponentsScale());
    layout().setHeight(srmConnectorModeGetHeight(mode) / target->bakedComponentsScale());
    srmConnectorRepaint(connector);
}

void Screen::updateTarget() noexcept
{
    SRMConnectorMode *mode { srmConnectorGetCurrentMode(connector) };

    const GrGLFramebufferInfo fbInfo
    {
        .fFBOID = srmConnectorGetFramebufferID(connector),
        .fFormat = GL_RGB8_OES
    };

    const GrBackendRenderTarget skTarget = GrBackendRenderTargets::MakeGL(
        srmConnectorModeGetWidth(mode),
        srmConnectorModeGetHeight(mode),
        0, 0,
        fbInfo);

    const SkSurfaceProps skSurfaceProps(0, kUnknown_SkPixelGeometry);

    target->setSurface(SkSurfaces::WrapBackendRenderTarget(
        akApp()->glContext()->skContext().get(),
        skTarget,
        GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
        SkColorType::kRGB_888x_SkColorType,
        SkColorSpace::MakeSRGB(),
        &skSurfaceProps));

    if (!target->surface())
    {
        AKLog::fatal("Failed to create wrapper SkSurface.");
        exit(1);
    }

    target->setViewport(SkRect::MakeXYWH(
        layout().position(YGEdgeLeft).value,
        0,
        layout().width().value,
        layout().height().value));
    target->setDstRect(SkIRect::MakeWH(skTarget.width(), skTarget.height()));
    target->setTransform(AKTransform::Normal);
    target->setAge(srmConnectorGetCurrentBufferAge(connector));
}
