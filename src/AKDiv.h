#ifndef AKDIV_H
#define AKDIV_H

#include <AKBakeable.h>

class AK::AKDiv : public AKBakeable
{
public:
    AKDiv(AKNode *parent = nullptr) noexcept : AKBakeable(parent) {};
    void onBake(SkCanvas *canvas, const SkRect &clip, bool surfaceChanged) override;
private:
    struct Data
    {
        SkRegion m_baked;
        SkISize m_prevSize;
    };
    std::unordered_map<AKTarget*,Data> m_data;
};

#endif // AKDIV_H
