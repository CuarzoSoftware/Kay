#include <CZ/AK/Nodes/AKAvatar.h>
#include <CZ/Ream/RCore.h>
#include <CZ/Ream/RDevice.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Ream/RSurface.h>
#include <CZ/Ream/RPass.h>
#include <CZ/skia/core/SkCanvas.h>
#include <drm_fourcc.h>
#include <pwd.h>
#include <unistd.h>
#include <algorithm>

using namespace CZ;
namespace fs = std::filesystem;

// A white antialiased circle on transparent (the mask), shared by every avatar.
static std::shared_ptr<RImage> CircleMask() noexcept
{
    static std::shared_ptr<RImage> mask;

    if (mask)
        return mask;

    constexpr Int32 size { 256 };
    auto surface { RSurface::Make(SkISize::Make(size, size), 1, true) };

    if (!surface)
        return {};

    {
        auto pass { surface->beginPass(RPassCap_SkCanvas) };

        if (!pass)
            return {};

        SkCanvas &c { *pass->getCanvas() };
        c.clear(SK_ColorTRANSPARENT);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorWHITE);
        c.drawCircle(size / 2.f, size / 2.f, size / 2.f, paint);
    }

    mask = surface->image();
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
    image.setMask(CircleMask());
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

void AKAvatar::setPicture(std::shared_ptr<RImage> picture) noexcept
{
    m_picture = picture;
    image.setImage(picture);
    image.setVisible(picture != nullptr);
    icon.setVisible(picture == nullptr);

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
