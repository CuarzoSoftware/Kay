#ifndef AKEDGESHADOW_H
#define AKEDGESHADOW_H

#include <AK/nodes/AKRenderableImage.h>

class AK::AKEdgeShadow : public AKRenderableImage
{
public:
    AKEdgeShadow(AKNode *parent = nullptr) noexcept;
};

#endif // AKEDGESHADOW_H
