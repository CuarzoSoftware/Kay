#include <include/core/SkCanvas.h>
#include <AK/nodes/AKImage.h>
#include <AK/AKPen.h>
#include <AK/AKTarget.h>
#include <AK/AKPainter.h>

using namespace AK;

void AKImage::onRender(AKPainter *painter, const SkRegion &damage)
{
    if (image())
    {
        painter->bindTextureMode({
            .texture = image(),
            .pos = { rect().x(), rect().y() },
            .srcRect = srcRect(),
            .dstSize = rect().size(),
            .srcTransform = transform(),
            .srcScale = scale()
        });
    }
    else
        painter->bindColorMode();

    painter->drawRegion(damage);
}
