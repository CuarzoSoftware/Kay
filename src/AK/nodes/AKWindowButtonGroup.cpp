#include <AK/events/AKPointerButtonEvent.h>
#include <AK/nodes/AKWindowButtonGroup.h>
#include <AK/AKTheme.h>

using namespace AK;

AKWindowButtonGroup::AKWindowButtonGroup(AKNode *parent) noexcept :
    AKContainer(YGFlexDirectionRow, true, parent)
{
    layout().setGap(YGGutterAll, AKTheme::WindowButtonGap);
    layout().setHeight(AKTheme::WindowButtonSize.height());

    const auto state { activated() ? AKWindowButton::State::Normal : AKWindowButton::State::Disabled };
    closeButton.setState(state);
    minimizeButton.setState(state);
    maximizeButton.setState(state);
}

void AKWindowButtonGroup::pointerEnterEvent(const AKPointerEnterEvent &)
{
    closeButton.setState(AKWindowButton::State::Hover);
    minimizeButton.setState(AKWindowButton::State::Hover);
    maximizeButton.setState(AKWindowButton::State::Hover);
}

void AKWindowButtonGroup::pointerLeaveEvent(const AKPointerLeaveEvent &)
{
    const auto state { activated() ? AKWindowButton::State::Normal : AKWindowButton::State::Disabled };
    closeButton.setState(state);
    minimizeButton.setState(state);
    maximizeButton.setState(state);
}

void AKWindowButtonGroup::pointerButtonEvent(const AKPointerButtonEvent &e)
{
    if (e.button() != AKPointerButtonEvent::Left)
        return;

    if (e.state() == AKPointerButtonEvent::Released)
    {
        const auto state { AKWindowButton::State::Hover };
        closeButton.setState(state);
        minimizeButton.setState(state);
        maximizeButton.setState(state);
    }
}

void AKWindowButtonGroup::windowStateEvent(const AKWindowStateEvent &)
{
    const auto state { activated() ? AKWindowButton::State::Normal : AKWindowButton::State::Disabled };
    closeButton.setState(state);
    minimizeButton.setState(state);
    maximizeButton.setState(state);
}
