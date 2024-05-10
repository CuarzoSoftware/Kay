#include <GLES2/gl2.h>
#include <private/AKApplication.h>
#include <private/AKSurface.h>
#include <private/AKFrame.h>
#include <AKPainter.h>
#include "include/core/SkCanvas.h"

using namespace AK;

AKPainter::AKPainter(AKFrame *frame) noexcept
{
    m_backend = frame;
}

void AKPainter::clear(const AKColorF &color) noexcept
{
    if (!m_backend)
        return;

    AKFrame *frame { static_cast<AKFrame*>(m_backend) };
    frame->imp()->skiaSurface->getCanvas()->clear((const SkColor4f &)color);
}

void AKPainter::bind() noexcept
{
    if (!m_backend)
        return;

    AKFrame *frame { static_cast<AKFrame*>(m_backend) };

    if (frame->m_surface)
    {
        eglMakeCurrent(app()->imp()->eglDisplay,
                       frame->m_surface->imp()->eglSurface,
                       frame->m_surface->imp()->eglSurface,
                       app()->imp()->eglContext);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }
    else
    {
        eglMakeCurrent(app()->imp()->eglDisplay,
                       EGL_NO_SURFACE,
                       EGL_NO_SURFACE,
                       app()->imp()->eglContext);
        glBindFramebuffer(GL_FRAMEBUFFER, frame->imp()->fbInfo.fFBOID);
    }
}
