#ifndef MVIBRANCYVIEW_H
#define MVIBRANCYVIEW_H

#include <CZ/AK/Nodes/AKSolidColor.h>
#include <CZ/Marco/Marco.h>

class CZ::MVibrancyView : public AKSolidColor
{
public:

    MVibrancyView(AKNode *parent = nullptr) noexcept :
        AKSolidColor(0x00000000, parent)
    {}

    bool vibrancyEnabled() const noexcept { return m_vibrancyEnabled; };
    SkColor disabledColor() const noexcept { return m_disabledColor; };

private:
    bool event(const CZEvent &event) noexcept override;
    bool m_vibrancyEnabled { true };
    SkColor m_disabledColor { 0xFFe6e7e7 };
};

#endif // MVIBRANCYVIEW_H
