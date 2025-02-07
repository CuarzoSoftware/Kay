#include <AK/nodes/AKSolidColor.h>
#include <AK/AKPainter.h>

using namespace AK;

void AKSolidColor::onRender(AKPainter *painter, const SkRegion &damage, const SkIRect &/*rect*/)
{
    painter->bindColorMode();
    painter->drawRegion(damage);
}
