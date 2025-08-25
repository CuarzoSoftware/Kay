#ifndef MARCO_H
#define MARCO_H

#include <CZ/AK/AK.h>

namespace CZ
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
    class MRootSurfaceNode;
    class MVibrancyView;

    /* Input */
    class MPointer;
    class MKeyboard;

    template <class T> class MProxy;

    MPointer &pointer() noexcept;
    MKeyboard &keyboard() noexcept;
};

#endif // MARCO_H
