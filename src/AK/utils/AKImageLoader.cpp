#include <skia/core/SkCanvas.h>
#include <skia/effects/SkImageFilters.h>
#include <skia/core/SkBitmap.h>
#include <AK/AKApplication.h>
#include <AK/AKSurface.h>
#include <AK/AKGLContext.h>
#include <AK/utils/AKImageLoader.h>
#include <AK/AKLog.h>
#include <skia/core/SkImage.h>
#include <skia/gpu/ganesh/SkImageGanesh.h>
#include <skia/core/SkStream.h>

#include <skia/modules/svg/include/SkSVGDOM.h>
#include <skia/svg/SkSVGCanvas.h>

using namespace AK;

sk_sp<SkImage> loadSVG(const std::filesystem::path &path, const SkISize &size)
{
    auto stream { SkMemoryStream::MakeFromFile(path.c_str()) };
    auto dom { SkSVGDOM::MakeFromStream(*stream.get()) };

    if (!dom)
        return nullptr;

    const SkSize finalSize
    {
        size.fWidth <= 0 ? dom->containerSize().width() : SkScalar(size.width()),
        size.fHeight <= 0 ? dom->containerSize().height() : SkScalar(size.height()),
    };

    if (finalSize.fWidth <= 0 || finalSize.fHeight <= 0 || dom->containerSize().width() <= 0 || dom->containerSize().height() <= 0)
        return nullptr;

    auto surf { AKSurface::Make(finalSize.toCeil(), 1, true) };
    surf->surface()->recordingContext()->asDirectContext()->resetContext();
    auto &c { *surf->surface()->getCanvas() };

    c.save();
    c.scale(finalSize.fWidth/dom->containerSize().width(), size.fHeight/dom->containerSize().height());
    dom->render(&c);
    c.restore();

    surf->surface()->recordingContext()->asDirectContext()->flush();
    return sk_sp<SkImage>(surf->releaseImage());
}

sk_sp<SkImage> AKImageLoader::loadFile(const std::filesystem::path &path, const SkISize &size) noexcept
{
    auto img { loadSVG(path, size) };

    if (img)
        return img;

    img = SkImages::DeferredFromEncodedData(SkData::MakeFromFileName(path.c_str()), SkAlphaType::kPremul_SkAlphaType);

    if (!img)
    {
        AKLog::error("[AKImageLoader] Failed to load image:  %s", path.c_str());
        return nullptr;
    }

    auto skImage = SkImages::TextureFromImage(akApp()->glContext()->skContext().get(), img.get());

    SkISize customSize;

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
