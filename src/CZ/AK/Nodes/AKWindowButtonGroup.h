#ifndef CZ_AKWINDOWBUTTONGROUP_H
#define CZ_AKWINDOWBUTTONGROUP_H

#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKWindowButton.h>

class CZ::AKWindowButtonGroup : public AKContainer
{
public:
    AKWindowButtonGroup(AKNode *parent) noexcept;
    AKWindowButton closeButton { AKWindowButton::Type::Close, this };
    AKWindowButton minimizeButton { AKWindowButton::Type::Minimize, this };
    AKWindowButton maximizeButton { AKWindowButton::Type::Fullscreen, this };
protected:
    bool eventFilter(const CZEvent &e, CZObject &o) noexcept override;
    void sceneChangedEvent(const AKSceneChangedEvent &e) override;
    void pointerEnterEvent(const CZPointerEnterEvent &e) override;
    void pointerLeaveEvent(const CZPointerLeaveEvent &e) override;
    void pointerButtonEvent(const CZPointerButtonEvent &e) override;
    void windowStateEvent(const CZWindowStateEvent &e) override;
};

#endif // CZ_AKWINDOWBUTTONGROUP_H
