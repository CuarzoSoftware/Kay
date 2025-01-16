#include <AK/utils/AKImageLoader.h>
#include <include/core/SkImage.h>
#include <include/gpu/ganesh/SkImageGanesh.h>
#include <include/core/SkStream.h>

using namespace AK;

sk_sp<SkImage> AKImageLoader::loadFile(GrDirectContext *context, const std::filesystem::path &path) noexcept
{
    if (!context)
        return sk_sp<SkImage>();

    auto img = SkImages::DeferredFromEncodedData(SkData::MakeFromFileName(path.c_str()));

    if (!img)
        return img;

    return SkImages::TextureFromImage(context, img.get());
}
