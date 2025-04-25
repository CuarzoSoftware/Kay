#ifndef AKEDGESHADOW_H
#define AKEDGESHADOW_H

#include <AK/nodes/AKRenderableImage.h>
#include <AK/AKEdge.h>

class AK::AKEdgeShadow : public AKRenderableImage
{
public:

    enum Changes
    {
        CHEdge = AKRenderableImage::CHLast,
        CHLast
    };

    AKEdgeShadow(AKEdge edge, AKNode *parent = nullptr) noexcept;
    AKCLASS_NO_COPY(AKEdgeShadow)

    void setEdge(AKEdge edge) noexcept;
    AKEdge edge() const noexcept { return m_edge; };

protected:
    AKEdge m_edge;
    void layoutEvent(const AKLayoutEvent &event) override;
};

#endif // AKEDGESHADOW_H
