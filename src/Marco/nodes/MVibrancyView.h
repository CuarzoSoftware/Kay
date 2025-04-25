#ifndef MVIBRANCYVIEW_H
#define MVIBRANCYVIEW_H

#include <AK/nodes/AKSolidColor.h>
#include <Marco/Marco.h>

class AK::MVibrancyView : public AKSolidColor
{
public:

    MVibrancyView(AKNode *parent = nullptr) noexcept :
        AKSolidColor(0x00000000, parent)
    {}

    bool vibrancyEnabled() const noexcept { return m_vibrancyEnabled; };
    SkColor disabledColor() const noexcept { return m_disabledColor; };

private:
    bool event(const AKEvent &event) override;
    bool m_vibrancyEnabled { true };
    SkColor m_disabledColor { 0xFFe6e7e7 };
};

#endif // MVIBRANCYVIEW_H
