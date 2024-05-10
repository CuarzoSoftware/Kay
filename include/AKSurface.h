#ifndef AKSURFACE_H
#define AKSURFACE_H

#include <AKObject.h>
#include <AKPoint.h>
#include <memory>

// Base class for surface roles (toplevel, popup, etc)

class AK::AKSurface : public AKObject
{
public:
    ~AKSurface() noexcept;

    AKFrame *widget() const noexcept;

    class Private;

    Private *imp() const
    {
        return m_imp.get();
    }

protected:
    AKSurface(const AKSize &size = AKSize(256, 256)) noexcept;

private:
    std::unique_ptr<Private> m_imp;
};

#endif // AKSURFACE_H
