#ifndef AKSURFACE_H
#define AKSURFACE_H

#include <GL/gl.h>
#include <include/core/SkColorSpace.h>
#include <include/core/SkImage.h>
#include <include/core/SkSurface.h>
#include <include/gpu/GrBackendSurface.h>
#include <include/gpu/gl/GrGLTypes.h>
#include <include/gpu/GrRecordingContext.h>
#include <include/core/SkRect.h>
#include <AK/AKObject.h>

class AK::AKSurface : public AKObject
{
public:
    ~AKSurface() { destroyStorage(); }

    static std::shared_ptr<AKSurface> Make(GrRecordingContext *context, const SkSize &size, const SkVector &scale, bool hasAlpha = true) noexcept;

    sk_sp<SkSurface> surface() const noexcept
    {
        return m_surface;
    }

    const GrGLFramebufferInfo& fbInfo() const noexcept
    {
        return m_fbInfo;
    }

    sk_sp<SkImage> image() const noexcept
    {
        return m_image;
    }

    const SkSize &size() const noexcept
    {
        return m_size;
    }

    const SkVector &scale() const noexcept
    {
        return m_scale;
    }

    const SkIRect &imageSrcRect() const noexcept
    {
        return m_imageSrcRect;
    }

    bool hasAlpha() const noexcept
    {
        return m_fbInfo.fFormat == GL_RGBA8;
    }

    bool setHasAlpha(bool alpha) noexcept;
    bool resize(const SkSize &size, const SkVector &scale, bool shrink = false) noexcept;
    bool shrink() noexcept;
private:
    AKSurface(GrRecordingContext *context, bool hasAlpha) noexcept :
        m_context(context)
    {
        m_fbInfo.fFormat = hasAlpha ? GL_RGBA8 : GL_RGB8;
    }
    void destroyStorage() noexcept;
    sk_sp<SkSurface> m_surface;
    sk_sp<SkImage> m_image;
    SkVector m_scale;
    SkIRect m_imageSrcRect;
    SkSize m_size;
    GrGLFramebufferInfo m_fbInfo { 0 };
    GrBackendRenderTarget m_renderTarget;
    GrGLTextureInfo m_textureInfo { 0, 0 };
    GrBackendTexture m_backendTexture;
    GrRecordingContext *m_context;
    sk_sp<SkColorSpace> m_colorSpace = SkColorSpace::MakeSRGB();
};

#endif // AKSURFACE_H
