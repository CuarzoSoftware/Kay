#ifndef AKPEN_H
#define AKPEN_H

#include <AK/AK.h>
#include <CZ/skia/core/SkPaint.h>

class AK::AKPen : public SkPaint
{
public:
    AKPen(bool enabled = true, bool autoBlendMode = true) noexcept :
        enabled(enabled),
        autoBlendMode(autoBlendMode)
    {
        setStroke(true);
    }

    static const AKPen& NoPen() noexcept
    {
        static AKPen noPen { false };
        return noPen;
    };

    bool nothingToDraw() const noexcept
    {
        return SkPaint::nothingToDraw() || !enabled;
    }

    bool enabled;
    bool autoBlendMode;

private:
    using SkPaint::setStyle;
    using SkPaint::setStroke;
};

#endif // AKPEN_H
