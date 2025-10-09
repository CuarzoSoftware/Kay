#include <CZ/AK/Nodes/AKIcon.h>
#include <CZ/AK/AKIconFont.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/Events/AKRenderEvent.h>
#include <CZ/Events/CZLayoutEvent.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Ream/RPass.h>

using namespace CZ;

AKIcon::AKIcon(const std::string &iconName, UInt32 size, AKNode *parent) noexcept :
    AKRenderable(RenderableHint::Image, parent)
{
    enableReplaceImageColor(true);
    setColor(AKTheme::IconLightScheme);
    layout().setAspectRatio(1.f);
    setSize(size);
    setIcon(iconName);
}

void AKIcon::setSize(UInt32 size) noexcept
{
    layout().setWidth(size);
    layout().setHeight(size);
}

void AKIcon::setIcon(const std::string &iconName) noexcept
{
    if (m_iconName == iconName)
        return;

    m_iconName = iconName;
    addDamage(AK_IRECT_INF);
    addChange(CHIcon);
}

void AKIcon::onSceneBegin()
{
    if (changes().test(CHIcon))
        if (theme()->iconFont)
            m_image = theme()->iconFont->getIcon(m_iconName, worldRect().width() * scale());
}

void AKIcon::layoutEvent(const CZLayoutEvent &event)
{
    if (event.changes.has(CZLayoutChangeScale | CZLayoutChangeSize))
        if (theme()->iconFont)
            m_image = theme()->iconFont->getIcon(m_iconName, worldRect().width() * scale());
}

void AKIcon::renderEvent(const AKRenderEvent &e)
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
