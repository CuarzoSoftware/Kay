#include <include/core/SkColorSpace.h>
#include <include/gpu/ganesh/SkImageGanesh.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>

#include <AK/AKTarget.h>
#include <AK/AKNode.h>
#include <GL/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <cassert>
#include <yoga/Yoga.h>

using namespace AK;

AK::AKNode::AKNode(AKNode *parent) noexcept
{
    setParent(parent);
}

AKNode::~AKNode()
{
    for (auto &t : m_targets)
    {
        t.second.target->m_damage.op(t.second.prevLocalClip, SkRegion::kUnion_Op);
        t.second.target->m_nodes[t.second.targetLink] = t.second.target->m_nodes.back();
        t.second.target->m_nodes.back()->m_targets[t.first].targetLink = t.second.targetLink;
        t.second.target->m_nodes.pop_back();
    }

    setParent(nullptr);
}

bool AKNode::updateBakeStorage() noexcept
{
    t->bake.scale = std::max(SkScalarCeilToScalar(t->target->m_xyScale.x()), SkScalarCeilToScalar(t->target->m_xyScale.y()));
    t->bake.srcRect = SkRect::MakeXYWH(0, 0,
                                       m_globalRect.width() * t->target->m_xyScale.x(),
                                       m_globalRect.height() *  t->target->m_xyScale.y());

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

AK::AKNode *AKNode::closestClipperParent() const noexcept
{
    assert(parent() != nullptr);

    if (parent()->clipsChildren() || parent() == t->target->root)
        return parent();

    return parent()->closestClipperParent();
}

void AKNode::setParent(AKNode *parent) noexcept
{
    assert(!parent || (parent != this && !parent->isSubchildOf(this)));

    if (m_parent)
    {
        YGNodeRemoveChild(m_parent->layout().m_node, layout().m_node);
        auto next = m_parent->m_children.erase(m_parent->m_children.begin() + m_parentLink);
        for (; next != m_parent->m_children.end(); next++) (*next)->m_parentLink--;
    }

    m_parent = parent;

    if (parent)
    {
        m_parentLink = YGNodeGetChildCount(parent->layout().m_node);
        YGNodeInsertChild(parent->layout().m_node, layout().m_node, m_parentLink);
        parent->m_children.push_back(this);
    }
}

void AKNode::insertBefore(AKNode *other) noexcept
{
    assert(!other || !other->isSubchildOf(this));

    if (other == this)
        return;

    if (other)
    {
        if (other->parent())
        {
            setParent(nullptr);
            m_parent = other->parent();
            m_parentLink = other->m_parentLink;
            YGNodeInsertChild(m_parent->layout().m_node, layout().m_node, m_parentLink);
            auto next = m_parent->m_children.insert(m_parent->m_children.begin() + m_parentLink, this) + 1;
            for (; next != m_parent->m_children.end(); next++) (*next)->m_parentLink++;
        }
        else
        {
            setParent(nullptr);
        }
    }
    else if (parent())
    {
        setParent(parent());
    }
}

void AKNode::insertAfter(AKNode *other) noexcept
{
    assert(!other || !other->isSubchildOf(this));

    if (other == this)
        return;

    if (other)
    {
        if (other->parent())
        {
            setParent(nullptr);

            if (other->parent()->children().back() == other)
            {
                setParent(other->parent());
                return;
            }

            m_parent = other->parent();
            m_parentLink = other->m_parentLink + 1;
            YGNodeInsertChild(m_parent->layout().m_node, layout().m_node, m_parentLink);
            auto next = m_parent->m_children.insert(m_parent->m_children.begin() + m_parentLink, this) + 1;
            for (; next != m_parent->m_children.end(); next++) (*next)->m_parentLink++;
        }
        else
        {
            setParent(nullptr);
        }
    }
    else if (parent())
    {
        insertBefore(parent()->children().front());
    }
}

