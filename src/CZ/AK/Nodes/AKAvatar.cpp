#include <CZ/AK/Nodes/AKAvatar.h>
#include <CZ/Ream/RCore.h>
#include <CZ/Ream/RDevice.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Ream/RSurface.h>
#include <CZ/Ream/RPass.h>
#include <CZ/skia/core/SkCanvas.h>
#include <CZ/skia/core/SkBitmap.h>
#include <drm_fourcc.h>
#include <pwd.h>
#include <unistd.h>
#include <algorithm>
#include <map>
#include <cmath>

using namespace CZ;
namespace fs = std::filesystem;

// A white antialiased circle on transparent (the mask), @p size pixels wide: drawn at the size it's
// shown at (a larger one scaled down without mipmaps loses its antialiasing). Shared by the avatars
// of that size.
static std::shared_ptr<RImage> CircleMask(Int32 size) noexcept
{
    static std::map<Int32, std::weak_ptr<RImage>> masks;
    size = std::clamp(size, 1, 1024);

    if (auto mask { masks[size].lock() })
        return mask;

    std::erase_if(masks, [](const auto &m){ return m.second.expired(); });

    // Drawn on the CPU: antialiased whatever the GPU context does
    auto ream { RCore::Get() };

    if (!ream || !ream->mainDevice())
        return {};

    const auto &formats { ream->mainDevice()->textureFormats().formats() };
    const auto format { formats.find(DRM_FORMAT_ABGR8888) }; // RGBA in memory

    if (format == formats.end())
        return {};

    SkBitmap bitmap;

    if (!bitmap.tryAllocPixels(SkImageInfo::Make(size, size, kRGBA_8888_SkColorType, kPremul_SkAlphaType)))
        return {};

    {
        SkCanvas c { bitmap };
        c.clear(SK_ColorTRANSPARENT);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorWHITE);
        c.drawCircle(size / 2.f, size / 2.f, size / 2.f, paint);
    }

    RPixelBufferInfo info {};
    info.pixels = static_cast<UInt8*>(bitmap.getPixels());
    info.stride = UInt32(bitmap.rowBytes());
    info.format = format->format();
    info.size = SkISize::Make(size, size);
    auto mask { RImage::MakeFromPixels(info, *format) };

    if (!mask)
        return {};

    masks[size] = mask;
    return mask;
}

AKAvatar::AKAvatar(Int32 size, AKNode *parent) noexcept :
    AKRoundSolidColor(0xFFA8A8AD, CZBorderRadius::Make(0), parent)
{
    layout().setJustifyContent(YGJustifyCenter);
    layout().setAlignItems(YGAlignCenter);
    layout().setFlexShrink(0.f);

    const SkRegion empty;
    image.setInputRegion(&empty);
    image.layout().setPositionType(YGPositionTypeAbsolute);
    image.layout().setPosition(YGEdgeLeft, 0.f);
    image.layout().setPosition(YGEdgeTop, 0.f);
    image.layout().setWidthPercent(100.f);
    image.layout().setHeightPercent(100.f);
    image.setSrcRectMode(AKImage::SrcRectMode::Custom);

    icon.setInputRegion(&empty);
    icon.setIconFromName("person");
    icon.setColor(SK_ColorWHITE);

    setSize(size);
    setPicture(nullptr);
}

void AKAvatar::setSize(Int32 size) noexcept
{
    size = std::max(size, 1);

    if (size == m_size)
        return;

    m_size = size;
    layout().setWidth(size);
    layout().setHeight(size);
    setBorderRadius(CZBorderRadius::Make(size / 2));
    icon.setSize(UInt32(std::max(1, Int32(size * 0.7f))));
}

void AKAvatar::onSceneBegin()
{
    AKRoundSolidColor::onSceneBegin();

    // Its diameter in pixels (the mask's antialiasing at the size it's drawn at)
    const Int32 pixels { Int32(std::ceil(m_size * std::max(1, scale()))) };

    if (pixels != m_maskSize)
    {
        m_maskSize = pixels;
        image.setMask(CircleMask(pixels));
    }
}

void AKAvatar::setPicture(std::shared_ptr<RImage> picture) noexcept
{
    m_picture = picture;
    image.setImage(picture);
    image.setVisible(picture != nullptr);
    icon.setVisible(picture == nullptr);

    // The gray circle only behind the silhouette: under the picture its edge would show around the
    // picture's antialiased one
    setBackgroundColor(picture ? SK_ColorTRANSPARENT : 0xFFA8A8AD);

    if (!picture)
        return;

    // The centered square of it.
    const SkISize s { picture->size() };
    const Int32 side { std::min(s.width(), s.height()) };
    image.setCustomSrcRectScale(1.f);
    image.setCustomSrcRect(SkRect::MakeXYWH((s.width() - side) / 2.f, (s.height() - side) / 2.f, side, side));
}

bool AKAvatar::setPictureFromFile(const fs::path &path) noexcept
{
    std::shared_ptr<RImage> picture;
    auto ream { RCore::Get() };

    if (!path.empty() && ream && ream->mainDevice())
    {
        const auto &formats { ream->mainDevice()->textureFormats().formats() };

        if (auto fmt { formats.find(DRM_FORMAT_ARGB8888) }; fmt != formats.end())
            picture = RImage::LoadFile(path, *fmt);
    }

    setPicture(picture);
    return picture != nullptr;
}

static const passwd *CurrentUser() noexcept
{
    return getpwuid(getuid());
}

fs::path AKAvatar::UserPicturePath() noexcept
{
    std::error_code ec;
    const passwd *pw { CurrentUser() };

    if (pw && pw->pw_name)
        if (const fs::path p { fs::path("/var/lib/AccountsService/icons") / pw->pw_name }; fs::is_regular_file(p, ec))
            return p;

    const char *home { pw && pw->pw_dir ? pw->pw_dir : getenv("HOME") };

    if (home)
        if (const fs::path p { fs::path(home) / ".face" }; fs::is_regular_file(p, ec))
            return p;

    return {};
}

std::string AKAvatar::UserLoginName() noexcept
{
    const passwd *pw { CurrentUser() };
    return pw && pw->pw_name ? pw->pw_name : "";
}

std::string AKAvatar::UserRealName() noexcept
{
    const passwd *pw { CurrentUser() };
    std::string name;

    // GECOS: "Full Name,room,phone,..."
    if (pw && pw->pw_gecos)
    {
        name = pw->pw_gecos;

        if (const auto comma { name.find(',') }; comma != std::string::npos)
            name.resize(comma);
    }

    return name.empty() ? UserLoginName() : name;
}
