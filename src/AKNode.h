#ifndef AKNODE_H
#define AKNODE_H

#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/gl/GrGLTypes.h"
#include <AKObject.h>
#include <AKBitset.h>

#include <include/core/SkRegion.h>
#include <unordered_map>
#include <yoga/Yoga.h>
#include <memory>
#include <list>
#include <vector>

class AK::AKNode : public AKObject
{
public:

    enum Caps : UInt32
    {
        Render  = 1 << 0,
        Bake    = 1 << 1,
        Scene   = 1 << 2
    };

    virtual ~AKNode();

    /* Insert at the end, nullptr unsets */
    void setParent(AKNode *parent) noexcept;

    AKNode *parent() const noexcept
    {
        return m_parent;
    }

    void insertBefore(AKNode *other) noexcept;
    void insertAfter(AKNode *other) noexcept;

    bool isSubchildOf(AKNode *node) const noexcept
    {
        if (!node || !parent()) return false;

        return parent() == node || parent()->isSubchildOf(node);
    }

    const std::vector<AKNode*> &children() const noexcept
    {
        return m_children;
    }

    UInt32 caps() const noexcept
    {
        return m_caps;
    }

    AKTarget *currentTarget() const noexcept
    {
        return t->target;
    }

    bool isVisible() const noexcept
    {
        return m_flags.check(Visible);
    }

    void setVisible(bool visible) noexcept
    {
        m_flags.setFlag(Visible, visible);
    }

    void setInputRegion(SkRegion *region) noexcept
    {
        m_inputRegion = region ? std::make_unique<SkRegion>(*region) : nullptr;
    }

    SkRegion *inputRegion() const noexcept
    {
        return m_inputRegion.get();
    }

    bool insideLastTarget() const noexcept
    {
        return m_insideLastTarget;
    }

    bool renderedOnLastTarget() const noexcept
    {
        return m_renderedOnLastTarget;
    }

    /* Layout */

    const SkIRect globalRect() const noexcept
    {
        return m_globalRect;
    }

    float layoutGetWidth() const noexcept
    {
        return YGNodeLayoutGetWidth(m_node);
    }

    float layoutGetHeight() const noexcept
    {
        return YGNodeLayoutGetHeight(m_node);
    }

    float layoutGetLeft() const noexcept
    {
        return YGNodeLayoutGetLeft(m_node);
    }

    float layoutGetTop() const noexcept
    {
        return YGNodeLayoutGetTop(m_node);
    }

    void styleCopyFrom(const AKNode &other) noexcept
    {
        YGNodeCopyStyle(m_node, other.m_node);
    }

    void styleSetDirection(YGDirection direction) noexcept
    {
        YGNodeStyleSetDirection(m_node, direction);
    }

    YGDirection styleGetDirection() const noexcept
    {
        return YGNodeStyleGetDirection(m_node);
    }

    void styleSetFlexDirection(YGFlexDirection flexDirection) noexcept
    {
        YGNodeStyleSetFlexDirection(m_node, flexDirection);
    }

    YGFlexDirection styleGetFlexDirection() const noexcept
    {
        return YGNodeStyleGetFlexDirection(m_node);
    }

    void styleSetJustifyContent(YGJustify justifyContent) noexcept
    {
        YGNodeStyleSetJustifyContent(m_node, justifyContent);
    }

    YGJustify styleGetJustifyContent() const noexcept
    {
        return YGNodeStyleGetJustifyContent(m_node);
    }

    void styleSetAlignContent(YGAlign alignContent) noexcept
    {
        YGNodeStyleSetAlignContent(m_node, alignContent);
    }

    YGAlign styleGetAlignContent() const noexcept
    {
        return YGNodeStyleGetAlignContent(m_node);
    }

    void styleSetAlignItems(YGAlign alignItems) noexcept
    {
        YGNodeStyleSetAlignItems(m_node, alignItems);
    }

    YGAlign styleGetAlignItems() const noexcept
    {
        return YGNodeStyleGetAlignItems(m_node);
    }

    void styleSetAlignSelf(YGAlign alignSelf) noexcept
    {
        YGNodeStyleSetAlignSelf(m_node, alignSelf);
    }

    YGAlign styleGetAlignSelf() const noexcept
    {
        return YGNodeStyleGetAlignSelf(m_node);
    }

    void styleSetPositionType(YGPositionType positionType) noexcept
    {
        YGNodeStyleSetPositionType(m_node, positionType);
    }

    YGPositionType styleGetPositionType() const noexcept
    {
        return YGNodeStyleGetPositionType(m_node);
    }

    void styleSetPosition(YGEdge edge, float position) noexcept
    {
        YGNodeStyleSetPosition(m_node, edge, position);
    }

    void styleSetPositionPercent(YGEdge edge, float position) noexcept
    {
        YGNodeStyleSetPositionPercent(m_node, edge, position);
    }

    YGValue styleGetPosition(YGEdge edge) const noexcept
    {
        return YGNodeStyleGetPosition(m_node, edge);
    }

    YGValue styleGetWidth() const noexcept
    {
        return YGNodeStyleGetWidth(m_node);
    }

    void styleSetWidth(float width) noexcept
    {
        YGNodeStyleSetWidth(m_node, width);
    }

    void styleSetWidthPercent(float width) noexcept
    {
        YGNodeStyleSetWidthPercent(m_node, width);
    }

    void styleSetWidthAuto() noexcept
    {
        YGNodeStyleSetWidthAuto(m_node);
    }

    YGValue styleGetHeight() const noexcept
    {
        return YGNodeStyleGetHeight(m_node);
    }

    void styleSetHeight(float height) noexcept
    {
        YGNodeStyleSetHeight(m_node, height);
    }

    void styleSetHeightPercent(float height) noexcept
    {
        YGNodeStyleSetHeightPercent(m_node, height);
    }

    void styleSetHeightAuto() noexcept
    {
        YGNodeStyleSetHeightAuto(m_node);
    }

    void styleSetMinWidth(float minWidth) noexcept
    {
        YGNodeStyleSetMinWidth(m_node, minWidth);
    }

    void styleSetMinWidthPercent(float minWidth) noexcept
    {
        YGNodeStyleSetMinWidthPercent(m_node, minWidth);
    }

    YGValue styleGetMinWidth() const noexcept
    {
        return YGNodeStyleGetMinWidth(m_node);
    }

    void styleSetMinHeight(float minHeight) noexcept
    {
        YGNodeStyleSetMinHeight(m_node, minHeight);
    }

    void styleSetMinHeightPercent(float minHeight) noexcept
    {
        YGNodeStyleSetMinHeightPercent(m_node, minHeight);
    }

    YGValue styleGetMinHeight() const noexcept
    {
        return YGNodeStyleGetMinHeight(m_node);
    }

    void styleSetMaxWidth(float maxWidth) noexcept
    {
        YGNodeStyleSetMaxWidth(m_node, maxWidth);
    }

    void styleSetMaxWidthPercent(float maxWidth) noexcept
    {
        YGNodeStyleSetMaxWidthPercent(m_node, maxWidth);
    }

    YGValue styleGetMaxWidth() const noexcept
    {
        return YGNodeStyleGetMaxWidth(m_node);
    }

    void styleSetMaxHeight(float maxHeight) noexcept
    {
        YGNodeStyleSetMaxHeight(m_node, maxHeight);
    }

    void styleSetMaxHeightPercent(float maxHeight) noexcept
    {
        YGNodeStyleSetMaxHeightPercent(m_node, maxHeight);
    }

    YGValue styleGetMaxHeight() const noexcept
    {
        return YGNodeStyleGetMaxHeight(m_node);
    }

    void styleSetFlexWrap(YGWrap flexWrap) noexcept
    {
        YGNodeStyleSetFlexWrap(m_node, flexWrap);
    }

    YGWrap styleGetFlexWrap() const noexcept
    {
        return YGNodeStyleGetFlexWrap(m_node);
    }

    void styleSetOverflow(YGOverflow overflow) noexcept
    {
        YGNodeStyleSetOverflow(m_node, overflow);
    }

    YGOverflow styleGetOverflow() const noexcept
    {
        return YGNodeStyleGetOverflow(m_node);
    }

    void styleSetDisplay(YGDisplay display) noexcept
    {
        YGNodeStyleSetDisplay(m_node, display);
    }

    YGDisplay styleGetDisplay() const noexcept
    {
        return YGNodeStyleGetDisplay(m_node);
    }

    void styleSetFlex(float flex) noexcept
    {
        YGNodeStyleSetFlex(m_node, flex);
    }

    float styleGetFlex() const noexcept
    {
        return YGNodeStyleGetFlex(m_node);
    }

    void styleSetFlexGrow(float flexGrow) noexcept
    {
        YGNodeStyleSetFlexGrow(m_node, flexGrow);
    }

    float styleGetFlexGrow() const noexcept
    {
        return YGNodeStyleGetFlexGrow(m_node);
    }

    void styleSetFlexShrink(float flexShrink) noexcept
    {
        YGNodeStyleSetFlexShrink(m_node, flexShrink);
    }

    float styleGetFlexShrink() const noexcept
    {
        return YGNodeStyleGetFlexShrink(m_node);
    }

    void styleSetFlexBasis(float flexBasis) noexcept
    {
        YGNodeStyleSetFlexBasis(m_node, flexBasis);
    }

    void styleSetFlexBasisPercent(float flexBasis) noexcept
    {
        YGNodeStyleSetFlexBasisPercent(m_node, flexBasis);
    }

    void styleSetFlexBasisAuto() noexcept
    {
        YGNodeStyleSetFlexBasisAuto(m_node);
    }

    YGValue styleGetFlexBasis() const noexcept
    {
        return YGNodeStyleGetFlexBasis(m_node);
    }

    void styleSetMargin(YGEdge edge, float margin) noexcept
    {
        YGNodeStyleSetMargin(m_node, edge, margin);
    }

    void styleSetMarginPercent(YGEdge edge, float margin) noexcept
    {
        YGNodeStyleSetMarginPercent(m_node, edge, margin);
    }

    void styleSetMarginAuto(YGEdge edge) noexcept
    {
        YGNodeStyleSetMarginAuto(m_node, edge);
    }

    YGValue styleGetMargin(YGEdge edge) const noexcept
    {
        return YGNodeStyleGetMargin(m_node, edge);
    }

    void styleSetPadding(YGEdge edge, float padding) noexcept
    {
        YGNodeStyleSetPadding(m_node, edge, padding);
    }

    void styleSetPaddingPercent(YGEdge edge, float padding) noexcept
    {
        YGNodeStyleSetPaddingPercent(m_node, edge, padding);
    }

    YGValue styleGetPadding(YGEdge edge) const noexcept
    {
        return YGNodeStyleGetPadding(m_node, edge);
    }

    void styleSetBorder(YGEdge edge, float border) noexcept
    {
        YGNodeStyleSetBorder(m_node, edge, border);
    }

    float styleGetBorder(YGEdge edge) const noexcept
    {
        return YGNodeStyleGetBorder(m_node, edge);
    }

    void styleSetGap(YGGutter gutter, float gapLength) noexcept
    {
        YGNodeStyleSetGap(m_node, gutter, gapLength);
    }

    void styleSetGapPercent(YGGutter gutter, float gapLength) noexcept
    {
        YGNodeStyleSetGapPercent(m_node, gutter, gapLength);
    }

    float styleGetGap(YGGutter gutter) const noexcept
    {
        return YGNodeStyleGetGap(m_node, gutter);
    }

    void styleSetAspectRatio(float aspectRatio) noexcept
    {
        YGNodeStyleSetAspectRatio(m_node, aspectRatio);
    }

    float styleGetAspectRatio() const noexcept
    {
        return YGNodeStyleGetAspectRatio(m_node);
    }

private:
    friend class AKRenderable;
    friend class AKBakeable;
    friend class AKSubScene;
    friend class AKContainer;
    friend class AKTarget;
    friend class AKScene;

    enum Flags : UInt64
    {
        Visible     = 1L << 0
    };

    struct TargetData
    {
        AKTarget *target { nullptr };
        size_t targetLink;
        SkIRect prevClip; // Rect clipped by parent
        SkIRect prevLocalRect, prevRect;
        SkRegion clientDamage,
            opaque, translucent,
            opaqueOverlay;
        struct {
            GrGLFramebufferInfo fbInfo { 0 };
            GrBackendRenderTarget renderTarget;
            GrGLTextureInfo textureInfo { 0, 0 };
            GrBackendTexture backendTexture;
            sk_sp<SkImage> image;
            sk_sp<SkSurface> surface;
            SkRect srcRect;
        } bake;        
    };

    AKNode(AKNode *parent = nullptr) noexcept;
    bool updateBakeStorage() noexcept;

    AKBitset<Flags> m_flags { Visible };

    SkIRect m_globalRect;
    UInt32 m_caps { 0 };
    YGNodeRef m_node { nullptr };
    AKNode *m_parent { nullptr };
    std::vector<AKNode*> m_children;
    size_t m_parentLink;
    std::unique_ptr<SkRegion> m_inputRegion;
    std::unordered_map<AKTarget*, TargetData> m_targets;
    TargetData *t;
    bool m_insideLastTarget { false };
    bool m_renderedOnLastTarget { false };
};

#endif // AKNODE_H
