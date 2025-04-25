#include <AK/events/AKPointerButtonEvent.h>
#include <AK/events/AKWindowCloseEvent.h>
#include <AK/nodes/AKWindowButton.h>
#include <AK/AKApplication.h>
#include <AK/AKTheme.h>

#include <Marco/roles/MToplevel.h>

using namespace AK;

AKWindowButton::AKWindowButton(Type type, AKNode *parent) noexcept :
    AKRenderableImage(nullptr, parent)
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

void AKWindowButton::pointerButtonEvent(const AKPointerButtonEvent &e)
{
    if (state() == State::Disabled || e.button() != AKPointerButtonEvent::Left)
        return;

    if (e.state() == AKPointerButtonEvent::Pressed)
        setState(Pressed);
    else
    {
        if (MToplevel *tl = dynamic_cast<MToplevel*>(window()))
        {
            switch (type())
            {
            case Type::Close:
                if (akApp()->sendEvent(AKWindowCloseEvent(), *tl))
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

        onClick.notify();
    }
}
