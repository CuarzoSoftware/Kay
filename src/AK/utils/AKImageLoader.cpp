#include "include/core/SkCanvas.h"
#include "include/effects/SkImageFilters.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include <include/core/SkBitmap.h>
#include <AK/AKApplication.h>
#include <AK/AKSurface.h>
#include <AK/AKGLContext.h>
#include <AK/utils/AKImageLoader.h>
#include <AK/AKLog.h>
#include <include/core/SkImage.h>
#include <include/gpu/ganesh/SkImageGanesh.h>
#include <include/core/SkStream.h>
#include <plutosvg.h>

using namespace AK;

sk_sp<SkImage> AKImageLoader::loadFile(const std::filesystem::path &path, const SkISize &size) noexcept
{
    SkISize customSize;
    plutovg_surface_t *svgSurf;
    plutosvg_document_t *svgDoc = plutosvg_document_load_from_file(path.c_str(), size.width(), size.height());
    if (svgDoc == NULL)
        goto skia;

    if (size.width() <= 0)
        customSize.fWidth = plutosvg_document_get_width(svgDoc);
    else
        customSize.fWidth = size.width();

    if (size.height() <= 0)
        customSize.fHeight = plutosvg_document_get_height(svgDoc);
    else
        customSize.fHeight = size.height();

    svgSurf = plutosvg_document_render_to_surface(svgDoc, NULL, customSize.width(), customSize.height(), NULL, NULL, NULL);

    if (svgSurf == NULL)
    {
        plutosvg_document_destroy(svgDoc);
        goto skia;
    }

    {
        plutovg_surface_write_to_png(svgSurf, "/home/eduardo/camera.png");

        // Swap R B
        int h, i;
        uint8_t tmp;
        uint8_t *pix { plutovg_surface_get_data(svgSurf) };
        for (int y = 0; y < plutovg_surface_get_height(svgSurf); y++)
        {
            h = y * plutovg_surface_get_stride(svgSurf);

            for (int x = 0; x < plutovg_surface_get_width(svgSurf) * 4; x+=4)
            {
                i = h + x;
                tmp = pix[i];
                pix[i] = pix[i + 2];
                pix[i + 2] = tmp;
            }
        }
    }

    {
        GrGLTextureInfo texInfo;
        texInfo.fFormat = GL_RGBA8;
        texInfo.fTarget = GL_TEXTURE_2D;
        glGenTextures(1, &texInfo.fID);
        glBindTexture(GL_TEXTURE_2D, texInfo.fID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, customSize.width(), customSize.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, plutovg_surface_get_data(svgSurf));
        glBindTexture(GL_TEXTURE_2D, 0);
        glFinish();

        auto backendTexture = GrBackendTextures::MakeGL(
            customSize.width(),
            customSize.height(),
            skgpu::Mipmapped::kNo,
            texInfo);

        auto skImage = SkImages::AdoptTextureFrom(
            akApp()->glContext()->skContext().get(),
            backendTexture,
            GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
            kRGBA_8888_SkColorType,
            SkAlphaType::kPremul_SkAlphaType);

        plutovg_surface_destroy(svgSurf);
        plutosvg_document_destroy(svgDoc);

        if (skImage)
            return skImage;

        glDeleteTextures(1, &texInfo.fID);
    }

skia:
    auto img = SkImages::DeferredFromEncodedData(SkData::MakeFromFileName(path.c_str()));

    if (!img)
    {
        AKLog::error("[AKImageLoader] Failed to load image:  %s", path.c_str());
        return img;
    }

    auto skImage = SkImages::TextureFromImage(akApp()->glContext()->skContext().get(), img.get());

    if (size.width() <= 0)
        customSize.fWidth = skImage->width();
    else
        customSize.fWidth = size.width();

    if (size.height() <= 0)
        customSize.fHeight = skImage->height();
    else
        customSize.fHeight = size.height();

    if (customSize == skImage->dimensions())
        return skImage;

    return scaleImage(skImage, customSize);
}

sk_sp<SkImage> AKImageLoader::scaleImage(sk_sp<SkImage> image, const SkISize &size) noexcept
{
    if (!image || size.width() <= 0 || size.height() <= 0)
        return nullptr;

    auto surface = AKSurface::Make(size, 1, true);
    surface->surface()->recordingContext()->asDirectContext()->resetContext();
    SkCanvas *c { surface->surface()->getCanvas() };
    SkPaint p;

    const SkScalar sigmaX { (SkScalar(image->width()) / SkScalar(size.width())) / 8.f };
    const SkScalar sigmaY { (SkScalar(image->height()) / SkScalar(size.height())) / 8.f };

    AKLog::fatal("Sigma %f %f", sigmaX, sigmaY);

    p.setImageFilter(SkImageFilters::Blur(sigmaX, sigmaY, SkTileMode::kMirror, nullptr));
    p.setAntiAlias(false);
    p.setBlendMode(SkBlendMode::kSrc);
    c->drawImageRect(image.get(), SkRect::Make(surface->imageSrcRect()), SkSamplingOptions(SkFilterMode::kLinear), &p);
    surface->surface()->recordingContext()->asDirectContext()->flush();
    return sk_sp<SkImage>(surface->releaseImage());
}
