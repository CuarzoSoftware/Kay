#ifndef AKSURFACE_H
#define AKSURFACE_H

#include <GL/gl.h>
#include <CZ/skia/core/SkColorSpace.h>
#include <CZ/skia/core/SkImage.h>
#include <CZ/skia/core/SkSurface.h>
#include <CZ/skia/core/SkRect.h>
#include <CZ/skia/gpu/ganesh/gl/GrGLTypes.h>
#include <CZ/skia/gpu/ganesh/GrBackendSurface.h>
#include <AK/AKTarget.h>

class AK::AKSurface : public AKTarget
{
public:
    ~AKSurface() { destroyStorage(); }

    static std::shared_ptr<AKSurface> Make(const SkISize &size, SkScalar scale, bool hasAlpha = true) noexcept;

    sk_sp<SkSurface> surface() const noexcept override;

    sk_sp<SkImage> image() const noexcept
    {
        return m_image;
    }

    const SkISize &size() const noexcept
    {
        return m_size;
    }

    SkScalar scale() const noexcept
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
    bool resize(const SkISize &size, SkScalar scale, bool shrink = false) noexcept;
    bool shrink() noexcept;
    SkImage *releaseImage() noexcept;

    void setViewportPos(SkScalar x, SkScalar y) noexcept { m_viewport.offsetTo(x, y); }
    const SkRect &viewport() const noexcept override { return m_viewport; };
    AKTransform transform() const noexcept override { return AKTransform::Normal; };
    const SkVector &xyScale() const noexcept override { return m_xyScale; };
    UInt32 fbId() const noexcept override;
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
    SkScalar m_scale;
    SkIRect m_imageSrcRect;
    SkRect m_viewport;
    SkISize m_size;
    mutable GrGLFramebufferInfo m_fbInfo { 0 };
    GrGLTextureInfo m_textureInfo { 0, 0 };
    GrBackendTexture m_backendTexture;
    sk_sp<SkColorSpace> m_colorSpace = SkColorSpace::MakeSRGB();
    UInt32 m_slot, m_serial { 1 };
    SkVector m_xyScale { 1.f, 1.f };
};

#endif // AKSURFACE_H
