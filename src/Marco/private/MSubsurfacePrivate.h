#ifndef MSUBSURFACEPRIVATE_H
#define MSUBSURFACEPRIVATE_H

#include <Marco/roles/MSubsurface.h>

class AK::MSubsurface::Imp
{
public:
    Imp(MSubsurface &obj) noexcept;
    MSubsurface &obj;
    SkIPoint pos { 0, 0 };
    AKWeak<MSurface> parent;
    wl_subsurface *wlSubSurface { nullptr };
    std::list<MSubsurface*>::iterator parentLink;
    bool posChanged { false };
};

#endif // MSUBSURFACEPRIVATE_H
