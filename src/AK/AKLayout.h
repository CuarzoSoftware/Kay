#ifndef AKLAYOUT_H
#define AKLAYOUT_H

#include <AK/AK.h>
#include <yoga/Yoga.h>

class AK::AKLayout
{
public:
    float calculatedWidth() const noexcept
    {
        return YGNodeLayoutGetWidth(m_node);
    }

    float calculatedHeight() const noexcept
    {
        return YGNodeLayoutGetHeight(m_node);
    }

    float calculatedLeft() const noexcept
    {
        return YGNodeLayoutGetLeft(m_node);
    }

    float calculatedTop() const noexcept
    {
        return YGNodeLayoutGetTop(m_node);
    }

    float calculatedRight() const noexcept
    {
        return YGNodeLayoutGetRight(m_node);
    }

    float calculatedBottom() const noexcept
    {
        return YGNodeLayoutGetBottom(m_node);
    }

    YGDirection calculatedDirection() const noexcept
    {
        return YGNodeLayoutGetDirection(m_node);
    }

    bool calculatedHadOverflow() const noexcept
    {
        return YGNodeLayoutGetHadOverflow(m_node);
    }

    float calculatedMargin(YGEdge edge) const noexcept
    {
        return YGNodeLayoutGetMargin(m_node, edge);
    }

    float calculatedBorder(YGEdge edge) const noexcept
    {
        return YGNodeLayoutGetBorder(m_node, edge);
    }

    float calculatedPadding(YGEdge edge) const noexcept
    {
        return YGNodeLayoutGetPadding(m_node, edge);
    }

    /* Conf */

    void setPointScaleFactor(float pixelsInPoint) noexcept
    {
        YGConfigSetPointScaleFactor(m_config, pixelsInPoint);
        YGNodeSetConfig(m_node, m_config);
        checkIsDirty();
    }

    float pointScaleFactor() const noexcept
    {
        return YGConfigGetPointScaleFactor(m_config);
    }

    /* Style */

    void copyStyle(const AKLayout &from) noexcept
    {
        YGNodeCopyStyle(m_node, from.m_node);
        checkIsDirty();
    }

    void setDirection(YGDirection direction) noexcept
    {
        YGNodeStyleSetDirection(m_node, direction);
        checkIsDirty();
    }

    YGDirection direction() const noexcept
    {
        return YGNodeStyleGetDirection(m_node);
    }

    void setFlexDirection(YGFlexDirection flexDirection) noexcept
    {
        YGNodeStyleSetFlexDirection(m_node, flexDirection);
        checkIsDirty();
    }

    YGFlexDirection flexDirection() const noexcept
    {
        return YGNodeStyleGetFlexDirection(m_node);
    }

    void setJustifyContent(YGJustify justifyContent) noexcept
    {
        YGNodeStyleSetJustifyContent(m_node, justifyContent);
        checkIsDirty();
    }

    YGJustify justifyContent() const noexcept
    {
        return YGNodeStyleGetJustifyContent(m_node);
    }

    void setAlignContent(YGAlign alignContent) noexcept
    {
        YGNodeStyleSetAlignContent(m_node, alignContent);
        checkIsDirty();
    }

    YGAlign alignContent() const noexcept
    {
        return YGNodeStyleGetAlignContent(m_node);
    }

    void setAlignItems(YGAlign alignItems) noexcept
    {
        YGNodeStyleSetAlignItems(m_node, alignItems);
        checkIsDirty();
    }

    YGAlign alignItems() const noexcept
    {
        return YGNodeStyleGetAlignItems(m_node);
    }

    void setAlignSelf(YGAlign alignSelf) noexcept
    {
        YGNodeStyleSetAlignSelf(m_node, alignSelf);
        checkIsDirty();
    }

    YGAlign alignSelf() const noexcept
    {
        return YGNodeStyleGetAlignSelf(m_node);
    }

    void setPositionType(YGPositionType positionType) noexcept
    {
        YGNodeStyleSetPositionType(m_node, positionType);
        checkIsDirty();
    }

    YGPositionType positionType() const noexcept
    {
        return YGNodeStyleGetPositionType(m_node);
    }

    void setPosition(YGEdge edge, float position) noexcept
    {
        YGNodeStyleSetPosition(m_node, edge, position);
        checkIsDirty();
    }

    void positionPercent(YGEdge edge, float position) noexcept
    {
        YGNodeStyleSetPositionPercent(m_node, edge, position);
    }

    YGValue position(YGEdge edge) const noexcept
    {
        return YGNodeStyleGetPosition(m_node, edge);
    }

    YGValue width() const noexcept
    {
        return YGNodeStyleGetWidth(m_node);
    }

    void setWidth(float width) noexcept
    {
        YGNodeStyleSetWidth(m_node, width);
        checkIsDirty();
    }

    void setWidthPercent(float width) noexcept
    {
        YGNodeStyleSetWidthPercent(m_node, width);
        checkIsDirty();
    }

    void setWidthAuto() noexcept
    {
        YGNodeStyleSetWidthAuto(m_node);
        checkIsDirty();
    }

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

    YGValue height() const noexcept
    {
        return YGNodeStyleGetHeight(m_node);
    }

    void setHeight(float height) noexcept
    {
        YGNodeStyleSetHeight(m_node, height);
        checkIsDirty();
    }

    void setHeightPercent(float height) noexcept
    {
        YGNodeStyleSetHeightPercent(m_node, height);
        checkIsDirty();
    }

    void setHeightAuto() noexcept
    {
        YGNodeStyleSetHeightAuto(m_node);
        checkIsDirty();
    }

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

    void setMinWidth(float minWidth) noexcept
    {
        YGNodeStyleSetMinWidth(m_node, minWidth);
        checkIsDirty();
    }

    void setMinWidthPercent(float minWidth) noexcept
    {
        YGNodeStyleSetMinWidthPercent(m_node, minWidth);
        checkIsDirty();
    }

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

    YGValue minWidth() const noexcept
    {
        return YGNodeStyleGetMinWidth(m_node);
    }

    void setMinHeight(float minHeight) noexcept
    {
        YGNodeStyleSetMinHeight(m_node, minHeight);
        checkIsDirty();
    }

    void setMinHeightPercent(float minHeight) noexcept
    {
        YGNodeStyleSetMinHeightPercent(m_node, minHeight);
        checkIsDirty();
    }

    YGValue minHeight() const noexcept
    {
        return YGNodeStyleGetMinHeight(m_node);
    }

    void setMaxWidth(float maxWidth) noexcept
    {
        YGNodeStyleSetMaxWidth(m_node, maxWidth);
        checkIsDirty();
    }

    void setMaxWidthPercent(float maxWidth) noexcept
    {
        YGNodeStyleSetMaxWidthPercent(m_node, maxWidth);
        checkIsDirty();
    }

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

    YGValue maxWidth() const noexcept
    {
        return YGNodeStyleGetMaxWidth(m_node);
    }

    void setMaxHeight(float maxHeight) noexcept
    {
        YGNodeStyleSetMaxHeight(m_node, maxHeight);
        checkIsDirty();
    }

    void setMaxHeightPercent(float maxHeight) noexcept
    {
        YGNodeStyleSetMaxHeightPercent(m_node, maxHeight);
        checkIsDirty();
    }

    YGValue maxHeight() const noexcept
    {
        return YGNodeStyleGetMaxHeight(m_node);
    }

    void setFlexWrap(YGWrap flexWrap) noexcept
    {
        YGNodeStyleSetFlexWrap(m_node, flexWrap);
        checkIsDirty();
    }

    YGWrap flexWrap() const noexcept
    {
        return YGNodeStyleGetFlexWrap(m_node);
    }

    void setOverflow(YGOverflow overflow) noexcept
    {
        YGNodeStyleSetOverflow(m_node, overflow);
        checkIsDirty();
    }

    YGOverflow overflow() const noexcept
    {
        return YGNodeStyleGetOverflow(m_node);
    }

    void setDisplay(YGDisplay display) noexcept;

    YGDisplay display() const noexcept
    {
        return YGNodeStyleGetDisplay(m_node);
    }

    void setFlex(float flex) noexcept
    {
        YGNodeStyleSetFlex(m_node, flex);
        checkIsDirty();
    }

    float flex() const noexcept
    {
        return YGNodeStyleGetFlex(m_node);
    }

    void setFlexGrow(float flexGrow) noexcept
    {
        YGNodeStyleSetFlexGrow(m_node, flexGrow);
        checkIsDirty();
    }

    float flexGrow() const noexcept
    {
        return YGNodeStyleGetFlexGrow(m_node);
    }

    void setFlexShrink(float flexShrink) noexcept
    {
        YGNodeStyleSetFlexShrink(m_node, flexShrink);
        checkIsDirty();
    }

    float flexShrink() const noexcept
    {
        return YGNodeStyleGetFlexShrink(m_node);
    }

    void setFlexBasis(float flexBasis) noexcept
    {
        YGNodeStyleSetFlexBasis(m_node, flexBasis);
        checkIsDirty();
    }

    void setFlexBasisPercent(float flexBasis) noexcept
    {
        YGNodeStyleSetFlexBasisPercent(m_node, flexBasis);
        checkIsDirty();
    }

    void setFlexBasisAuto() noexcept
    {
        YGNodeStyleSetFlexBasisAuto(m_node);
        checkIsDirty();
    }

    YGValue flexBasis() const noexcept
    {
        return YGNodeStyleGetFlexBasis(m_node);
    }

    void setMargin(YGEdge edge, float margin) noexcept
    {
        YGNodeStyleSetMargin(m_node, edge, margin);
        checkIsDirty();
    }

    void setMarginPercent(YGEdge edge, float margin) noexcept
    {
        YGNodeStyleSetMarginPercent(m_node, edge, margin);
        checkIsDirty();
    }

    void setMarginAuto(YGEdge edge) noexcept
    {
        YGNodeStyleSetMarginAuto(m_node, edge);
        checkIsDirty();
    }

    YGValue margin(YGEdge edge) const noexcept
    {
        return YGNodeStyleGetMargin(m_node, edge);
    }

    void setPadding(YGEdge edge, float padding) noexcept
    {
        YGNodeStyleSetPadding(m_node, edge, padding);
        checkIsDirty();
    }

    void setPaddingPercent(YGEdge edge, float padding) noexcept
    {
        YGNodeStyleSetPaddingPercent(m_node, edge, padding);
        checkIsDirty();
    }

    YGValue padding(YGEdge edge) const noexcept
    {
        return YGNodeStyleGetPadding(m_node, edge);
    }

    void setBorder(YGEdge edge, float border) noexcept
    {
        YGNodeStyleSetBorder(m_node, edge, border);
        checkIsDirty();
    }

    float border(YGEdge edge) const noexcept
    {
        return YGNodeStyleGetBorder(m_node, edge);
    }

    void setGap(YGGutter gutter, float gapLength) noexcept
    {
        YGNodeStyleSetGap(m_node, gutter, gapLength);
        checkIsDirty();
    }

    void setGapPercent(YGGutter gutter, float gapLength) noexcept
    {
        YGNodeStyleSetGapPercent(m_node, gutter, gapLength);
        checkIsDirty();
    }

    float gap(YGGutter gutter) const noexcept
    {
        return YGNodeStyleGetGap(m_node, gutter);
    }

    void setAspectRatio(float aspectRatio) noexcept
    {
        YGNodeStyleSetAspectRatio(m_node, aspectRatio);
        checkIsDirty();
    }

    float aspectRatio() const noexcept
    {
        return YGNodeStyleGetAspectRatio(m_node);
    }

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
     *       always relative to the parent node. Properties such as AKNode::rect() or
     *       AKNode::globalRect() are only updated from AKScene::render().
     */
    void calculate() noexcept;


    /**
     * @brief Calculates the node layout.
     *
     * @warning Use the alternative variant if you need to update the layout during
     *          an AKNode::layoutEvent(). If the parent dimensions depend on its
     *          children and the provided available width and height do not match
     *          those of the parent, the scene may calculate incorrect values for
     *          AKNode::rect() and AKNode::globalRect().
     *
     * @note This function only updates the `calculated...()` layout values, which are
     *       always relative to the parent node. Properties such as AKNode::rect()
     *       or AKNode::globalRect() are only updated via AKScene::render().
     */
    void calculate(float availableWidth, float availableHeight, YGDirection ownerDirection) noexcept
    {
        YGNodeCalculateLayout(m_node, availableWidth, availableHeight, ownerDirection);
    }

private:
    friend class AKNode;
    friend class AKScene;
    void apply(bool calculate) noexcept;
    AKLayout(AKNode &akNode) noexcept;
    AKCLASS_NO_COPY(AKLayout)
    ~AKLayout() { YGNodeFree(m_node); YGConfigFree(m_config); }
    void checkIsDirty() noexcept;
    static void applyTree(AKNode *node);
    YGNodeRef m_node { nullptr };
    AKNode &m_akNode;
    YGConfigRef m_config;
};

#endif // AKLAYOUT_H
