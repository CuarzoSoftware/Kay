#ifndef AKWINDOWSTATE_H
#define AKWINDOWSTATE_H

namespace AK
{
    enum AKWindowState
    {
        AKMaximized   = 1 << 1,
        AKFullscreen  = 1 << 2,
        AKResizing    = 1 << 3,
        AKActivated   = 1 << 4,
        AKTiledLeft   = 1 << 5,
        AKTiledRight  = 1 << 6,
        AKTiledTop    = 1 << 7,
        AKTiledBottom = 1 << 8,
        AKSuspended   = 1 << 9,
        AKAllWindowStates = AKMaximized | AKFullscreen | AKResizing | AKActivated |
                            AKTiledLeft | AKTiledRight | AKTiledTop | AKTiledBottom | AKSuspended
    };
}

#endif // AKWINDOWSTATE_H
