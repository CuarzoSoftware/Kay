#include <AK/AKApplication.h>
#include <AK/AKGLContext.h>
#include <AK/utils/AKImageLoader.h>
#include <AK/AKLog.h>
#include <include/core/SkImage.h>
#include <include/gpu/ganesh/SkImageGanesh.h>
#include <include/core/SkStream.h>

using namespace AK;

sk_sp<SkImage> AKImageLoader::loadFile(const std::filesystem::path &path) noexcept
{
    auto img = SkImages::DeferredFromEncodedData(SkData::MakeFromFileName(path.c_str()));

    if (!img)
    {
        AKLog::error("[AKImageLoader] Failed to load image:  %s", path.c_str());
        return img;
    }

    return SkImages::TextureFromImage(akApp()->glContext()->skContext().get(), img.get());
}
