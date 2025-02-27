#include <AK/AKTheme.h>
#include <AK/AKApplication.h>

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

class AKKeyboard &AK::akKeyboard() noexcept
{
    return akApp()->keyboard();
}

sk_sp<SkFontMgr> AK::akFontManager() noexcept
{
    return akApp()->fontManager();
}

AKPointer &AK::akPointer() noexcept
{
    return akApp()->pointer();
}
