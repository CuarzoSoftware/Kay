#include <AK/events/AKSceneChangedEvent.h>
#include <AK/AKScene.h>

using namespace AK;

AKSceneChangedEvent::AKSceneChangedEvent(AKScene *oldScene, AKScene *newScene, UInt32 serial, UInt32 ms, UInt64 us) noexcept :
    AKEvent(SceneChanged, serial, ms, us), m_old(oldScene), m_new(newScene)
{}

void AKSceneChangedEvent::setOldScene(AKScene *scene) noexcept
{
    m_old.reset(scene);
}

void AKSceneChangedEvent::setNewScene(AKScene *scene) noexcept
{
    m_new.reset(scene);
}

AKScene *AKSceneChangedEvent::oldScene() const noexcept
{
    return m_old;
}

AKScene *AKSceneChangedEvent::newScene() const noexcept
{
    return m_new;
}
