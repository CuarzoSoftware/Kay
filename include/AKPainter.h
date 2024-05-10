#ifndef AKPAINTER_H
#define AKPAINTER_H

#include <AKObject.h>

class AK::AKPainter : public AKObject
{
public:
    AKPainter(AKFrame *frame) noexcept;
    void clear(const AKColorF &color) noexcept;
    void bind() noexcept;

    void *backend() const noexcept
    {
        return m_backend;
    }
private:
    void *m_backend { nullptr };
};

#endif // AKPAINTER_H
