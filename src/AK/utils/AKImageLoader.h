#ifndef AKIMAGELOADER_H
#define AKIMAGELOADER_H

#include <AK/AK.h>
#include <skia/core/SkImage.h>
#include <filesystem>

// TODO: Rename to AKImageUtils
class AK::AKImageLoader
{
public:
    static sk_sp<SkImage> loadFile(const std::filesystem::path &path, const SkISize &size = {-1, -1}) noexcept;
    static sk_sp<SkImage> scaleImage(sk_sp<SkImage> image, const SkISize &size) noexcept;
};

#endif // AKIMAGELOADER_H
