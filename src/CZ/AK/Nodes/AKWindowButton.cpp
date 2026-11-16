#include <CZ/Core/CZCore.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/AK/Nodes/AKWindowButton.h>
#include <CZ/AK/AKApp.h>
#include <CZ/AK/AKTheme.h>

#include <CZ/AK/AKScene.h>

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
    // Standalone (an AKWindowButtonGroup takes its buttons' input)
    if (state() == State::Disabled || e.button != BTN_LEFT)
        return;

    e.accept();

    if (e.pressed)
    {
        setState(Pressed);

        if (scene())
            scene()->setPointerGrab(this);

        return;
    }

    if (state() != Pressed)
        return;

    if (scene() && scene()->pointerGrab() == this)
        scene()->setPointerGrab(nullptr);

    setState(Normal);

    if (worldRect().contains(SkScalarFloorToInt(AKApp::Get()->pointer().pos().x()),
                             SkScalarFloorToInt(AKApp::Get()->pointer().pos().y())))
        onClick.notify();
}
