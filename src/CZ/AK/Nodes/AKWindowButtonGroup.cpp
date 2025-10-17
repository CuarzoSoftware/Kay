#include <CZ/Core/Events/CZWindowStateEvent.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZKeyboardKeyEvent.h>
#include <CZ/AK/Events/AKSceneChangedEvent.h>
#include <CZ/AK/Nodes/AKWindowButtonGroup.h>
#include <CZ/AK/AKScene.h>
#include <CZ/AK/AKTheme.h>

#ifdef CZ_ENABLE_MARCO
#include <CZ/Marco/roles/MToplevel.h>
#endif

using namespace CZ;

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

bool AKWindowButtonGroup::eventFilter(const CZEvent &ev, CZObject &o) noexcept
{
    // TODO: Fix this
    CZ_UNUSED(ev)
    CZ_UNUSED(o)
    
#ifdef CZ_ENABLE_MARCO

    if (ev.type() == CZEvent::Type::KeyboardKey && &o == (AKObject*)scene() && window() && window()->role() == MSurface::Role::Toplevel)
    {
        if (!static_cast<MToplevel*>(window())->fullscreen())
        {
            const auto &e { static_cast<const CZKeyboardKeyEvent &>(ev) };

            if (e.keyCode == KEY_LEFTALT)
            {
                if (e.pressed)
                    maximizeButton.setType(AKWindowButton::Maximize);
                else
                    maximizeButton.setType(AKWindowButton::Fullscreen);
            }
        }
    }
#endif

    return false;
}

void AKWindowButtonGroup::sceneChangedEvent(const AKSceneChangedEvent &e)
{
    if (e.oldScene)
        e.oldScene->removeEventFilter(this);

    if (e.newScene)
        e.newScene->installEventFilter(this);
}

void AKWindowButtonGroup::pointerEnterEvent(const CZPointerEnterEvent &)
{
    closeButton.setState(AKWindowButton::State::Hover);
    maximizeButton.setState(AKWindowButton::State::Hover);

#ifdef CZ_ENABLE_MARCO
    MToplevel *tl = dynamic_cast<MToplevel*>(window());

    if (tl && tl->fullscreen())
        minimizeButton.setState(AKWindowButton::State::Disabled);
    else
        minimizeButton.setState(AKWindowButton::State::Hover);
#endif
}

void AKWindowButtonGroup::pointerLeaveEvent(const CZPointerLeaveEvent &)
{
    const auto state { activated() ? AKWindowButton::State::Normal : AKWindowButton::State::Disabled };
    closeButton.setState(state);
    maximizeButton.setState(state);

#ifdef CZ_ENABLE_MARCO
    MToplevel *tl = dynamic_cast<MToplevel*>(window());

    if (tl && tl->fullscreen())
        minimizeButton.setState(AKWindowButton::State::Disabled);
    else
        minimizeButton.setState(state);
#endif
}

void AKWindowButtonGroup::pointerButtonEvent(const CZPointerButtonEvent &e)
{
    if (e.button != BTN_LEFT)
        return;

    if (!e.pressed)
    {
        e.accept();

        const auto state { AKWindowButton::State::Hover };
        closeButton.setState(state);
        maximizeButton.setState(state);

#ifdef CZ_ENABLE_MARCO

        MToplevel *tl = dynamic_cast<MToplevel*>(window());

        if (tl && tl->fullscreen())
            minimizeButton.setState(AKWindowButton::State::Disabled);
        else
            minimizeButton.setState(state);
#endif

    }
}

void AKWindowButtonGroup::windowStateEvent(const CZWindowStateEvent &e)
{
    const auto state { activated() ? AKWindowButton::State::Normal : AKWindowButton::State::Disabled };
    closeButton.setState(state);
    maximizeButton.setState(state);

    if (e.newState.has(CZWinFullscreen))
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
