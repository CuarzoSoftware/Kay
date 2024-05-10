#ifndef AKWIDGET_H
#define AKWIDGET_H

#include <AKObject.h>
#include <AKRect.h>
#include <list>

class AK::AKWidget : public AKObject
{
public:
    AKWidget(AKWidget *parent = nullptr) noexcept
    {
        setParent(parent);
    }

    const std::list<AKWidget*> &children() const noexcept
    {
        return m_children;
    }

    AKWidget *parent() const noexcept
    {
        return m_parent;
    }

    void setParent(AKWidget *parent) noexcept
    {
        // AKFrames with AKSurface backend can't have a parent
        if (m_surface)
            return;

        if (parent == this || parent == m_parent)
            return;

        if (m_parent)
            m_parent->m_children.erase(m_parentLink);

        m_parent = nullptr;

        if (parent)
        {
            m_parent = parent;
            m_parent->m_children.emplace_back(this);
            m_parentLink = std::prev(m_parent->m_children.end());
        }
    }

    virtual void paintEvent(AKPainter &painter) noexcept;

    const AKPoint &pos() const noexcept
    {
        return m_rect.pos();
    }

    const AKSize &size() const noexcept
    {
        return m_rect.size();
    }

    void setPos(const AKPoint &pos) noexcept;
    void setSize(const AKSize &size) noexcept;

    AKPoint frameRelativePos() const noexcept;

    const AKColorF &backgroundColor() const noexcept
    {
        return m_backgroundColor;
    }

    void setBackgroundColor(const AKColorF &color) noexcept
    {
        m_backgroundColor = color;
    }

    bool isFrame() const noexcept
    {
        return m_isFrame;
    }

private:
    friend class AKSurface;
    friend class AKFrame;
    friend class AKPainter;
    AKRect m_rect;
    bool m_isFrame { false };
    AKSurface *m_surface { nullptr };
    std::list<AKWidget*> m_children;
    std::list<AKWidget*>::iterator m_parentLink;
    AKWidget *m_parent { nullptr };
    AKColorF m_backgroundColor {0.f, 0.f, 0.f, 0.f};
};

#endif // AKWIDGET_H
