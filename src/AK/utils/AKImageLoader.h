#ifndef AKIMAGELOADER_H
#define AKIMAGELOADER_H

#include <AK/AK.h>
#include <include/core/SkImage.h>
#include <filesystem>

class AK::AKImageLoader
{
public:
    static sk_sp<SkImage> loadFile(const std::filesystem::path &path) noexcept;
};

#endif // AKIMAGELOADER_H
