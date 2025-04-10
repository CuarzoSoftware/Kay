#include <include/gpu/ganesh/SkImageGanesh.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/gpu/ganesh/gl/GrGLBackendSurface.h>
#include <include/gpu/ganesh/GrDirectContext.h>
#include <AK/AKApplication.h>
#include <AK/AKSurface.h>
#include <AK/AKGLContext.h>
#include <GLES2/gl2.h>
#include <cassert>

using namespace AK;

std::shared_ptr<AKSurface> AKSurface::Make(const SkISize &size, SkScalar scale, bool hasAlpha) noexcept
{
    auto surface = std::shared_ptr<AKSurface>(new AKSurface(hasAlpha));
    surface->resize(size, scale, true);
    return surface;
}

sk_sp<SkSurface> AKSurface::surface() const noexcept
{
    if (!m_image)
        return nullptr;

    AKGLContext *ctx {  akApp()->glContext() };
    const auto &fbo { ctx->getFBO(m_slot) };

    if (fbo.serial == m_serial && fbo.skSurface)
        return fbo.skSurface;

    ctx->destroyFBO(m_slot);

    AKGLContext::FBO newFBO;
    newFBO.serial = m_serial;
    glBindTexture(GL_TEXTURE_2D, m_textureInfo.fID);
    glGenFramebuffers(1, &newFBO.id);
    glBindFramebuffer(GL_FRAMEBUFFER, newFBO.id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureInfo.fID, 0);
    m_fbInfo.fFBOID = newFBO.id;
    GrBackendRenderTarget target = GrBackendRenderTargets::MakeGL(
        m_imageSrcRect.width(),
        m_imageSrcRect.height(),
        0, 0,
        m_fbInfo);

    static SkSurfaceProps skSurfaceProps(0, kUnknown_SkPixelGeometry);
    const auto skiaFormat = hasAlpha() ? kRGBA_8888_SkColorType : kRGB_888x_SkColorType;

    newFBO.skSurface = SkSurfaces::WrapBackendRenderTarget(
        ctx->skContext().get(),
        target,
        GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
        skiaFormat,
        m_colorSpace,
        &skSurfaceProps);

    assert("[AkSurface::resize] Failed to create SkSurface" && newFBO.skSurface);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ctx->storeFBO(m_slot, newFBO);
    return newFBO.skSurface;
}

UInt32 AKSurface::fbId() const noexcept
{
    surface();
    return akApp()->glContext()->getFBO(m_slot).id;
}

bool AKSurface::setHasAlpha(bool alpha) noexcept
{
    if (hasAlpha() == alpha)
        return false;

    m_fbInfo.fFormat = alpha ? GL_RGBA8 : GL_RGB8;
    destroyStorage();
    return shrink();
}

bool AKSurface::resize(const SkISize &size, SkScalar scale, bool shrink) noexcept
{
    m_size = size;
    m_scale = scale;

    m_imageSrcRect = SkIRect::MakeWH(
        SkScalar(size.width()) * m_scale,
        SkScalar(size.height()) * m_scale);

    m_viewport = SkRect::Make(m_imageSrcRect);
    m_xyScale.set(scale, scale);

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

    m_backendTexture = GrBackendTextures::MakeGL(
        m_imageSrcRect.width(),
        m_imageSrcRect.height(),
        skgpu::Mipmapped::kNo,
        m_textureInfo);

    m_image = SkImages::BorrowTextureFrom(
        akApp()->glContext()->skContext().get(),
        m_backendTexture,
        GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
        skiaFormat,
        SkAlphaType::kPremul_SkAlphaType,
        m_colorSpace);

    assert("[AkSurface::resize] Failed to create SkImage" && m_image);
    m_fbInfo.fFormat = glSizedFormat;
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

bool AKSurface::shrink() noexcept
{
    return resize(m_size, m_scale, true);
}

SkImage *AKSurface::releaseImage() noexcept
{
    akApp()->glContext()->destroyFBO(m_slot);

    if (m_image)
    {
        const auto skiaFormat = hasAlpha() ? kRGBA_8888_SkColorType : kRGB_888x_SkColorType;
        m_image = SkImages::AdoptTextureFrom(akApp()->glContext()->skContext().get(), m_backendTexture, GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin, skiaFormat, SkAlphaType::kPremul_SkAlphaType, m_colorSpace);
    }

    m_serial++;
    return m_image.release();
}

void AKSurface::destroyStorage() noexcept
{
    akApp()->glContext()->destroyFBO(m_slot);

    if (m_image)
    {
        glBindTexture(GL_FRAMEBUFFER, 0);
        glDeleteTextures(1, &m_textureInfo.fID);
        m_image.reset();
    }

    m_serial++;
}
