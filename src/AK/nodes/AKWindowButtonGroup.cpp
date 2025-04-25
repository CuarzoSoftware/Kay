#include <AK/events/AKWindowStateEvent.h>
#include <AK/events/AKPointerButtonEvent.h>
#include <AK/events/AKKeyboardKeyEvent.h>
#include <AK/events/AKSceneChangedEvent.h>
#include <AK/nodes/AKWindowButtonGroup.h>
#include <AK/AKScene.h>
#include <AK/AKTheme.h>

#include <Marco/roles/MToplevel.h>

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

bool AKWindowButtonGroup::eventFilter(const AKEvent &ev, AKObject &o)
{
    if (ev.type() == AKEvent::KeyboardKey && &o == (AKObject*)scene() && window() && window()->role() == MSurface::Role::Toplevel)
    {
        if (!static_cast<MToplevel*>(window())->fullscreen())
        {
            const auto &e { static_cast<const AKKeyboardKeyEvent &>(ev) };

            if (e.keyCode() == KEY_LEFTALT)
            {
                if (e.state() == AKKeyboardKeyEvent::Pressed)
                    maximizeButton.setType(AKWindowButton::Maximize);
                else
                    maximizeButton.setType(AKWindowButton::Fullscreen);
            }
        }
    }

    return false;
}

void AKWindowButtonGroup::sceneChangedEvent(const AKSceneChangedEvent &e)
{
    if (e.oldScene())
        e.oldScene()->removeEventFilter(this);

    if (e.newScene())
        e.newScene()->installEventFilter(this);
}

void AKWindowButtonGroup::pointerEnterEvent(const AKPointerEnterEvent &)
{
    closeButton.setState(AKWindowButton::State::Hover);
    maximizeButton.setState(AKWindowButton::State::Hover);

    MToplevel *tl = dynamic_cast<MToplevel*>(window());

    if (tl && tl->fullscreen())
        minimizeButton.setState(AKWindowButton::State::Disabled);
    else
        minimizeButton.setState(AKWindowButton::State::Hover);
}

void AKWindowButtonGroup::pointerLeaveEvent(const AKPointerLeaveEvent &)
{
    const auto state { activated() ? AKWindowButton::State::Normal : AKWindowButton::State::Disabled };
    closeButton.setState(state);
    maximizeButton.setState(state);
    MToplevel *tl = dynamic_cast<MToplevel*>(window());

    if (tl && tl->fullscreen())
        minimizeButton.setState(AKWindowButton::State::Disabled);
    else
        minimizeButton.setState(state);
}

void AKWindowButtonGroup::pointerButtonEvent(const AKPointerButtonEvent &e)
{
    if (e.button() != AKPointerButtonEvent::Left)
        return;

    if (e.state() == AKPointerButtonEvent::Released)
    {
        const auto state { AKWindowButton::State::Hover };
        closeButton.setState(state);
        maximizeButton.setState(state);
        MToplevel *tl = dynamic_cast<MToplevel*>(window());

        if (tl && tl->fullscreen())
            minimizeButton.setState(AKWindowButton::State::Disabled);
        else
            minimizeButton.setState(state);

    }
}

void AKWindowButtonGroup::windowStateEvent(const AKWindowStateEvent &e)
{
    const auto state { activated() ? AKWindowButton::State::Normal : AKWindowButton::State::Disabled };
    closeButton.setState(state);
    maximizeButton.setState(state);

    if (e.states().check(AKFullscreen))
    {
        maximizeButton.setType(AKWindowButton::Type::UnsetFullscreen);
        minimizeButton.setState(AKWindowButton::State::Disabled);
    }
    else
    {
        maximizeButton.setType(AKWindowButton::Type::Fullscreen);
        minimizeButton.setState(state);
    }
}
