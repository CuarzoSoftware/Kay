#include <CZ/AK/Nodes/AKFontIcon.h>
#include <CZ/AK/AKIconFont.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/Events/AKRenderEvent.h>
#include <CZ/Events/CZLayoutEvent.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Ream/RPass.h>

using namespace CZ;

AKFontIcon::AKFontIcon(const std::string &iconName, UInt32 size, std::shared_ptr<AKIconFont> iconFont, AKNode *parent) noexcept :
    AKRenderable(RenderableHint::Image, parent)
{
    setIconFont(iconFont);
    enableReplaceImageColor(true);
    setColor(AKTheme::IconLightScheme);
    layout().setAspectRatio(1.f);
    setSize(size);
    setIconFromName(iconName);
}

AKFontIcon::AKFontIcon(const std::string &iconName, UInt32 size, AKNode *parent) noexcept :
    AKRenderable(RenderableHint::Image, parent)
{
    setIconFont({});
    enableReplaceImageColor(true);
    setColor(AKTheme::IconLightScheme);
    layout().setAspectRatio(1.f);
    setSize(size);
    setIconFromName(iconName);
}

AKFontIcon::AKFontIcon(AKNode *parent) noexcept :
    AKRenderable(RenderableHint::Image, parent)
{
    setIconFont({});
    enableReplaceImageColor(true);
    setColor(AKTheme::IconLightScheme);
    layout().setAspectRatio(1.f);
    setSize(0);
}

void AKFontIcon::setSize(UInt32 size) noexcept
{
    layout().setWidth(size);
    layout().setHeight(size);
}

void AKFontIcon::setIconFromName(const std::string &iconName) noexcept
{
    if (!m_isUTF8 && m_icon == iconName)
        return;

    m_isUTF8 = false;
    m_icon = iconName;
    addChange(CHIcon);
}

void AKFontIcon::setIconFromUTF8(const std::string &utf8) noexcept
{
    if (m_isUTF8 && m_icon == utf8)
        return;

    m_isUTF8 = true;
    m_icon = utf8;
    addChange(CHIcon);
}

void AKFontIcon::setIconFont(std::shared_ptr<AKIconFont> iconFont) noexcept
{
    if (!iconFont)
        iconFont = theme()->iconFont;

    if (iconFont == m_iconFont)
        return;

    m_iconFont = iconFont;
    addChange(CHIcon);
}

void AKFontIcon::onSceneBegin()
{
    if (changes().testAnyOf(CHIcon, CHLayoutSize, CHLayoutScale))
    {
        if (m_iconFont)
        {
            if (isUTF8())
                m_image = m_iconFont->getIconByUTF8(m_icon, worldRect().width() * scale());
            else
                m_image = m_iconFont->getIconByName(m_icon, worldRect().width() * scale());
        }
    }

    if (m_image)
        invisibleRegion.setEmpty();
    else
        invisibleRegion.setRect(AK_IRECT_INF);
}

void AKFontIcon::renderEvent(const AKRenderEvent &e)
{
    if (e.damage.isEmpty() || !image())
        return;

    auto *p { e.pass->getPainter() };
    RDrawImageInfo info {};
    info.image = image();
    info.src = SkRect::Make(image()->size());
    info.dst = e.rect;
    p->drawImage(info, &e.damage);
}
