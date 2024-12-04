#include <AK/AKSurface.h>
#include <GLES2/gl2.h>
#include <cassert>
#include <include/core/SkColorSpace.h>
#include <include/gpu/ganesh/SkImageGanesh.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>

using namespace AK;

std::shared_ptr<AKSurface> AKSurface::Make(GrRecordingContext *context, const SkSize &size, const SkVector &scale, bool hasAlpha) noexcept
{
    auto surface = std::shared_ptr<AKSurface>(new AKSurface(context, hasAlpha));
    surface->resize(size, scale, true);
    return surface;
}

bool AKSurface::setHasAlpha(bool alpha) noexcept
{
    if (hasAlpha() == alpha)
        return false;

    m_fbInfo.fFormat = alpha ? GL_RGBA8 : GL_RGB8;
    destroyStorage();
    return shrink();
}

bool AKSurface::resize(const SkSize &size, const SkVector &scale, bool shrink) noexcept
{
    m_size = size;
    m_scale = scale;
    m_imageSrcRect = SkIRect::MakeWH(
        size.width() * scale.x(),
        size.height() * scale.y());

    const bool needsRecreation {
        !m_image ||
        (shrink && (m_image->width() != m_imageSrcRect.width() || (m_image->height() != m_imageSrcRect.height()))) ||
        (!shrink && (m_image->width() < m_imageSrcRect.width() || (m_image->height() < m_imageSrcRect.height())))
    };

    if (!needsRecreation)
        return false;

    destroyStorage();

    const auto glFormat = hasAlpha() ? GL_RGBA : GL_RGB;
    const auto glSizedFormat = hasAlpha() ? GL_RGBA8 : GL_RGB8;
    const auto skiaFormat = hasAlpha() ? kRGBA_8888_SkColorType : kRGB_888x_SkColorType;

    m_textureInfo.fTarget = GL_TEXTURE_2D;
    m_textureInfo.fFormat = glSizedFormat;

    glGenTextures(1, &m_textureInfo.fID);
    glBindTexture(m_textureInfo.fTarget, m_textureInfo.fID);
    glTexImage2D(m_textureInfo.fTarget, 0, glFormat,
                 m_imageSrcRect.width(), m_imageSrcRect.height(),
                 0, glFormat, GL_UNSIGNED_BYTE, nullptr);

    m_backendTexture = GrBackendTexture(
        m_imageSrcRect.width(),
        m_imageSrcRect.height(),
        GrMipMapped::kNo,
        m_textureInfo);

    m_image = SkImages::BorrowTextureFrom(
        m_context,
        m_backendTexture,
        GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
        skiaFormat,
        SkAlphaType::kPremul_SkAlphaType,
        m_colorSpace);

    assert("[AkSurface::resize] Failed to create SkImage" && m_image);

    glGenFramebuffers(1, &m_fbInfo.fFBOID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbInfo.fFBOID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureInfo.fID, 0);
    m_fbInfo.fFormat = glSizedFormat;

    m_renderTarget = GrBackendRenderTarget(
        m_imageSrcRect.width(),
        m_imageSrcRect.height(),
        0, 0,
        m_fbInfo);

    static SkSurfaceProps skSurfaceProps(0, kUnknown_SkPixelGeometry);

    m_surface = SkSurfaces::WrapBackendRenderTarget(
        m_context,
        m_renderTarget,
        GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
        skiaFormat,
        m_colorSpace,
        &skSurfaceProps);

    assert("[AkSurface::resize] Failed to create SkSurface" && m_surface);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_FRAMEBUFFER, 0);
    return true;
}

bool AKSurface::shrink() noexcept
{
    return resize(m_size, m_scale, true);
}

void AKSurface::destroyStorage() noexcept
{
    if (m_image)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &m_fbInfo.fFBOID);
        glDeleteTextures(1, &m_textureInfo.fID);
        m_image.reset();
    }
}
