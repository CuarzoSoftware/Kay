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

    /* Style */

    void copyStyle(const AKLayout &from) noexcept
    {
        YGNodeCopyStyle(m_node, from.m_node);
    }

    void setDirection(YGDirection direction) noexcept
    {
        YGNodeStyleSetDirection(m_node, direction);
    }

    YGDirection direction() const noexcept
    {
        return YGNodeStyleGetDirection(m_node);
    }

    void setFlexDirection(YGFlexDirection flexDirection) noexcept
    {
        YGNodeStyleSetFlexDirection(m_node, flexDirection);
    }

    YGFlexDirection flexDirection() const noexcept
    {
        return YGNodeStyleGetFlexDirection(m_node);
    }

    void setJustifyContent(YGJustify justifyContent) noexcept
    {
        YGNodeStyleSetJustifyContent(m_node, justifyContent);
    }

    YGJustify justifyContent() const noexcept
    {
        return YGNodeStyleGetJustifyContent(m_node);
    }

    void setAlignContent(YGAlign alignContent) noexcept
    {
        YGNodeStyleSetAlignContent(m_node, alignContent);
    }

    YGAlign alignContent() const noexcept
    {
        return YGNodeStyleGetAlignContent(m_node);
    }

    void setAlignItems(YGAlign alignItems) noexcept
    {
        YGNodeStyleSetAlignItems(m_node, alignItems);
    }

    YGAlign alignItems() const noexcept
    {
        return YGNodeStyleGetAlignItems(m_node);
    }

    void setAlignSelf(YGAlign alignSelf) noexcept
    {
        YGNodeStyleSetAlignSelf(m_node, alignSelf);
    }

    YGAlign alignSelf() const noexcept
    {
        return YGNodeStyleGetAlignSelf(m_node);
    }

    void setPositionType(YGPositionType positionType) noexcept
    {
        YGNodeStyleSetPositionType(m_node, positionType);
    }

    YGPositionType positionType() const noexcept
    {
        return YGNodeStyleGetPositionType(m_node);
    }

    void setPosition(YGEdge edge, float position) noexcept
    {
        YGNodeStyleSetPosition(m_node, edge, position);
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
    }

    void setWidthPercent(float width) noexcept
    {
        YGNodeStyleSetWidthPercent(m_node, width);
    }

    void setWidthAuto() noexcept
    {
        YGNodeStyleSetWidthAuto(m_node);
    }

    YGValue height() const noexcept
    {
        return YGNodeStyleGetHeight(m_node);
    }

    void setHeight(float height) noexcept
    {
        YGNodeStyleSetHeight(m_node, height);
    }

    void setHeightPercent(float height) noexcept
    {
        YGNodeStyleSetHeightPercent(m_node, height);
    }

    void setHeightAuto() noexcept
    {
        YGNodeStyleSetHeightAuto(m_node);
    }

    void setMinWidth(float minWidth) noexcept
    {
        YGNodeStyleSetMinWidth(m_node, minWidth);
    }

    void setMinWidthPercent(float minWidth) noexcept
    {
        YGNodeStyleSetMinWidthPercent(m_node, minWidth);
    }

    YGValue minWidth() const noexcept
    {
        return YGNodeStyleGetMinWidth(m_node);
    }

    void setMinHeight(float minHeight) noexcept
    {
        YGNodeStyleSetMinHeight(m_node, minHeight);
    }

    void setMinHeightPercent(float minHeight) noexcept
    {
        YGNodeStyleSetMinHeightPercent(m_node, minHeight);
    }

    YGValue minHeight() const noexcept
    {
        return YGNodeStyleGetMinHeight(m_node);
    }

    void setMaxWidth(float maxWidth) noexcept
    {
        YGNodeStyleSetMaxWidth(m_node, maxWidth);
    }

    void setMaxWidthPercent(float maxWidth) noexcept
    {
        YGNodeStyleSetMaxWidthPercent(m_node, maxWidth);
    }

    YGValue maxWidth() const noexcept
    {
        return YGNodeStyleGetMaxWidth(m_node);
    }

    void setMaxHeight(float maxHeight) noexcept
    {
        YGNodeStyleSetMaxHeight(m_node, maxHeight);
    }

    void setMaxHeightPercent(float maxHeight) noexcept
    {
        YGNodeStyleSetMaxHeightPercent(m_node, maxHeight);
    }

    YGValue maxHeight() const noexcept
    {
        return YGNodeStyleGetMaxHeight(m_node);
    }

    void setFlexWrap(YGWrap flexWrap) noexcept
    {
        YGNodeStyleSetFlexWrap(m_node, flexWrap);
    }

    YGWrap flexWrap() const noexcept
    {
        return YGNodeStyleGetFlexWrap(m_node);
    }

    void setOverflow(YGOverflow overflow) noexcept
    {
        YGNodeStyleSetOverflow(m_node, overflow);
    }

    YGOverflow overflow() const noexcept
    {
        return YGNodeStyleGetOverflow(m_node);
    }

    void setDisplay(YGDisplay display) noexcept
    {
        YGNodeStyleSetDisplay(m_node, display);
    }

    YGDisplay display() const noexcept
    {
        return YGNodeStyleGetDisplay(m_node);
    }

    void setFlex(float flex) noexcept
    {
        YGNodeStyleSetFlex(m_node, flex);
    }

    float flex() const noexcept
    {
        return YGNodeStyleGetFlex(m_node);
    }

    void setFlexGrow(float flexGrow) noexcept
    {
        YGNodeStyleSetFlexGrow(m_node, flexGrow);
    }

    float flexGrow() const noexcept
    {
        return YGNodeStyleGetFlexGrow(m_node);
    }

    void setFlexShrink(float flexShrink) noexcept
    {
        YGNodeStyleSetFlexShrink(m_node, flexShrink);
    }

    float flexShrink() const noexcept
    {
        return YGNodeStyleGetFlexShrink(m_node);
    }

    void setFlexBasis(float flexBasis) noexcept
    {
        YGNodeStyleSetFlexBasis(m_node, flexBasis);
    }

    void setFlexBasisPercent(float flexBasis) noexcept
    {
        YGNodeStyleSetFlexBasisPercent(m_node, flexBasis);
    }

    void setFlexBasisAuto() noexcept
    {
        YGNodeStyleSetFlexBasisAuto(m_node);
    }

    YGValue flexBasis() const noexcept
    {
        return YGNodeStyleGetFlexBasis(m_node);
    }

    void setMargin(YGEdge edge, float margin) noexcept
    {
        YGNodeStyleSetMargin(m_node, edge, margin);
    }

    void setMarginPercent(YGEdge edge, float margin) noexcept
    {
        YGNodeStyleSetMarginPercent(m_node, edge, margin);
    }

    void setMarginAuto(YGEdge edge) noexcept
    {
        YGNodeStyleSetMarginAuto(m_node, edge);
    }

    YGValue margin(YGEdge edge) const noexcept
    {
        return YGNodeStyleGetMargin(m_node, edge);
    }

    void setPadding(YGEdge edge, float padding) noexcept
    {
        YGNodeStyleSetPadding(m_node, edge, padding);
    }

    void setPaddingPercent(YGEdge edge, float padding) noexcept
    {
        YGNodeStyleSetPaddingPercent(m_node, edge, padding);
    }

    YGValue padding(YGEdge edge) const noexcept
    {
        return YGNodeStyleGetPadding(m_node, edge);
    }

    void setBorder(YGEdge edge, float border) noexcept
    {
        YGNodeStyleSetBorder(m_node, edge, border);
    }

    float border(YGEdge edge) const noexcept
    {
        return YGNodeStyleGetBorder(m_node, edge);
    }

    void setGap(YGGutter gutter, float gapLength) noexcept
    {
        YGNodeStyleSetGap(m_node, gutter, gapLength);
    }

    void setGapPercent(YGGutter gutter, float gapLength) noexcept
    {
        YGNodeStyleSetGapPercent(m_node, gutter, gapLength);
    }

    float gap(YGGutter gutter) const noexcept
    {
        return YGNodeStyleGetGap(m_node, gutter);
    }

    void setAspectRatio(float aspectRatio) noexcept
    {
        YGNodeStyleSetAspectRatio(m_node, aspectRatio);
    }

    float aspectRatio() const noexcept
    {
        return YGNodeStyleGetAspectRatio(m_node);
    }
private:
    friend class AKNode;
    friend class AKScene;
    AKLayout() noexcept : m_node(YGNodeNew()) {}
    ~AKLayout() { YGNodeFree(m_node); }
    YGNodeRef m_node { nullptr };
};

#endif // AKLAYOUT_H
