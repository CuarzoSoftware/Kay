#ifndef MSUBSURFACEPRIVATE_H
#define MSUBSURFACEPRIVATE_H

#include <CZ/Marco/roles/MSubsurface.h>

class CZ::MSubsurface::Imp
{
public:
    Imp(MSubsurface &obj) noexcept;
    MSubsurface &obj;
    SkIPoint pos { 0, 0 };
    CZWeak<MSurface> parent;
    wl_subsurface *wlSubSurface { nullptr };
    std::list<MSubsurface*>::iterator parentLink;
    bool posChanged { false };
};

#endif // MSUBSURFACEPRIVATE_H
