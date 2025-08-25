#ifdef CZ_MARCO_ENABLED
#ifndef CZ_AKWINDOWBUTTON_H
#define CZ_AKWINDOWBUTTON_H

#include <CZ/AK/Nodes/AKImage.h>

class CZ::AKWindowButton : public AKImage
{
public:

    enum Type
    {
        Close,
        Minimize,
        Maximize,
        Fullscreen,
        UnsetFullscreen,
    };

    enum State
    {
        Disabled,
        Normal,
        Hover,
        Pressed
    };

    AKWindowButton(Type type, AKNode *parent = nullptr) noexcept;
    bool setType(Type type) noexcept;
    Type type() const noexcept { return m_type; }

    bool setState(State state) noexcept;
    State state() const noexcept { return m_state; }

    void pointerButtonEvent(const CZPointerButtonEvent &e) override;

    CZSignal<> onClick;
protected:
    Type m_type;
    State m_state { Disabled };
};

#endif // CZ_AKWINDOWBUTTON_H
#endif