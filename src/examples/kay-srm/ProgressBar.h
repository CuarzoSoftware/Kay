#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <CZ/AK/AKTime.h>
#include <CZ/AK/Nodes/AKBakeable.h>

using namespace CZ;

class ProgressBar : public AKBakeable
{
public:

    enum Changes
    {
        CHPercent = AKBakeable::Changes::CHLast
    };

    ProgressBar(AKNode *parent = nullptr) noexcept;

    bool setPercent(SkScalar percent) noexcept;
    SkScalar percent() const noexcept { return m_percent; };

    void bakeEvent(const AKBakeEvent &e) override;

    // Just for the animaton
    SkScalar startTime = AKTime::ms();
    SkScalar duration { 10000.f };
protected:
    SkScalar m_percent { 0.f };
};

#endif // PROGRESSBAR_H
