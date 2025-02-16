#include <AK/AKTheme.h>
#include <AK/AKApplication.h>
#include <AK/input/AKKeymap.h>

using namespace AK;

static AKTheme *m_theme { nullptr };

AKTheme *AK::theme() noexcept
{
    if (!m_theme)
        setTheme(nullptr);

    return m_theme;
}

void AK::setTheme(AKTheme *theme) noexcept
{
    if (m_theme)
        delete m_theme;

    if (theme)
        m_theme = theme;
    else
        m_theme = new AKTheme();
}

AKKeymap *AK::keymap() noexcept
{
    static AKKeymap keymap;
    return &keymap;
}

sk_sp<SkFontMgr> AK::AKFontManager() noexcept
{
    return AKApp()->fontManager();
}
