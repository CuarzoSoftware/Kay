#include <Marco/nodes/MVibrancyView.h>
#include <AK/events/AKVibrancyEvent.h>

using namespace AK;

bool MVibrancyView::event(const AKEvent &event)
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
