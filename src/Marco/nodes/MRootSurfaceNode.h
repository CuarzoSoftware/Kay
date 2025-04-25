#ifndef MROOTSURFACENODE_H
#define MROOTSURFACENODE_H

#include <Marco/Marco.h>
#include <AK/nodes/AKContainer.h>

class AK::MRootSurfaceNode : public AKContainer
{
public:
    MRootSurfaceNode(MSurface &surface) noexcept :
        m_surface(surface) {}

    MSurface &surface() const noexcept
    {
        return m_surface;
    }
private:
    MSurface &m_surface;
};

#endif // MROOTSURFACENODE_H
