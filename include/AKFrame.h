#ifndef AKFRAME_H
#define AKFRAME_H

#include <AKWidget.h>
#include <memory>
#include "include/core/SkRegion.h"

class AK::AKFrame : public AKWidget
{
public:
    AKFrame(AKWidget *parent = nullptr) noexcept;
    ~AKFrame() noexcept;

    void render(AKFrame *frame = nullptr, SkRegion *clip = nullptr) noexcept;

    class Private;

    Private *imp() const noexcept
    {
        return m_imp.get();
    }
private:
    friend class AKSurface;
    AKFrame(AKSurface *surface, const AKSize &size) noexcept;
    std::unique_ptr<Private> m_imp;
};

#endif // AKFRAME_H
