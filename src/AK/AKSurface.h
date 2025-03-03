#ifndef AKSURFACE_H
#define AKSURFACE_H

#include <GL/gl.h>
#include <include/core/SkColorSpace.h>
#include <include/core/SkImage.h>
#include <include/core/SkSurface.h>
#include <include/core/SkRect.h>
#include <include/gpu/ganesh/gl/GrGLTypes.h>
#include <include/gpu/ganesh/GrBackendSurface.h>
#include <AK/AKObject.h>

class AK::AKSurface : public AKObject
{
public:
    ~AKSurface() { destroyStorage(); }

    static std::shared_ptr<AKSurface> Make(const SkISize &size, Int32 scale, bool hasAlpha = true) noexcept;

    sk_sp<SkSurface> surface() const noexcept;

    GLuint fbId() const noexcept;

    sk_sp<SkImage> image() const noexcept
    {
        return m_image;
    }

    const SkISize &size() const noexcept
    {
        return m_size;
    }

    Int32 scale() const noexcept
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
    bool resize(const SkISize &size, Int32 scale, bool shrink = false) noexcept;
    bool shrink() noexcept;
    SkImage *releaseImage() noexcept;
private:
    AKSurface(bool hasAlpha) noexcept
    {
        static UInt32 slot { 0 };
        m_slot = slot;
        m_fbInfo.fFormat = hasAlpha ? GL_RGBA8 : GL_RGB8;
        slot++;
    }
    AKCLASS_NO_COPY(AKSurface)
    void destroyStorage() noexcept;
    sk_sp<SkImage> m_image;
    Int32 m_scale;
    SkIRect m_imageSrcRect;
    SkISize m_size;
    mutable GrGLFramebufferInfo m_fbInfo { 0 };
    GrGLTextureInfo m_textureInfo { 0, 0 };
    GrBackendTexture m_backendTexture;
    sk_sp<SkColorSpace> m_colorSpace = SkColorSpace::MakeSRGB();
    UInt32 m_slot, m_serial { 1 };
};

#endif // AKSURFACE_H
