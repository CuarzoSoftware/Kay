#include <CZ/Core/Events/CZWindowStateEvent.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZKeyboardKeyEvent.h>
#include <CZ/Core/CZWeak.h>
#include <CZ/AK/Events/AKSceneChangedEvent.h>
#include <CZ/AK/Nodes/AKWindowButtonGroup.h>
#include <CZ/AK/AKScene.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKApp.h>
#include <linux/input-event-codes.h>
#include <cmath>

using namespace CZ;

AKWindowButtonGroup::AKWindowButtonGroup(AKNode *parent) noexcept :
    AKContainer(YGFlexDirectionRow, true, parent)
{
    layout().setGap(YGGutterAll, AKTheme::WindowButtonGap);
    layout().setHeight(AKTheme::WindowButtonSize.height());

    // The group takes the input (the gaps between the buttons too): see buttonAtPointer()
    const SkRegion empty;

    for (AKWindowButton *b : { &closeButton, &minimizeButton, &maximizeButton })
        b->setInputRegion(&empty);

    updateButtons();
}

bool AKWindowButtonGroup::fullscreen() const noexcept
{
    return scene() && scene()->windowState().has(CZWinFullscreen);
}

void AKWindowButtonGroup::updateButtons() noexcept
{
    // Third button: fullscreen (maximize while Alt is held), exit fullscreen while fullscreen
    const bool fs { fullscreen() };
    maximizeButton.setType(fs ? AKWindowButton::UnsetFullscreen :
                           m_alt ? AKWindowButton::Maximize : AKWindowButton::Fullscreen);

    for (AKWindowButton *b : { &closeButton, &minimizeButton, &maximizeButton })
    {
        AKWindowButton::State state { activated() ? AKWindowButton::Normal : AKWindowButton::Disabled };

        if (b == &minimizeButton && fs)
            state = AKWindowButton::Disabled; // Can't minimize a fullscreen window
        else if (b == m_pressed && m_pressedInside)
            state = AKWindowButton::Pressed;
        else if (m_hover)
            state = AKWindowButton::Hover;

        b->setState(state);
    }
}

bool AKWindowButtonGroup::pointerInside() const noexcept
{
    const SkPoint &pos { AKApp::Get()->pointer().pos() };
    return SkRect::Make(worldRect()).contains(pos.x(), pos.y());
}

AKWindowButton *AKWindowButtonGroup::buttonAtPointer() const noexcept
{
    const SkPoint &pos { AKApp::Get()->pointer().pos() };

    for (const AKWindowButton *b : { &closeButton, &minimizeButton, &maximizeButton })
        if (SkRect::Make(b->worldRect()).contains(pos.x(), pos.y()))
            return const_cast<AKWindowButton*>(b);

    return nullptr;
}

void AKWindowButtonGroup::updatePointer() noexcept
{
    // While a button is held the group holds the pointer (see pointerButtonEvent()): hovering
    // follows where it actually is
    const bool hover { pointerInside() };
    const bool pressedInside { m_pressed && buttonAtPointer() == m_pressed };

    if (hover == m_hover && pressedInside == m_pressedInside)
        return;

    m_hover = hover;
    m_pressedInside = pressedInside;
    updateButtons();
}

bool AKWindowButtonGroup::eventFilter(const CZEvent &ev, CZObject &o) noexcept
{
    if (&o != (CZObject*)scene())
        return false;

    if (ev.type() == CZEvent::Type::KeyboardKey)
    {
        const auto &e { static_cast<const CZKeyboardKeyEvent &>(ev) };

        if ((e.code == KEY_LEFTALT || e.code == KEY_RIGHTALT) && m_alt != e.isPressed)
        {
            m_alt = e.isPressed;
            updateButtons();
        }
    }
    else if (ev.type() == CZEvent::Type::KeyboardLeave && m_alt)
    {
        // Its release won't be seen
        m_alt = false;
        updateButtons();
    }

    return false;
}

void AKWindowButtonGroup::sceneChangedEvent(const AKSceneChangedEvent &e)
{
    if (e.oldScene)
    {
        e.oldScene->removeEventFilter(this);

        if (e.oldScene->pointerGrab() == this)
            e.oldScene->setPointerGrab(nullptr);
    }

    m_pressed = nullptr;
    m_pressedInside = false;
    m_hover = false;
    m_alt = false;

    if (e.newScene)
        e.newScene->installEventFilter(this);

    updateButtons();
}

void AKWindowButtonGroup::pointerEnterEvent(const CZPointerEnterEvent &)
{
    updatePointer();
}

void AKWindowButtonGroup::pointerMoveEvent(const CZPointerMoveEvent &)
{
    updatePointer();
}

void AKWindowButtonGroup::pointerLeaveEvent(const CZPointerLeaveEvent &)
{
    // Left the window while held: the press is dropped (its release may never come)
    if (m_pressed)
    {
        m_pressed = nullptr;

        if (scene() && scene()->pointerGrab() == this)
            scene()->setPointerGrab(nullptr);
    }

    m_hover = false;
    m_pressedInside = false;
    updateButtons();
}

void AKWindowButtonGroup::pointerButtonEvent(const CZPointerButtonEvent &e)
{
    if (e.button != BTN_LEFT)
        return;

    e.accept();

    if (e.pressed)
    {
        AKWindowButton *button { buttonAtPointer() };

        if (m_pressed || !button || button->state() == AKWindowButton::Disabled)
            return;

        // Held until released (wherever the pointer goes)
        m_pressed = button;
        m_pressedInside = true;

        if (scene())
            scene()->setPointerGrab(this);

        updateButtons();
        return;
    }

    if (!m_pressed)
        return;

    AKWindowButton *button { m_pressed };
    const bool clicked { buttonAtPointer() == button };
    m_pressed = nullptr;
    m_pressedInside = false;

    if (scene() && scene()->pointerGrab() == this)
        scene()->setPointerGrab(nullptr);

    // Minimizing: the window goes away under the pointer (no leave event may follow)
    if (clicked && button == &minimizeButton)
        m_hover = false;
    else
        m_hover = pointerInside();

    updateButtons();

    if (clicked)
        button->onClick.notify(); // May destroy it (e.g. closing the window)
}

void AKWindowButtonGroup::windowStateEvent(const CZWindowStateEvent &)
{
    updateButtons();
}
