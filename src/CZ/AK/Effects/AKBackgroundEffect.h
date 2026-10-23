#ifndef CZ_AKBACKGROUNDEFFECT_H
#define CZ_AKBACKGROUNDEFFECT_H

#include <CZ/AK/Nodes/AKRenderable.h>
#include <CZ/Core/CZWeak.h>

/**
 * @brief Base class for effects rendered behind a node (shadows, blur, etc.).
 *
 * A background effect is not part of the layout: it is attached to a *target node*
 * with AKNode::addBackgroundEffect() and rendered behind it, at the position given
 * by stackPosition(). Its rect is defined by `effectRect`, relative to the target
 * node (e.g. inflated to make room for a shadow).
 *
 * See @ref creating_components_page "Creating Components" for how to implement one.
 *
 * @ingroup AKEffects
 */
class CZ::AKBackgroundEffect : public AKRenderable
{
public:
    /**
     * @brief Where the effect is stacked relative to the target node.
     */
    enum StackPosition
    {
        /// Positioned right behind the target node.
        Behind,

        /// Positioned at the bottom of the target's parent (behind all siblings).
        Background
    };

    /**
     * @brief Changes tracked in addition to AKRenderable::Changes.
     */
    enum Changes
    {
        CHStackPosition = AKRenderable::CHLast, ///< stackPosition() changed.

        CHLast ///< Marker for subclass chaining.
    };

    /**
     * @brief The stacking position relative to the target node.
     */
    StackPosition stackPosition() const noexcept
    {
        return m_stackPosition;
    }

    /**
     * @brief Sets the stacking position relative to the target node.
     */
    void setStackPosition(StackPosition stackPosition) noexcept
    {
        if (m_stackPosition == stackPosition)
            return;

        addChange(CHStackPosition);
        m_stackPosition = stackPosition;
    }

    /**
     * @brief The node this effect is attached to, or `nullptr` (see AKNode::addBackgroundEffect()).
     */
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
    /**
     * @brief Constructor.
     *
     * @param stackPosition The initial stacking position.
     */
    AKBackgroundEffect(StackPosition stackPosition) noexcept :
        AKRenderable(RenderableHint::Image),
        m_stackPosition(stackPosition)
    {
        m_caps |= BackgroundEffectBit;
    }

    /**
     * @brief The effect rect, relative to the target node's top-left corner.
     */
    SkIRect effectRect;

    /**
     * @brief Triggered when the effect is attached to or detached from a target node.
     */
    virtual void onTargetNodeChanged() = 0;

    /**
     * @brief Triggered once the target node's rect is calculated during AKScene::render().
     *
     * Override to set `effectRect` (e.g. inflate it to make room for a shadow).
     * The default implementation matches the target node's size.
     */
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
