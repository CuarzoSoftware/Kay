#ifndef AKTOPLEVELP_H
#define AKTOPLEVELP_H

#include <AKToplevel.h>
#include <xdg-shell-client-protocol.h>

using namespace AK;

class AKToplevel::Private
{
public:
    xdg_surface *xdgSurface { nullptr };
    xdg_toplevel *xdgToplevel { nullptr };
};


#endif // AKTOPLEVELP_H
