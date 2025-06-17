#ifndef AKBRUSH_H
#define AKBRUSH_H

#include <AK/AK.h>
#include <CZ/skia/core/SkPaint.h>

class AK::AKBrush : public SkPaint
{
public:
    AKBrush(bool enabled = true, bool autoBlendMode = true) noexcept :
        enabled(enabled),
        autoBlendMode(autoBlendMode)
    {
        setStroke(false);
    }

    static const AKBrush& NoBrush() noexcept
    {
        static AKBrush noBrush { false };
        return noBrush;
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

#endif // AKBRUSH_H
