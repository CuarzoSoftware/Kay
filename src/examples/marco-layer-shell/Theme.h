#ifndef THEME_H
#define THEME_H

#include <Marco/MTheme.h>

using namespace AK;

class Theme : public MTheme
{
public:
    using MTheme::MTheme;

    static inline Int32     DockHeight                         { 96 };
    static inline Int32     DockBorderRadius                   { 24 };
    static inline Int32     DockShadowRadius                   { 32 };
    static inline Int32     DockShadowOffsetY                  { 8 };
    static inline SkRect    DockHThreePatchSideSrcRect         { SkRect::MakeWH((DockBorderRadius + DockShadowRadius) , DockHeight + 2.f * DockShadowRadius) };
    static inline SkRect    DockHThreePatchCenterSrcRect       { SkRect::MakeXYWH(DockHThreePatchSideSrcRect.width(), 0.f, 1.f, DockHThreePatchSideSrcRect.height()) };
    virtual sk_sp<SkImage>  dockHThreePatchImage(Int32 scale) noexcept;

protected:
    std::unordered_map<Int32,sk_sp<SkImage>> m_dockHThreePatchImage;
};

#endif // THEME_H
