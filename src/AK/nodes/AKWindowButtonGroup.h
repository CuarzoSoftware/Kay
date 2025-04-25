#ifndef AKWINDOWBUTTONGROUP_H
#define AKWINDOWBUTTONGROUP_H

#include <AK/nodes/AKContainer.h>
#include <AK/nodes/AKWindowButton.h>

class AK::AKWindowButtonGroup : public AKContainer
{
public:
    AKWindowButtonGroup(AKNode *parent) noexcept;
    AKWindowButton closeButton { AKWindowButton::Type::Close, this };
    AKWindowButton minimizeButton { AKWindowButton::Type::Minimize, this };
    AKWindowButton maximizeButton { AKWindowButton::Type::Fullscreen, this };
protected:
    bool eventFilter(const AKEvent &e, AKObject &o) override;
    void sceneChangedEvent(const AKSceneChangedEvent &e) override;
    void pointerEnterEvent(const AKPointerEnterEvent &e) override;
    void pointerLeaveEvent(const AKPointerLeaveEvent &e) override;
    void pointerButtonEvent(const AKPointerButtonEvent &e) override;
    void windowStateEvent(const AKWindowStateEvent &e) override;
};

#endif // AKWINDOWBUTTONGROUP_H
