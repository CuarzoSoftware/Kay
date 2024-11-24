#include <include/core/SkColorSpace.h>
#include <include/gpu/ganesh/SkImageGanesh.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>

#include <AKTarget.h>
#include <AKNode.h>
#include <GL/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <cassert>
#include <yoga/Yoga.h>

using namespace AK;

AK::AKNode::AKNode(AKNode *parent) noexcept :
    m_node(YGNodeNew())
{
    allNodes().push_back(this);
    m_allNodesIndex = allNodes().size() - 1;
    setParent(parent);
}

bool AKNode::updateBakeStorage() noexcept
{
    t->bake.srcRect = SkRect::MakeXYWH(0, 0,
        SkScalarCeilToInt(m_globalRect.width() * t->target->scale),
        SkScalarCeilToInt(m_globalRect.height() * t->target->scale));

    // No resizing required
    if (t->bake.image && t->bake.image->width() >= t->bake.srcRect.width() && t->bake.image->height() >= t->bake.srcRect.height())
        return false;

    if (t->bake.image)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &t->bake.fbInfo.fFBOID);
        glDeleteTextures(1, &t->bake.textureInfo.fID);
    }

    t->bake.textureInfo.fTarget = GL_TEXTURE_2D;
    t->bake.textureInfo.fFormat = GL_RGBA8;
    glGenTextures(1, &t->bake.textureInfo.fID);
    glBindTexture(t->bake.textureInfo.fTarget, t->bake.textureInfo.fID);
    glTexImage2D(t->bake.textureInfo.fTarget, 0, GL_RGBA, t->bake.srcRect.width(), t->bake.srcRect.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    t->bake.backendTexture = GrBackendTexture(
        t->bake.srcRect.width(),
        t->bake.srcRect.height(),
        GrMipMapped::kNo,
        t->bake.textureInfo);

    t->bake.image = SkImages::BorrowTextureFrom(
        t->target->surface->recordingContext(),
        t->bake.backendTexture,
        GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
        kRGBA_8888_SkColorType,
        SkAlphaType::kPremul_SkAlphaType,
        SkColorSpace::MakeSRGB(),
        nullptr,
        nullptr);

    assert("Failed to create SkImage" && t->bake.image);

    glGenFramebuffers(1, &t->bake.fbInfo.fFBOID);
    glBindFramebuffer(GL_FRAMEBUFFER, t->bake.fbInfo.fFBOID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, t->bake.textureInfo.fID, 0);
    t->bake.fbInfo.fFormat = GL_RGBA8;

    t->bake.renderTarget = GrBackendRenderTarget(
        t->bake.srcRect.width(),
        t->bake.srcRect.height(),
        0, 0,
        t->bake.fbInfo);

    static SkSurfaceProps skSurfaceProps(0, kUnknown_SkPixelGeometry);

    t->bake.surface = SkSurfaces::WrapBackendRenderTarget(
        t->target->surface->recordingContext(),
        t->bake.renderTarget,
        GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
        SkColorType::kRGBA_8888_SkColorType,
        SkColorSpace::MakeSRGB(),
        &skSurfaceProps);

    assert("Failed to create SkSurface" && t->bake.surface);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_FRAMEBUFFER, 0);
    return true;
}

AKNode::~AKNode()
{
    allNodes()[m_allNodesIndex] = allNodes().back();
    allNodes()[m_allNodesIndex]->m_allNodesIndex = m_allNodesIndex;
    allNodes().pop_back();

    for (auto &t : m_targets)
    {
        t.first->m_damage.op(t.second.prevClip, SkRegion::kUnion_Op);
        t.first->m_nodes[t.second.targetLink] = t.first->m_nodes.back();
        t.first->m_nodes.back()->m_targets[t.first].targetLink = t.second.targetLink;
        t.first->m_nodes.pop_back();
    }

    YGNodeFree(m_node);
    setParent(nullptr);
}

void AKNode::setParent(AKNode *parent) noexcept
{
    assert(!parent || (parent != this && !parent->isSubchildOf(this)));

    if (m_parent)
    {
        YGNodeRemoveChild(m_parent->m_node, m_node);
        m_parent->m_children.erase(m_parentLink);
    }

    m_parent = parent;

    if (parent)
    {
        m_nodeIndex = YGNodeGetChildCount(parent->m_node);
        YGNodeInsertChild(parent->m_node, m_node, m_nodeIndex);
        parent->m_children.push_back(this);
        m_parentLink = std::prev(parent->m_children.end());
    }
}

