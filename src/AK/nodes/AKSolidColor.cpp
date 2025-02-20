#include <AK/nodes/AKSolidColor.h>
#include <AK/events/AKRenderEvent.h>
#include <AK/AKPainter.h>

using namespace AK;

void AKSolidColor::renderEvent(const AKRenderEvent &p)
{
    p.painter.bindColorMode();
    p.painter.drawRegion(p.damage);
}
