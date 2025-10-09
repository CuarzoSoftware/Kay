#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKApp.h>

using namespace CZ;

static AKTheme *m_theme { nullptr };

AKTheme *CZ::theme() noexcept
{
    return m_theme;
}

void CZ::setTheme(AKTheme *theme) noexcept
{
    if (m_theme)
        delete m_theme;

    if (theme)
        m_theme = theme;
    else
        m_theme = new AKTheme();
}

class AKKeyboard &CZ::akKeyboard() noexcept
{
    return AKApp::Get()->keyboard();
}

sk_sp<SkFontMgr> CZ::akFontManager() noexcept
{
    return AKApp::Get()->fontManager();
}

AKPointer &CZ::akPointer() noexcept
{
    return AKApp::Get()->pointer();
}

const std::filesystem::path &CZ::AKAssetsDir() noexcept
{
    static const std::filesystem::path AssetsDir { CZ_KAY_ASSETS_DIR };
    return AssetsDir;
}

const std::filesystem::path &CZ::AKFontsDir() noexcept
{
    static const std::filesystem::path FontsDir { CZ_KAY_FONTS_DIR };
    return FontsDir;
}
