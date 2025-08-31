#ifndef CZ_AKSCENECHANGEDEVENT_H
#define CZ_AKSCENECHANGEDEVENT_H

#include <CZ/Events/CZEvent.h>
#include <CZ/Core/CZWeak.h>
#include <CZ/AK/AK.h>

class CZ::AKSceneChangedEvent final : public CZEvent
{
public:
    CZ_EVENT_DECLARE_COPY

    AKSceneChangedEvent(AKScene *oldScene, AKScene *newScene) noexcept : CZEvent(Type::AKSceneChanged),
        oldScene(oldScene), newScene(newScene) {}

    CZWeak<AKScene> oldScene;
    CZWeak<AKScene> newScene;
};;

#endif // CZ_AKSCENECHANGEDEVENT_H
