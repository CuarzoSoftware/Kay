#ifndef AKNODE_H
#define AKNODE_H

#include <AKObject.h>
#include <include/core/SkRegion.h>
#include <memory>
#include <list>

class AK::AKNode : public AKObject
{
public:

    enum Caps : UInt32
    {
        Render  = 1 << 0,
        Bake    = 1 << 1
    };

    virtual ~AKNode();

    AKNode *parent() const noexcept
    {
        return m_parent;
    }

    const std::list<AKNode*> &children() const noexcept
    {
        return m_children;
    }

    UInt32 caps() const noexcept
    {
        return m_caps;
    }

    void setInputRegion(SkRegion *region) noexcept
    {
        m_inputRegion = region ? std::make_unique<SkRegion>(*region) : nullptr;
    }

    SkRegion *inputRegion() const noexcept
    {
        return m_inputRegion.get();
    }

private:
    friend class AKRenderable;
    friend class AKBakeable;
    friend class AKLayout;
    AKNode(AKNode *parent = nullptr) noexcept;
    UInt32 m_caps;
    YGNodeRef m_node { nullptr };
    AKNode *m_parent { nullptr };
    std::list<AKNode*> m_children;
    std::list<AKNode*>::iterator m_parentLink;
    std::unique_ptr<SkRegion> m_inputRegion;
};

#endif // AKNODE_H
