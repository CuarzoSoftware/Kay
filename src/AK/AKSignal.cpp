#include <AK/AKSignal.h>
#include <AK/AKObject.h>

using namespace AK;

AKListener::~AKListener()
{
    signal->listeners[signalLink] = signal->listeners.back();
    signal->listeners[signalLink]->signalLink = signalLink;
    signal->listeners.pop_back();

    object->m_listeners[objectLink] = object->m_listeners.back();
    object->m_listeners[objectLink]->objectLink = objectLink;
    object->m_listeners.pop_back();
}

AKListener::AKListener(AKObject *object, AKSignalBase *signal) noexcept :
    signal(signal),
    object(object)
{
    object->m_listeners.push_back(this);
    objectLink = object->m_listeners.size() - 1;
    signalLink = signal->listeners.size();
}
