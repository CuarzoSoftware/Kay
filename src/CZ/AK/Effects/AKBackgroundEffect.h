#ifndef CZ_AKBACKGROUNDEFFECT_H
#define CZ_AKBACKGROUNDEFFECT_H

#include <CZ/AK/Nodes/AKRenderable.h>
#include <CZ/Core/CZWeak.h>

class CZ::AKBackgroundEffect : public AKRenderable
{
public:
    enum StackPosition
    {
        /* Positioned right behind the target node */
        Behind,

        /* Positioned at the bottom of the target's parent (behind all siblings) */
        Background
    };

    enum Changes
    {
        CHStackPosition = AKRenderable::CHLast,

        CHLast
    };

    StackPosition stackPosition() const noexcept
    {
        return m_stackPosition;
    }

    void setStackPosition(StackPosition stackPosition) noexcept
    {
        if (m_stackPosition == stackPosition)
            return;

        addChange(CHStackPosition);
        m_stackPosition = stackPosition;
    }

    AKNode *targetNode() const noexcept
    {
        return m_targetNode;
    }

    ~AKBackgroundEffect()
    {
        if (targetNode())
            targetNode()->m_backgroundEffects.erase(this);
    }

protected:
    AKBackgroundEffect(StackPosition stackPosition) noexcept :
        AKRenderable(RenderableHint::Image),
        m_stackPosition(stackPosition)
    {
        m_caps |= BackgroundEffectBit;
    }

    // Rect relative to target
    SkIRect effectRect;

    virtual void onTargetNodeChanged() = 0;

    virtual void targetNodeRectCalculated()
    {
        effectRect = SkIRect::MakeSize(targetNode()->worldRect().size());
    }

private:
    friend class AKNode;
    friend class AKScene;
    using AKRenderable::layout;
    using AKRenderable::setParent;
    using AKRenderable::addBackgroundEffect;
    using AKRenderable::removeBackgroundEffect;
    StackPosition m_stackPosition;
    CZWeak<AKNode> m_targetNode;
};

#endif // CZ_AKBACKGROUNDEFFECT_H
