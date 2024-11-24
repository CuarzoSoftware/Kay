#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include <AKScene.h>
#include <AKRenderable.h>
#include <AKBakeable.h>
#include <yoga/Yoga.h>
#include <include/core/SkCanvas.h>

using namespace AK;

AKScene::AKScene() noexcept
{

}

AKTarget *AK::AKScene::createTarget() noexcept
{
    m_targets.emplace_back(new AKTarget(this));
    return m_targets.back();
}

bool AKScene::destroyTarget(AKTarget *target)
{
    if (std::find(m_targets.begin(), m_targets.end(), target) == m_targets.end())
        return false;

    delete target;
    return true;
}

bool AKScene::render(AKTarget *target)
{
    // Invalid target
    if (!target || target->scale <= 0.f || !target->surface || std::find(m_targets.begin(), m_targets.end(), target) == m_targets.end())
        return false;
    t = target;

    // Keep the target surface alive
    sk_sp<SkSurface> targetSurfaceRef { target->surface };
    c = target->surface->getCanvas();
    c->save();

    updateMatrix();
    YGNodeCalculateLayout(root()->m_node, YGUndefined, YGUndefined, YGDirectionLTR);

    /* NEW DAMAGE */
    for (std::list<AKNode*>::const_reverse_iterator it = root()->children().crbegin(); it != root()->children().crend(); it++)
        calculateNewDamage(*it);
    t->m_damage.translate(-t->m_viewport.x(), -t->m_viewport.y());

    /* DAMAGE RING */
    if (t->age == 0)
    {
        t->m_damage.setRect(SkIRect::MakeXYWH(0, 0, t->m_viewport.width(), t->m_viewport.height()));
        t->m_damageRing[t->m_damageIndex] = t->m_damage;
    }
    else
    {
        t->m_damageRing[t->m_damageIndex] = t->m_damage;

        for (UInt32 i = 1; i < t->age; i++)
        {
            Int32 damageIndex = t->m_damageIndex - i;

            if (damageIndex < 0)
                damageIndex = 4 + damageIndex;

            t->m_damage.op(t->m_damageRing[damageIndex], SkRegion::Op::kUnion_Op);
        }
    }

    t->outDamageRegion = t->m_damage;
    t->outOpaqueRegion = t->m_opaque;
    t->outOpaqueRegion.translate(-t->m_viewport.x(), -t->m_viewport.y());
    t->m_damage.translate(t->m_viewport.x(), t->m_viewport.y());

    if (t->m_damageIndex == 3)
        t->m_damageIndex = 0;
    else
        t->m_damageIndex++;

    for (std::list<AKNode*>::const_reverse_iterator it = root()->children().crbegin(); it != root()->children().crend(); it++)
        renderOpaque(*it);

    renderBackground();

    for (AKNode *child : root()->children())
        renderTranslucent(child);

    c->restore();

    t->m_damage.setEmpty();
    t->m_opaque.setEmpty();
    return true;
}

void AKScene::updateMatrix() noexcept
{
    t->m_matrix.setIdentity();
    t->m_matrix.preScale(t->scale, t->scale);
    const SkSize size { t->surface->width() / t->scale,  t->surface->height() / t->scale };
    SkPoint trans { -t->pos };

    switch (t->transform) {
    case AK::AKTransform::Normal:
        t->m_viewport.setXYWH(
            t->pos.x(), t->pos.y(),
            size.width(),  size.height());
        break;
    case AK::AKTransform::Rotated90:
        t->m_viewport.setXYWH(
            t->pos.x(), t->pos.y(),
            size.height(),  size.width());
        t->m_matrix.preRotate(-90.f);
        trans.fX -= size.height();
        break;
    case AK::AKTransform::Rotated180:
        t->m_viewport.setXYWH(
            t->pos.x(), t->pos.y(),
            size.width(),  size.height());
        t->m_matrix.preRotate(-180.f);
        trans.fX -= size.width();
        trans.fY -= size.height();
        break;
    case AK::AKTransform::Rotated270:
        t->m_viewport.setXYWH(
            t->pos.x(), t->pos.y(),
            size.height(),  size.width());
        t->m_matrix.preRotate(90.f);
        trans.fY -= size.width();
        break;
    case AK::AKTransform::Flipped:
        t->m_viewport.setXYWH(
            t->pos.x(), t->pos.y(),
            size.width(),  size.height());
        t->m_matrix.preScale(-1.f, 1.f);
        trans.fX -= size.width();
        break;
    case AK::AKTransform::Flipped90:
        t->m_viewport.setXYWH(
            t->pos.x(), t->pos.y(),
            size.height(),  size.width());
        t->m_matrix.preRotate(-90.f);
        t->m_matrix.preScale(-1.f, 1.f);
        break;
    case AK::AKTransform::Flipped180:
        t->m_viewport.setXYWH(
            t->pos.x(), t->pos.y(),
            size.width(),  size.height());
        t->m_matrix.preScale(1.f, -1.f);
        trans.fY -= size.height();
        break;
    case AK::AKTransform::Flipped270:
        t->m_viewport.setXYWH(
            t->pos.x(), t->pos.y(),
            size.height(),  size.width());
        t->m_matrix.preRotate(90.f);
        t->m_matrix.preScale(-1.f, 1.f);
        trans.fY -= size.width();
        trans.fX -= size.height();
        break;
    default:
        break;
    }

    t->m_matrix.preTranslate(trans.x(), trans.y());
    c->setMatrix(t->m_matrix);
}

void AKScene::calculateNewDamage(AKNode *node)
{
    node->t = &node->m_targets[t];

    if (!node->t->target)
    {
        t->m_nodes.push_back(node);
        node->t->target = t;
        node->t->targetLink = t->m_nodes.size() - 1;
    }

    SkIRect parentRect;

    if (node->parent())
        parentRect = node->parent()->globalRect();

    node->m_globalRect = SkIRect::MakeXYWH(
        node->layoutGetLeft() + parentRect.x(),
        node->layoutGetTop() + parentRect.y(),
        node->layoutGetWidth(),
        node->layoutGetHeight());

    if ((node->caps() & AKNode::Bake) && !node->m_globalRect.isEmpty())
    {
        AKBakeable *bake { static_cast<AKBakeable*>(node) };
        if (bake->updateBakeStorage() || node->m_globalRect.size() != bake->t->prevRect.size())
        {
            SkCanvas *canvas { bake->t->bake.surface->getCanvas() };
            canvas->save();
            canvas->clipRect(bake->t->bake.srcRect);
            canvas->setMatrix(SkMatrix::Scale(t->scale, t->scale));
            bake->onBake(canvas);
            canvas->restore();
        }
    }

    SkIRect clip = node->m_globalRect;

    SkIRect localRect = SkIRect::MakeXYWH(
        node->m_globalRect.x() - t->m_viewport.x(),
        node->m_globalRect.y() - t->m_viewport.y(),
        node->m_globalRect.width(),
        node->m_globalRect.height());

    if (node->parent() == root())
    {
        node->m_insideLastTarget = clip.intersect(t->m_viewport);

        if (!node->m_insideLastTarget)
            clip.setEmpty();
    }
    else
    {
        if (!clip.intersect(node->parent()->t->prevClip))
            clip.setEmpty();
        node->m_insideLastTarget = SkIRect::Intersects(node->m_globalRect, t->m_viewport);
    }

    if (localRect == node->t->prevLocalRect)
    {
        // Clip that is now hidden or is now exposed
        SkRegion clipDiff;
        clipDiff.op(node->t->prevClip, SkRegion::Op::kUnion_Op);
        clipDiff.op(clip, SkRegion::Op::kXOR_Op);
        t->m_damage.op(clipDiff, SkRegion::Op::kUnion_Op);

        node->t->clientDamage.translate(
            node->m_globalRect.x(),
            node->m_globalRect.y());

        node->t->clientDamage.op(clip, SkRegion::Op::kIntersect_Op);
        t->m_damage.op(node->t->clientDamage, SkRegion::Op::kUnion_Op);
    }
    else
    {
        // Both current and prev clip need to be repainted
        t->m_damage.op(node->t->prevClip, SkRegion::Op::kUnion_Op);
        t->m_damage.op(clip, SkRegion::Op::kUnion_Op);
    }

    node->t->clientDamage.setEmpty();
    node->t->prevClip = clip;
    node->t->prevLocalRect = localRect;
    node->t->prevRect = node->m_globalRect;

    for (std::list<AKNode*>::const_reverse_iterator it = node->children().crbegin(); it != node->children().crend(); it++)
        calculateNewDamage(*it);

    if ((node->caps() & AKNode::Caps::Render) == 0)
        return;

    AKRenderable *rend { static_cast<AKRenderable*>(node) };
    rend->m_renderedOnLastTarget = !t->m_opaque.contains(clip) && !clip.isEmpty() && !node->m_globalRect.isEmpty();

    if (!rend->m_renderedOnLastTarget)
        return;

    rend->t->opaqueOverlay = t->m_opaque;

    if (rend->opaqueRegion())
    {
        rend->t->opaque = *rend->opaqueRegion();
        rend->t->opaque.translate(node->m_globalRect.x(), node->m_globalRect.y());
        rend->t->opaque.op(clip, SkRegion::kIntersect_Op);
        t->m_opaque.op(rend->t->opaque, SkRegion::kUnion_Op);

        rend->t->translucent.setRect(clip);
        rend->t->translucent.op(rend->t->opaque, SkRegion::kDifference_Op);
    }
    else
    {
        rend->t->opaque.setEmpty();
        rend->t->translucent.setRect(clip);
    }
}

void AKScene::renderOpaque(AKNode *node)
{
    for (std::list<AKNode*>::const_reverse_iterator it = node->children().crbegin(); it != node->children().crend(); it++)
        renderOpaque(*it);

    if (!node->caps() || node->t->opaque.isEmpty() || !node->m_renderedOnLastTarget)
        return;

    AKRenderable *rend { static_cast<AKRenderable*>(node) };
    rend->t->opaque.op(t->m_damage, SkRegion::kIntersect_Op);
    rend->t->opaque.op(rend->t->opaqueOverlay, SkRegion::kDifference_Op);

    if (rend->t->opaque.isEmpty())
        return;

    c->save();
    rend->onRender(c, node->t->opaque, true);
    c->restore();
}

void AKScene::renderBackground() noexcept
{
    c->save();

    SkRegion background { t->m_damage };
    background.op(t->m_opaque, SkRegion::Op::kDifference_Op);

    SkPaint paint;
    paint.setColor(m_clearColor);
    paint.setBlendMode(SkBlendMode::kSrc);

    SkRegion::Iterator it(background);
    while (!it.done())
    {
        c->drawIRect(it.rect(), paint);
        it.next();
    }

    c->restore();
}

void AKScene::renderTranslucent(AKNode *node)
{
    AKRenderable *rend;

    if (!node->caps() || node->t->translucent.isEmpty() || !node->m_renderedOnLastTarget)
        goto skip;

    rend = static_cast<AKRenderable*>(node);
    rend->t->translucent.op(t->m_damage, SkRegion::kIntersect_Op);
    rend->t->translucent.op(rend->t->opaqueOverlay, SkRegion::kDifference_Op);

    if (node->t->translucent.isEmpty())
        goto skip;

    c->save();
    rend->onRender(c, node->t->translucent, false);
    c->restore();

    skip:

    for (AKNode *child : node->children())
        renderTranslucent(child);
}
