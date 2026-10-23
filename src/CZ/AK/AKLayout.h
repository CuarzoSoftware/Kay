#ifndef CZ_AKLAYOUT_H
#define CZ_AKLAYOUT_H

#include <CZ/AK/AK.h>
#include <CZ/Core/CZWeak.h>
#include <yoga/Yoga.h>

/**
 * @brief The Flexbox (Yoga) layout of a node.
 *
 * Each AKNode owns an AKLayout, accessible via AKNode::layout(), which wraps a
 * Yoga node. The setters and getters map 1:1 to the Yoga YGNodeStyleSet* /
 * YGNodeStyleGet* functions, and each setter that dirties the Yoga node
 * automatically flags the owner with AKNode::CHLayout so it is re-laid out and
 * repainted on the next AKScene::render().
 *
 * The scene calculates the layout during AKScene::render(). The calculated
 * results (position and size) are then available through AKNode::worldRect()
 * or the `calculated...()` getters, the latter being relative to the parent node.
 *
 * @ingroup AKCore
 */
class CZ::AKLayout
{
public:
    /** @brief The calculated width in logical coordinates. */
    float calculatedWidth() const noexcept
    {
        return YGNodeLayoutGetWidth(m_node);
    }

    /** @brief The calculated height in logical coordinates. */
    float calculatedHeight() const noexcept
    {
        return YGNodeLayoutGetHeight(m_node);
    }

    /** @brief The calculated left offset relative to the parent node in logical coordinates. */
    float calculatedLeft() const noexcept
    {
        return YGNodeLayoutGetLeft(m_node);
    }

    /** @brief The calculated top offset relative to the parent node in logical coordinates. */
    float calculatedTop() const noexcept
    {
        return YGNodeLayoutGetTop(m_node);
    }

    /** @brief The calculated right offset relative to the parent node in logical coordinates. */
    float calculatedRight() const noexcept
    {
        return YGNodeLayoutGetRight(m_node);
    }

    /** @brief The calculated bottom offset relative to the parent node in logical coordinates. */
    float calculatedBottom() const noexcept
    {
        return YGNodeLayoutGetBottom(m_node);
    }

    /** @brief The calculated layout direction (LTR or RTL). */
    YGDirection calculatedDirection() const noexcept
    {
        return YGNodeLayoutGetDirection(m_node);
    }

    /** @brief Whether the content overflowed the node during the last layout calculation. */
    bool calculatedHadOverflow() const noexcept
    {
        return YGNodeLayoutGetHadOverflow(m_node);
    }

    /** @brief The calculated margin for the given edge in logical coordinates. */
    float calculatedMargin(YGEdge edge) const noexcept
    {
        return YGNodeLayoutGetMargin(m_node, edge);
    }

    /** @brief The calculated border for the given edge in logical coordinates. */
    float calculatedBorder(YGEdge edge) const noexcept
    {
        return YGNodeLayoutGetBorder(m_node, edge);
    }

    /** @brief The calculated padding for the given edge in logical coordinates. */
    float calculatedPadding(YGEdge edge) const noexcept
    {
        return YGNodeLayoutGetPadding(m_node, edge);
    }

    /* Conf */

    /**
     * @brief Sets the point scale factor used to round layout results to physical pixels.
     *
     * Each AKLayout owns its own Yoga config, initialized with a factor of 1.
     */
    void setPointScaleFactor(float pixelsInPoint) noexcept
    {
        YGConfigSetPointScaleFactor(m_config, pixelsInPoint);
        YGNodeSetConfig(m_node, m_config);
        checkIsDirty();
    }

    /** @brief The current point scale factor. */
    float pointScaleFactor() const noexcept
    {
        return YGConfigGetPointScaleFactor(m_config);
    }

    /* Style */

    /** @brief Copies the entire style of another layout into this one. */
    void copyStyle(const AKLayout &from) noexcept
    {
        YGNodeCopyStyle(m_node, from.m_node);
        checkIsDirty();
    }

    /** @brief Sets the layout direction (LTR or RTL). */
    void setDirection(YGDirection direction) noexcept
    {
        YGNodeStyleSetDirection(m_node, direction);
        checkIsDirty();
    }

    /** @brief The layout direction style. */
    YGDirection direction() const noexcept
    {
        return YGNodeStyleGetDirection(m_node);
    }

    /** @brief Sets the flex-direction (main axis along which children are placed). */
    void setFlexDirection(YGFlexDirection flexDirection) noexcept
    {
        YGNodeStyleSetFlexDirection(m_node, flexDirection);
        checkIsDirty();
    }

    /** @brief The flex-direction. */
    YGFlexDirection flexDirection() const noexcept
    {
        return YGNodeStyleGetFlexDirection(m_node);
    }

    /** @brief Sets how children are justified along the main axis. */
    void setJustifyContent(YGJustify justifyContent) noexcept
    {
        YGNodeStyleSetJustifyContent(m_node, justifyContent);
        checkIsDirty();
    }

    /** @brief The justify-content value. */
    YGJustify justifyContent() const noexcept
    {
        return YGNodeStyleGetJustifyContent(m_node);
    }

    /** @brief Sets how wrapped lines are aligned along the cross axis. */
    void setAlignContent(YGAlign alignContent) noexcept
    {
        YGNodeStyleSetAlignContent(m_node, alignContent);
        checkIsDirty();
    }

    /** @brief The align-content value. */
    YGAlign alignContent() const noexcept
    {
        return YGNodeStyleGetAlignContent(m_node);
    }

    /** @brief Sets how children are aligned along the cross axis. */
    void setAlignItems(YGAlign alignItems) noexcept
    {
        YGNodeStyleSetAlignItems(m_node, alignItems);
        checkIsDirty();
    }

    /** @brief The align-items value. */
    YGAlign alignItems() const noexcept
    {
        return YGNodeStyleGetAlignItems(m_node);
    }

    /** @brief Sets this node's own cross-axis alignment, overriding the parent's align-items. */
    void setAlignSelf(YGAlign alignSelf) noexcept
    {
        YGNodeStyleSetAlignSelf(m_node, alignSelf);
        checkIsDirty();
    }

    /** @brief The align-self value. */
    YGAlign alignSelf() const noexcept
    {
        return YGNodeStyleGetAlignSelf(m_node);
    }

    /**
     * @brief Sets the position type (relative, absolute or static).
     *
     * Ignored while an anchor node is set (see setAnchorNode()), since the
     * anchor forces YGPositionTypeAbsolute.
     */
    void setPositionType(YGPositionType positionType) noexcept
    {
        if (anchorNode())
            return;

        YGNodeStyleSetPositionType(m_node, positionType);
        checkIsDirty();
    }

    /** @brief The position type. */
    YGPositionType positionType() const noexcept
    {
        return YGNodeStyleGetPositionType(m_node);
    }

    /** @brief Sets the position offset for the given edge in logical points. */
    void setPosition(YGEdge edge, float position) noexcept
    {
        YGNodeStyleSetPosition(m_node, edge, position);
        checkIsDirty();
    }

    /**
     * @brief Sets the position offset for the given edge as a percentage.
     *
     * @note Unlike the other setters, this one does not trigger the dirty check.
     */
    void positionPercent(YGEdge edge, float position) noexcept
    {
        YGNodeStyleSetPositionPercent(m_node, edge, position);
    }

    /** @brief The position offset for the given edge. */
    YGValue position(YGEdge edge) const noexcept
    {
        return YGNodeStyleGetPosition(m_node, edge);
    }

    /** @brief The width style (point, percent or auto). */
    YGValue width() const noexcept
    {
        return YGNodeStyleGetWidth(m_node);
    }

    /** @brief Sets the width in logical points. */
    void setWidth(float width) noexcept
    {
        YGNodeStyleSetWidth(m_node, width);
        checkIsDirty();
    }

    /** @brief Sets the width as a percentage of the parent's width. */
    void setWidthPercent(float width) noexcept
    {
        YGNodeStyleSetWidthPercent(m_node, width);
        checkIsDirty();
    }

    /** @brief Sets the width to auto. */
    void setWidthAuto() noexcept
    {
        YGNodeStyleSetWidthAuto(m_node);
        checkIsDirty();
    }

    /** @brief Sets the width from a YGValue, dispatching to the auto, point or percent setter. */
    void setWidthYGValue(YGValue value) noexcept
    {
        switch (value.unit)
        {
        case YGUnitAuto:
            setWidthAuto();
            break;
        case YGUnitPoint:
            setWidth(value.value);
            break;
        case YGUnitPercent:
            setWidthPercent(value.value);
            break;
        default:
            break;
        }
    }

    /** @brief The height style (point, percent or auto). */
    YGValue height() const noexcept
    {
        return YGNodeStyleGetHeight(m_node);
    }

    /** @brief Sets the height in logical points. */
    void setHeight(float height) noexcept
    {
        YGNodeStyleSetHeight(m_node, height);
        checkIsDirty();
    }

    /** @brief Sets the height as a percentage of the parent's height. */
    void setHeightPercent(float height) noexcept
    {
        YGNodeStyleSetHeightPercent(m_node, height);
        checkIsDirty();
    }

    /** @brief Sets the height to auto. */
    void setHeightAuto() noexcept
    {
        YGNodeStyleSetHeightAuto(m_node);
        checkIsDirty();
    }

    /** @brief Sets the height from a YGValue, dispatching to the auto, point or percent setter. */
    void setHeightYGValue(YGValue value) noexcept
    {
        switch (value.unit)
        {
        case YGUnitAuto:
            setHeightAuto();
            break;
        case YGUnitPoint:
            setHeight(value.value);
            break;
        case YGUnitPercent:
            setHeightPercent(value.value);
            break;
        default:
            break;
        }
    }

    /** @brief Sets the minimum width in logical points. */
    void setMinWidth(float minWidth) noexcept
    {
        YGNodeStyleSetMinWidth(m_node, minWidth);
        checkIsDirty();
    }

    /** @brief Sets the minimum width as a percentage of the parent's width. */
    void setMinWidthPercent(float minWidth) noexcept
    {
        YGNodeStyleSetMinWidthPercent(m_node, minWidth);
        checkIsDirty();
    }

    /**
     * @brief Sets the minimum width from a YGValue, dispatching to the point or percent setter.
     *
     * A YGUnitUndefined value applies it through both setters, effectively unsetting the constraint.
     */
    void setMinWidthYGValue(YGValue value) noexcept
    {
        switch (value.unit)
        {
        case YGUnitUndefined:
            setMinWidth(value.value);
            setMinWidthPercent(value.value);
            break;
        case YGUnitPoint:
            setMinWidth(value.value);
            break;
        case YGUnitPercent:
            setMinWidthPercent(value.value);
            break;
        default:
            break;
        }
    }

    /** @brief The minimum width style. */
    YGValue minWidth() const noexcept
    {
        return YGNodeStyleGetMinWidth(m_node);
    }

    /** @brief Sets the minimum height in logical points. */
    void setMinHeight(float minHeight) noexcept
    {
        YGNodeStyleSetMinHeight(m_node, minHeight);
        checkIsDirty();
    }

    /** @brief Sets the minimum height as a percentage of the parent's height. */
    void setMinHeightPercent(float minHeight) noexcept
    {
        YGNodeStyleSetMinHeightPercent(m_node, minHeight);
        checkIsDirty();
    }

    /** @brief The minimum height style. */
    YGValue minHeight() const noexcept
    {
        return YGNodeStyleGetMinHeight(m_node);
    }

    /** @brief Sets the maximum width in logical points. */
    void setMaxWidth(float maxWidth) noexcept
    {
        YGNodeStyleSetMaxWidth(m_node, maxWidth);
        checkIsDirty();
    }

    /** @brief Sets the maximum width as a percentage of the parent's width. */
    void setMaxWidthPercent(float maxWidth) noexcept
    {
        YGNodeStyleSetMaxWidthPercent(m_node, maxWidth);
        checkIsDirty();
    }

    /**
     * @brief Sets the maximum width from a YGValue, dispatching to the point or percent setter.
     *
     * A YGUnitUndefined value applies it through both setters, effectively unsetting the constraint.
     */
    void setMaxWidthYGValue(YGValue value) noexcept
    {
        switch (value.unit)
        {
        case YGUnitUndefined:
            setMaxWidth(value.value);
            setMaxWidthPercent(value.value);
            break;
        case YGUnitPoint:
            setMaxWidth(value.value);
            break;
        case YGUnitPercent:
            setMaxWidthPercent(value.value);
            break;
        default:
            break;
        }
    }

    /** @brief The maximum width style. */
    YGValue maxWidth() const noexcept
    {
        return YGNodeStyleGetMaxWidth(m_node);
    }

    /** @brief Sets the maximum height in logical points. */
    void setMaxHeight(float maxHeight) noexcept
    {
        YGNodeStyleSetMaxHeight(m_node, maxHeight);
        checkIsDirty();
    }

    /** @brief Sets the maximum height as a percentage of the parent's height. */
    void setMaxHeightPercent(float maxHeight) noexcept
    {
        YGNodeStyleSetMaxHeightPercent(m_node, maxHeight);
        checkIsDirty();
    }

    /** @brief The maximum height style. */
    YGValue maxHeight() const noexcept
    {
        return YGNodeStyleGetMaxHeight(m_node);
    }

    /** @brief Sets whether children wrap into multiple lines. */
    void setFlexWrap(YGWrap flexWrap) noexcept
    {
        YGNodeStyleSetFlexWrap(m_node, flexWrap);
        checkIsDirty();
    }

    /** @brief The flex-wrap value. */
    YGWrap flexWrap() const noexcept
    {
        return YGNodeStyleGetFlexWrap(m_node);
    }

    /** @brief Sets how content exceeding the node's bounds is treated (visible, hidden or scroll). */
    void setOverflow(YGOverflow overflow) noexcept
    {
        YGNodeStyleSetOverflow(m_node, overflow);
        checkIsDirty();
    }

    /** @brief The overflow value. */
    YGOverflow overflow() const noexcept
    {
        return YGNodeStyleGetOverflow(m_node);
    }

    /**
     * @brief Sets the display mode (flex, none or contents).
     *
     * YGDisplayNone hides the node and excludes it from the layout
     * (see AKNode::visible()). When switching from YGDisplayNone back to a
     * visible mode, the node is flagged with AKNode::CHLayout on every target
     * it is currently assigned to, so each one gets repainted.
     */
    void setDisplay(YGDisplay display) noexcept;

    /** @brief The display mode. */
    YGDisplay display() const noexcept
    {
        return YGNodeStyleGetDisplay(m_node);
    }

    /** @brief Sets the flex shorthand value. */
    void setFlex(float flex) noexcept
    {
        YGNodeStyleSetFlex(m_node, flex);
        checkIsDirty();
    }

    /** @brief The flex shorthand value. */
    float flex() const noexcept
    {
        return YGNodeStyleGetFlex(m_node);
    }

    /** @brief Sets the flex-grow factor. */
    void setFlexGrow(float flexGrow) noexcept
    {
        YGNodeStyleSetFlexGrow(m_node, flexGrow);
        checkIsDirty();
    }

    /** @brief The flex-grow factor. */
    float flexGrow() const noexcept
    {
        return YGNodeStyleGetFlexGrow(m_node);
    }

    /** @brief Sets the flex-shrink factor. */
    void setFlexShrink(float flexShrink) noexcept
    {
        YGNodeStyleSetFlexShrink(m_node, flexShrink);
        checkIsDirty();
    }

    /** @brief The flex-shrink factor. */
    float flexShrink() const noexcept
    {
        return YGNodeStyleGetFlexShrink(m_node);
    }

    /** @brief Sets the flex-basis in logical points. */
    void setFlexBasis(float flexBasis) noexcept
    {
        YGNodeStyleSetFlexBasis(m_node, flexBasis);
        checkIsDirty();
    }

    /** @brief Sets the flex-basis as a percentage of the parent's main axis size. */
    void setFlexBasisPercent(float flexBasis) noexcept
    {
        YGNodeStyleSetFlexBasisPercent(m_node, flexBasis);
        checkIsDirty();
    }

    /** @brief Sets the flex-basis to auto. */
    void setFlexBasisAuto() noexcept
    {
        YGNodeStyleSetFlexBasisAuto(m_node);
        checkIsDirty();
    }

    /** @brief The flex-basis value. */
    YGValue flexBasis() const noexcept
    {
        return YGNodeStyleGetFlexBasis(m_node);
    }

    /** @brief Sets the margin for the given edge in logical points. */
    void setMargin(YGEdge edge, float margin) noexcept
    {
        YGNodeStyleSetMargin(m_node, edge, margin);
        checkIsDirty();
    }

    /** @brief Sets the margin for the given edge as a percentage. */
    void setMarginPercent(YGEdge edge, float margin) noexcept
    {
        YGNodeStyleSetMarginPercent(m_node, edge, margin);
        checkIsDirty();
    }

    /** @brief Sets the margin for the given edge to auto. */
    void setMarginAuto(YGEdge edge) noexcept
    {
        YGNodeStyleSetMarginAuto(m_node, edge);
        checkIsDirty();
    }

    /** @brief The margin style for the given edge. */
    YGValue margin(YGEdge edge) const noexcept
    {
        return YGNodeStyleGetMargin(m_node, edge);
    }

    /** @brief Sets the padding for the given edge in logical points. */
    void setPadding(YGEdge edge, float padding) noexcept
    {
        YGNodeStyleSetPadding(m_node, edge, padding);
        checkIsDirty();
    }

    /** @brief Sets the padding for the given edge as a percentage. */
    void setPaddingPercent(YGEdge edge, float padding) noexcept
    {
        YGNodeStyleSetPaddingPercent(m_node, edge, padding);
        checkIsDirty();
    }

    /** @brief The padding style for the given edge. */
    YGValue padding(YGEdge edge) const noexcept
    {
        return YGNodeStyleGetPadding(m_node, edge);
    }

    /** @brief Sets the border width for the given edge in logical points. */
    void setBorder(YGEdge edge, float border) noexcept
    {
        YGNodeStyleSetBorder(m_node, edge, border);
        checkIsDirty();
    }

    /** @brief The border width for the given edge. */
    float border(YGEdge edge) const noexcept
    {
        return YGNodeStyleGetBorder(m_node, edge);
    }

    /** @brief Sets the gap between children for the given gutter (column, row or all) in logical points. */
    void setGap(YGGutter gutter, float gapLength) noexcept
    {
        YGNodeStyleSetGap(m_node, gutter, gapLength);
        checkIsDirty();
    }

    /** @brief Sets the gap between children for the given gutter as a percentage. */
    void setGapPercent(YGGutter gutter, float gapLength) noexcept
    {
        YGNodeStyleSetGapPercent(m_node, gutter, gapLength);
        checkIsDirty();
    }

    /** @brief The gap for the given gutter. */
    float gap(YGGutter gutter) const noexcept
    {
        return YGNodeStyleGetGap(m_node, gutter);
    }

    /** @brief Sets the aspect ratio (width / height). */
    void setAspectRatio(float aspectRatio) noexcept
    {
        YGNodeStyleSetAspectRatio(m_node, aspectRatio);
        checkIsDirty();
    }

    /** @brief The aspect ratio. */
    float aspectRatio() const noexcept
    {
        return YGNodeStyleGetAspectRatio(m_node);
    }

    /**
     * @brief Sets an anchor node for this node.
     *
     * Makes this node’s position relative to the specified anchor node
     * instead of its parent, while preserving the current hierarchy.
     *
     * While an anchor is set, the position type is forced to
     * YGPositionTypeAbsolute. The previous position type is restored
     * when the anchor is unset.
     *
     * If the anchor node is destroyed, it is automatically unset.
     *
     * @param anchor The node to use as anchor. Pass nullptr to unset.
     *               Passing this node itself is ignored.
     */
    void setAnchorNode(AKNode *anchor) noexcept;

    /**
     * @brief Returns the current anchor node.
     *
     * @return The current anchor node, or nullptr if none is set.
     */
    AKNode *anchorNode() const noexcept { return m_anchorNode; }

    /**
     * @brief Calculates the node layout.
     *
     * If the node has a parent, YGNodeCalculateLayout is called
     * using the parent node and the already calculated width and height.
     *
     * If the node does not have a parent, YGNodeCalculateLayout is called
     * from the node itself with YGUndefined for both width and height.
     *
     * @note This only updates the `calculated...()` values of the layout, which are
     *       always relative to the parent node. Properties such as AKNode::worldRect() or
     *       AKNode::sceneRect() are only updated from AKScene::render().
     */
    void calculate() noexcept;

    /**
     * @brief Calculates the node layout.
     *
     * @warning Use the alternative variant if you need to update the layout during
     *          an AKNode::layoutEvent(). If the parent dimensions depend on its
     *          children and the provided available width and height do not match
     *          those of the parent, the scene may calculate incorrect values for
     *          AKNode::rect() and AKNode::worldRect().
     *
     * @note This function only updates the `calculated...()` layout values, which are
     *       always relative to the parent node. Properties such as AKNode::rect()
     *       or AKNode::globalRect() are only updated via AKScene::render().
     */
    void calculate(float availableWidth, float availableHeight, YGDirection ownerDirection) noexcept
    {
        YGNodeCalculateLayout(m_node, availableWidth, availableHeight, ownerDirection);
    }

    /** @brief The underlying Yoga node handle. */
    YGNodeRef ygNode() const noexcept { return m_node; };

private:
    friend class AKNode;
    friend class AKScene;
    void apply(bool calculate, bool updateRoot) noexcept;
    AKLayout(AKNode &akNode) noexcept;
    CZ_DISABLE_COPY(AKLayout)
    ~AKLayout() { YGNodeFree(m_node); YGConfigFree(m_config); }
    void checkIsDirty() noexcept;

    // Only called by AKScene, updates worldRect, sceneRect, etc
    static void applyTree(AKNode *node);
    YGNodeRef m_node { nullptr };
    AKNode &m_akNode;
    YGConfigRef m_config;

    CZWeak<AKNode> m_anchorNode;
    YGPositionType m_posTypeBeforeAnchorNode;
};

#endif // CZ_AKLAYOUT_H
