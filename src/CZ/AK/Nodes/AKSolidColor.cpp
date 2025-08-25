#include <CZ/AK/Nodes/AKSolidColor.h>
#include <CZ/AK/Events/AKRenderEvent.h>
#include <CZ/Ream/RPass.h>

using namespace CZ;

void AKSolidColor::renderEvent(const AKRenderEvent &e)
{
    auto *p { e.pass->getPainter() };
    p->drawColor(e.damage);
}
