#ifndef AKWINDOWBUTTON_H
#define AKWINDOWBUTTON_H

#include <AK/nodes/AKRenderableImage.h>

class AK::AKWindowButton : public AKRenderableImage
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

    void pointerButtonEvent(const AKPointerButtonEvent &e) override;

    AKSignal<> onClick;
protected:
    Type m_type;
    State m_state { Disabled };
};

#endif // AKWINDOWBUTTON_H
