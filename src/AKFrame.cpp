#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkRegion.h"
#include "include/gpu/gl/GrGLTypes.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <iostream>
#include <private/AKFrame.h>
#include <private/AKSurface.h>
#include <private/AKApplication.h>
#include <AKPainter.h>

using namespace AK;

// Constructor for native GL fb and tex
AKFrame::AKFrame(AKWidget *parent) noexcept : AKWidget(parent), m_imp(std::make_unique<Private>())
{
    m_rect.setSize(AKSize(100, 100));
    m_isFrame = true;

    imp()->textureInfo.fFormat = GL_RGBA8_OES;
    imp()->textureInfo.fTarget = GL_TEXTURE_2D;
    glGenTextures(1,&imp()->textureInfo.fID);
    glBindTexture(GL_TEXTURE_2D, imp()->textureInfo.fID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_rect.w(), m_rect.h(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    imp()->skiaTexture = GrBackendTexture(
        m_rect.w(),
        m_rect.h(),
        GrMipMapped::kNo,
        imp()->textureInfo);

    imp()->fbInfo.fFormat = GL_RGBA8_OES;
    glGenFramebuffers(1, &imp()->fbInfo.fFBOID);
    glBindFramebuffer(GL_FRAMEBUFFER, imp()->fbInfo.fFBOID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, imp()->textureInfo.fID, 0);

    const GLenum status { glCheckFramebufferStatus(GL_FRAMEBUFFER) };

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Framebuffer is not complete!\n";
        exit(1);
    }

    imp()->frame = SkImage::MakeFromTexture(
        app()->imp()->skiaContext,
        imp()->skiaTexture,
        GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,
        SkColorType::kRGBA_8888_SkColorType,
        SkAlphaType::kPremul_SkAlphaType,
        app()->imp()->colorSpace,
        nullptr,
        nullptr);

    if (!imp()->frame.get())
    {
        std::cerr << "SkImage::MakeFromTexture returned null.\n";
        exit(1);
    }

    SkSurfaceProps skSurfaceProps(0, kUnknown_SkPixelGeometry);

    imp()->target = GrBackendRenderTarget(
        m_rect.w(),
        m_rect.h(),
        0, 0,
        imp()->fbInfo);

    imp()->skiaSurface = SkSurface::MakeFromBackendRenderTarget(
        app()->imp()->skiaContext,
        imp()->target,
        GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,
        SkColorType::kRGBA_8888_SkColorType,
        app()->imp()->colorSpace,
        &skSurfaceProps);

    if (!imp()->skiaSurface.get())
    {
        std::cerr << "SkSurface::MakeRenderTarget returned null.\n";
        exit(1);
    }

    imp()->painter.reset(new AKPainter(this));
    imp()->painter->bind();
}

AKFrame::~AKFrame() noexcept
{

}

static void drawWidget(AKWidget *widget, AKFrame *frame, SkRegion *clip)
{
    if (widget->isFrame())
    {
        if (frame != widget)
        {
            SkRegion rcpy = *clip;
            static_cast<AKFrame*>(widget)->render(frame, &rcpy);
            return;
        }
        else
            frame = static_cast<AKFrame*>(widget);
    }

    frame->imp()->painter->bind();
    frame->imp()->skiaSurface->getCanvas()->save();
    frame->imp()->skiaSurface->getCanvas()->resetMatrix();

    if (!widget->isFrame())
    {
        const AKPoint frameRelPos { widget->frameRelativePos() };
        clip->op(SkIRect::MakeXYWH(frameRelPos.x(), frameRelPos.y(), widget->size().w(), widget->size().h()), SkRegion::Op::kIntersect_Op);
        frame->imp()->skiaSurface->getCanvas()->translate(frameRelPos.x(), frameRelPos.y());
    }

    frame->imp()->skiaSurface->getCanvas()->clipRegion(*clip);

    widget->paintEvent(*frame->imp()->painter);
    frame->imp()->skiaSurface->getCanvas()->restore();
    frame->imp()->skiaSurface->getCanvas()->flush();

    for (AKWidget *child : widget->children())
    {
        SkRegion rcpy = *clip;
        drawWidget(child, frame, &rcpy);
    }
}

void AKFrame::render(AK::AKFrame *frame, SkRegion *clip) noexcept
{
    imp()->painter.get()->bind();
    imp()->skiaSurface->getCanvas()->resetMatrix();
    imp()->skiaSurface->getCanvas()->save();
    SkRegion reg;
    reg.op(SkIRect::MakeWH(m_rect.w(), m_rect.h()), SkRegion::Op::kUnion_Op);
    drawWidget(this, this, &reg);
    imp()->skiaSurface->getCanvas()->restore();
    imp()->skiaSurface->getCanvas()->flush();

    if (m_surface)
    {
        eglSwapBuffers(app()->imp()->eglDisplay, m_surface->imp()->eglSurface);
    }
    else if (frame)
    {
        frame->imp()->painter->bind();
        frame->imp()->skiaSurface->getCanvas()->resetMatrix();
        frame->imp()->skiaSurface->getCanvas()->save();

        frame->imp()->skiaSurface->getCanvas()->translate(frameRelativePos().x(), frameRelativePos().y());
        if (clip)
            frame->imp()->skiaSurface->getCanvas()->clipRegion(*clip);
        frame->imp()->skiaSurface->getCanvas()->drawImage(imp()->frame, 0, 0);
        frame->imp()->skiaSurface->getCanvas()->restore();
        frame->imp()->skiaSurface->getCanvas()->flush();
    }
}

// Constructor for AKSurface (Wayland EGL surface and no tex)
AKFrame::AKFrame(AKSurface *surface, const AKSize &size) noexcept : AKWidget(nullptr), m_imp(std::make_unique<Private>())
{
    setBackgroundColor({1.f, 1.f, 1.f, 1.f});
    m_rect.setSize(size);
    m_surface = surface;
    m_isFrame = true;

    imp()->fbInfo.fFBOID = 0;
    imp()->fbInfo.fFormat = GL_RGBA8_OES;

    surface->imp()->wlSurface = wl_compositor_create_surface(app()->imp()->wlCompositor);
    surface->imp()->wlEGLWindow = wl_egl_window_create(surface->imp()->wlSurface, m_rect.w(), m_rect.h());
    surface->imp()->eglSurface = eglCreateWindowSurface(
        app()->imp()->eglDisplay,
        app()->imp()->eglConfig,
        (EGLNativeWindowType)surface->imp()->wlEGLWindow,
        NULL);

    if (surface->imp()->eglSurface == EGL_NO_SURFACE)
    {
        std::cerr << "Failed to create EGL surface.\n";
        exit(1);
    }

    imp()->target = GrBackendRenderTarget(
        m_rect.w(),
        m_rect.h(),
        0, 0,
        imp()->fbInfo);

    SkSurfaceProps skSurfaceProps(0, kUnknown_SkPixelGeometry);

    imp()->skiaSurface = SkSurface::MakeFromBackendRenderTarget(
        app()->imp()->skiaContext,
        imp()->target,
        GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,
        SkColorType::kRGBA_8888_SkColorType,
        app()->imp()->colorSpace,
        &skSurfaceProps);

    if (!imp()->skiaSurface.get())
    {
        std::cerr << "SkSurface::MakeRenderTarget returned null.\n";
        exit(1);
    }

    imp()->painter.reset(new AKPainter(this));
    imp()->painter->bind();
    //eglSwapInterval(app()->imp()->eglDisplay, 0);
}
