#include <AK/events/AKPointerButtonEvent.h>
#include <AK/nodes/AKWindowButton.h>
#include <AK/AKTheme.h>

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
    if (e.button() != AKPointerButtonEvent::Left)
        return;

    if (e.state() == AKPointerButtonEvent::Pressed)
        setState(Pressed);
    else
        onClick.notify();
}
