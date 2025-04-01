#ifndef MARCO_H
#define MARCO_H

#include <AK/AK.h>

namespace AK
{
    class MApplication;
    class MScreen;

    class MSurface;
    class MToplevel;
    class MPopup;
    class MLayerSurface;
    class MSubsurface;

    class MTheme;
    class MCSDShadow;

    /* Input */
    class MPointer;
    class MKeyboard;

    template <class T> class MProxy;

    inline MApplication *app() noexcept { return (MApplication*)akApp(); };
    MPointer &pointer() noexcept;
    MKeyboard &keyboard() noexcept;
};

#endif // MARCO_H
