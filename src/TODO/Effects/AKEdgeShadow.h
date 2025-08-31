#ifndef CZ_AKEDGESHADOW_H
#define CZ_AKEDGESHADOW_H

#include <CZ/AK/Nodes/AKImage.h>
#include <CZ/Core/CZEdge.h>

class CZ::AKEdgeShadow : public AKImage
{
public:

    enum Changes
    {
        CHEdge = AKImage::CHLast,
        CHLast
    };

    AKEdgeShadow(CZEdge edge, AKNode *parent = nullptr) noexcept;
    CZ_DISABLE_COPY(AKEdgeShadow)

    void setEdge(CZEdge edge) noexcept;
    CZEdge edge() const noexcept { return m_edge; };

protected:
    CZEdge m_edge;
    void layoutEvent(const CZLayoutEvent &event) override;
};

#endif // CZ_AKEDGESHADOW_H
