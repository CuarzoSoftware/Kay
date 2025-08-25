#include <CZ/Marco/nodes/MVibrancyView.h>
#include <CZ/AK/Events/AKVibrancyEvent.h>

using namespace CZ;

bool MVibrancyView::event(const CZEvent &event)
{
    if (event.type() == AKVibrancyEvent::Vibrancy)
    {
        const auto &e { static_cast<const AKVibrancyEvent&>(event) };
        if (e.vibrancyState() == AKVibrancyState::Enabled)
            setColorWithAlpha(0x00000000);
        else
            setColorWithAlpha(disabledColor());

        return true;
    }

    return AKSolidColor::event(event);
}
