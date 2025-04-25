#ifndef AKSCENECHANGEDEVENT_H
#define AKSCENECHANGEDEVENT_H

#include <AK/events/AKTouchEvent.h>
#include <AK/AKWeak.h>
#include <AK/AKTime.h>

class AK::AKSceneChangedEvent final : public AKEvent
{
public:
    AKEVENT_DECLARE_COPY

    AKSceneChangedEvent(AKScene *oldScene, AKScene *newScene, UInt32 serial = AKTime::nextSerial(),
                        UInt32 ms = AKTime::ms(), UInt64 us = AKTime::us()) noexcept;

    void setOldScene(AKScene *scene) noexcept;
    void setNewScene(AKScene *scene) noexcept;

    AKScene *oldScene() const noexcept;
    AKScene *newScene() const noexcept;

protected:
    AKWeak<AKScene> m_old, m_new;
};

#endif // AKSCENECHANGEDEVENT_H
