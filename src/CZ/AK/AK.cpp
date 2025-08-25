#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKApp.h>

using namespace CZ;

static AKTheme *m_theme { nullptr };
static const std::filesystem::path m_assetsDir { KAY_DEFAULT_ASSETS_PATH };

AKTheme *CZ::theme() noexcept
{
    if (!m_theme)
        setTheme(nullptr);

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

const std::filesystem::path &CZ::akAssetsDir() noexcept
{
    return m_assetsDir;
}
