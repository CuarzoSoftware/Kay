#ifndef AKDIV_H
#define AKDIV_H

#include <AKBakeable.h>

class AK::AKDiv : public AKBakeable
{
public:
    AKDiv(AKNode *parent = nullptr) noexcept : AKBakeable(parent) {};
    void onBake(SkCanvas *canvas) override;
};

#endif // AKDIV_H
