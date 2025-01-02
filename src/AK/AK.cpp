#include <AK/AKTheme.h>

using namespace AK;

static AKTheme *m_theme { nullptr };

AKTheme *AK::theme() noexcept
{
    if (!m_theme)
        m_theme = new AKTheme();

    return m_theme;
}
