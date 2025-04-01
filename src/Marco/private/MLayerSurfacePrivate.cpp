#include <Marco/private/MLayerSurfacePrivate.h>
#include <Marco/private/MSurfacePrivate.h>

using namespace AK;

void MLayerSurface::Imp::configure(void *data, zwlr_layer_surface_v1 *layerSurface, UInt32 serial, UInt32 width, UInt32 height)
{
    auto &role { *static_cast<MLayerSurface*>(data) };
    role.MSurface::imp()->flags.add(MSurface::Imp::PendingConfigureAck);
    role.imp()->configureSerial = serial;

    bool notifyStates { role.MSurface::imp()->flags.check(MSurface::Imp::PendingFirstConfigure) };
    bool notifySuggestedSize { notifyStates };
    role.MSurface::imp()->flags.remove(MSurface::Imp::PendingFirstConfigure);

    const SkISize suggestedSize (width, height);

    if (role.suggestedSize() != suggestedSize )
    {
        role.imp()->suggestedSize = suggestedSize;
        notifySuggestedSize = true;
    }

    if (notifySuggestedSize)
    {
        role.suggestedSizeChanged();
    }

    if (notifyStates)
    {
        role.MSurface::imp()->flags.add(MSurface::Imp::ForceUpdate);
        role.update();
    }
}

void MLayerSurface::Imp::closed(void *data, zwlr_layer_surface_v1 *layerSurface)
{

}


MLayerSurface::Imp::Imp(MLayerSurface &obj) noexcept : obj(obj)
{
    obj.MSurface::imp()->flags.add(MSurface::Imp::PendingNullCommit);
    layerSurfaceListener.configure = &configure;
    layerSurfaceListener.closed = &closed;
}
