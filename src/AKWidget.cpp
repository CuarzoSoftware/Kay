#include "include/core/SkCanvas.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/gl/GrGLTypes.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <iostream>
#include <private/AKApplication.h>
#include <private/AKSurface.h>
#include <private/AKFrame.h>
#include <AKWidget.h>

using namespace AK;

void AKWidget::paintEvent(AKPainter &painter) noexcept
{
    painter.clear(backgroundColor());
}

void AKWidget::setPos(const AKPoint &pos) noexcept
{
    // AKFrames with AKSurface backend always have pos (0,0)
    if (m_surface)
        return;

    m_rect.setPos(pos);
}

void AKWidget::setSize(const AKSize &size) noexcept
{
    m_rect.setSize(size);

    if (m_surface)
    {
        AKFrame &frame { *static_cast<AKFrame*>(this) };
        wl_egl_window_resize(m_surface->imp()->wlEGLWindow, m_rect.w(), m_rect.h(), 0, 0);

        frame.imp()->target = GrBackendRenderTarget(
            m_rect.w(),
            m_rect.h(),
            0, 0,
            frame.imp()->fbInfo);

        SkSurfaceProps skSurfaceProps(0, kUnknown_SkPixelGeometry);

        frame.imp()->skiaSurface = SkSurface::MakeFromBackendRenderTarget(
            (GrRecordingContext*)app()->imp()->skiaContext,
            frame.imp()->target,
            GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,
            SkColorType::kRGBA_8888_SkColorType,
            app()->imp()->colorSpace,
            &skSurfaceProps);

        if (!frame.imp()->skiaSurface.get())
        {
            std::cerr << "SkSurface::MakeRenderTarget returned null.\n";
            exit(1);
        }
    }
    else if (m_isFrame)
    {
        AKFrame &frame { *static_cast<AKFrame*>(this) };

        glDeleteFramebuffers(1, &frame.imp()->fbInfo.fFBOID);
        glDeleteTextures(1, &frame.imp()->textureInfo.fID);

        glGenTextures(1, &frame.imp()->textureInfo.fID);
        glBindTexture(GL_TEXTURE_2D, frame.imp()->textureInfo.fID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_rect.w(), m_rect.h(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        frame.imp()->skiaTexture = GrBackendTexture(
            m_rect.w(),
            m_rect.h(),
            GrMipMapped::kNo,
            frame.imp()->textureInfo);

        frame.imp()->fbInfo.fFormat = GL_RGBA8_OES;
        glGenFramebuffers(1, &frame.imp()->fbInfo.fFBOID);
        glBindFramebuffer(GL_FRAMEBUFFER, frame.imp()->fbInfo.fFBOID);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame.imp()->textureInfo.fID, 0);

        const GLenum status { glCheckFramebufferStatus(GL_FRAMEBUFFER) };

        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cerr << "Framebuffer is not complete!\n";
            exit(1);
        }

        frame.imp()->frame = SkImage::MakeFromTexture(
            app()->imp()->skiaContext,
            frame.imp()->skiaTexture,
            GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,
            SkColorType::kRGBA_8888_SkColorType,
            SkAlphaType::kPremul_SkAlphaType,
            app()->imp()->colorSpace,
            nullptr,
            nullptr);

        if (!frame.imp()->frame.get())
        {
            std::cerr << "SkImage::MakeFromTexture returned null.\n";
            exit(1);
        }

        SkSurfaceProps skSurfaceProps(0, kUnknown_SkPixelGeometry);

        frame.imp()->target = GrBackendRenderTarget(
            m_rect.w(),
            m_rect.h(),
            0, 0,
            frame.imp()->fbInfo);

        frame.imp()->skiaSurface = SkSurface::MakeFromBackendRenderTarget(
            app()->imp()->skiaContext,
            frame.imp()->target,
            GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,
            SkColorType::kRGBA_8888_SkColorType,
            app()->imp()->colorSpace,
            &skSurfaceProps);

        if (!frame.imp()->skiaSurface.get())
        {
            std::cerr << "SkSurface::MakeRenderTarget returned null.\n";
            exit(1);
        }
    }
}

AKPoint AKWidget::frameRelativePos() const noexcept
{
    AKPoint p { pos() };

    AKWidget *par { parent() };

    while (par && !par->isFrame())
    {
        if (!par->isFrame())
            p += par->pos();

        par = par->parent();
    }

    return p;
}
