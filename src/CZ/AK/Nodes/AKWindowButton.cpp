#include <CZ/Core/CZCore.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZCloseEvent.h>
#include <CZ/AK/Nodes/AKWindowButton.h>
#include <CZ/AK/AKApp.h>
#include <CZ/AK/AKTheme.h>

#ifdef CZ_ENABLE_MARCO
#include <CZ/Marco/roles/MToplevel.h>
#endif

using namespace CZ;

AKWindowButton::AKWindowButton(Type type, AKNode *parent) noexcept :
    AKImage(nullptr, parent)
{
    layout().setWidth(AKTheme::WindowButtonSize.width());
    layout().setHeight(AKTheme::WindowButtonSize.height());
    layout().setMinWidth(AKTheme::WindowButtonSize.width());
    layout().setMinHeight(AKTheme::WindowButtonSize.height());
    layout().setMaxWidth(AKTheme::WindowButtonSize.width());
    layout().setMaxHeight(AKTheme::WindowButtonSize.height());
    setType(type);
}

bool AKWindowButton::setType(Type type) noexcept
{
    if (type == m_type)
        return false;

    m_type = type;
    setImage(theme()->windowButtonImage(scale(), type, state()));
    addDamage(AK_IRECT_INF);
    return true;
}

bool AKWindowButton::setState(State state) noexcept
{
    if (state == m_state)
        return false;

    m_state = state;
    setImage(theme()->windowButtonImage(scale(), type(), state));
    addDamage(AK_IRECT_INF);
    return true;
}

void AKWindowButton::pointerButtonEvent(const CZPointerButtonEvent &e)
{
    if (state() == State::Disabled || e.button != BTN_LEFT)
        return;

    e.accept();

    if (e.pressed)
        setState(Pressed);
    else
    {
#ifdef CZ_ENABLE_MARCO

        if (MToplevel *tl = dynamic_cast<MToplevel*>(window()))
        {
            switch (type())
            {
            case Type::Close:
                if (CZCore::Get()->sendEvent(CZCloseEvent(), *tl))
                    tl->setMapped(false);
                break;
            case Type::Fullscreen:
                tl->setFullscreen(true);
                break;
            case Type::UnsetFullscreen:
                tl->setFullscreen(false);
                break;
            case Type::Maximize:
                tl->setMaximized(!tl->maximized());
                break;
            case Type::Minimize:
                tl->setMinimized();
                break;
            }
        }
#endif
        onClick.notify();
    }
}
