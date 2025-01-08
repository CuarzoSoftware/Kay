#ifndef AKBACKGROUNDEFFECT_H
#define AKBACKGROUNDEFFECT_H

#include <AK/nodes/AKRenderable.h>
#include <AK/AKWeak.h>

class AK::AKBackgroundEffect : public AKRenderable
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
        Chg_StackPosition = AKRenderable::Chg_Last,

        Chg_Last
    };

    StackPosition stackPosition() const noexcept
    {
        return m_stackPosition;
    }

    void setStackPosition(StackPosition stackPosition) noexcept
    {
        if (m_stackPosition == stackPosition)
            return;

        addChange(Chg_StackPosition);
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
        AKRenderable(Texture),
        m_stackPosition(stackPosition)
    {
        m_caps |= BackgroundEffect;
    }

    // Rect relative to target
    SkIRect effectRect;

    virtual void onTargetNodeChanged() = 0;

    void onSceneCalculatedRect() override
    {
        effectRect = SkIRect::MakeSize(targetNode()->rect().size());
    }

    using AKRenderable::onRender;
    using AKRenderable::currentTarget;
private:
    friend class AKNode;
    friend class AKScene;
    using AKRenderable::layout;
    using AKRenderable::setParent;
    using AKRenderable::addBackgroundEffect;
    using AKRenderable::removeBackgroundEffect;
    StackPosition m_stackPosition;
    AKWeak<AKNode> m_targetNode;
};

#endif // AKBACKGROUNDEFFECT_H
