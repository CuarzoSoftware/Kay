#ifndef CZ_AKSCENECHANGEDEVENT_H
#define CZ_AKSCENECHANGEDEVENT_H

#include <CZ/Core/Events/CZEvent.h>
#include <CZ/Core/CZWeak.h>
#include <CZ/AK/AK.h>

/**
 * @brief Notifies a node that the AKScene it belongs to changed.
 *
 * Posted by AKNode when the node (or one of its ancestors) is moved into a tree
 * whose root belongs to a different scene, and delivered through
 * AKNode::sceneChangedEvent().
 *
 * @ingroup AKEvents
 */
class CZ::AKSceneChangedEvent final : public CZEvent
{
public:
    CZ_EVENT_DECLARE_COPY

    /**
     * @brief Constructs a scene changed event.
     *
     * @param oldScene The scene the node belonged to before (can be `nullptr`).
     * @param newScene The scene the node now belongs to (can be `nullptr`).
     */
    AKSceneChangedEvent(AKScene *oldScene, AKScene *newScene) noexcept : CZEvent(Type::AKSceneChanged),
        oldScene(oldScene), newScene(newScene) {}

    /// The previous scene, or `nullptr`.
    CZWeak<AKScene> oldScene;

    /// The new scene, or `nullptr`.
    CZWeak<AKScene> newScene;
};;

#endif // CZ_AKSCENECHANGEDEVENT_H
