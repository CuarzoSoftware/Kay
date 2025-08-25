#ifndef MROOTSURFACENODE_H
#define MROOTSURFACENODE_H

#include <CZ/Marco/Marco.h>
#include <CZ/AK/Nodes/AKContainer.h>

class CZ::MRootSurfaceNode : public AKContainer
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
