#include <AK/nodes/AKSolidColor.h>
#include <AK/AKPainter.h>

using namespace AK;

void AKSolidColor::onRender(const OnRenderParams &params)
{
    params.painter.bindColorMode();
    params.painter.drawRegion(params.damage);
}
