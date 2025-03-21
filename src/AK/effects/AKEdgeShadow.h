#ifndef AKEDGESHADOW_H
#define AKEDGESHADOW_H

#include <AK/nodes/AKRenderableImage.h>

class AK::AKEdgeShadow : public AKRenderableImage
{
public:
    AKEdgeShadow(AKNode *parent = nullptr) noexcept;
    AKCLASS_NO_COPY(AKEdgeShadow)

protected:
    void layoutEvent(const AKLayoutEvent &event) override;
};

#endif // AKEDGESHADOW_H
