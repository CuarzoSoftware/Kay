#ifndef AKTOPLEVEL_H
#define AKTOPLEVEL_H

#include <AKSurface.h>

class AK::AKToplevel : public AKSurface
{
public:

    AKToplevel(const AKSize &size = AKSize(256, 256)) noexcept;
    ~AKToplevel() noexcept;

    class Private;

    Private *imp() const noexcept
    {
        return m_imp.get();
    }
private:
    std::unique_ptr<Private> m_imp;
};

#endif // AKTOPLEVEL_H
