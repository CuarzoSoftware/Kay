#include <AKScene.h>
#include <AKRenderable.h>
#include <AKBakeable.h>
#include <cassert>
#include <yoga/Yoga.h>
#include <include/core/SkCanvas.h>
#include <include/gpu/GrDirectContext.h>

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
    validateTarget(target);

    // Keep the target surface alive
    const sk_sp<SkSurface> surfaceRef { target->surface };
    c = surfaceRef->getCanvas();
    c->save();

    const bool isNestedScene = (t->root->caps() & AKNode::Scene);

    if (!isNestedScene)
    {
        YGNodeCalculateLayout(t->root->m_node, YGUndefined, YGUndefined, YGDirectionInherit);
        t->root->m_globalRect.fLeft = t->root->layoutGetLeft();
        t->root->m_globalRect.fTop = t->root->layoutGetTop();
    }

    t->m_globalIViewport = SkIRect::MakeXYWH(
        t->viewport.x() + float(t->root->globalRect().x()),
        t->viewport.y() + float(t->root->globalRect().y()),
        t->viewport.width(), t->viewport.height());

    updateMatrix();

    for (auto it = t->root->children().crbegin(); it != t->root->children().crend(); it++)
        calculateNewDamage(*it);

    updateDamageRing();

    for (auto it = t->root->children().crbegin(); it != t->root->children().crend(); it++)
        renderOpaque(*it);

    renderBackground();

    for (auto *child : t->root->children())
        renderTranslucent(child);

    t->m_damage.setEmpty();
    t->m_opaque.setEmpty();

    c->restore();
    return true;
}

void AKScene::validateTarget(AKTarget *target) noexcept
{
    assert("AKTarget is nullptr" && target);
    assert("AKTarget wasn't created by this scene" && target->m_scene == this);
    assert("AKTarget has no surface" && target->surface);
    assert("Invalid surface size" && target->surface->width() > 0 && target->surface->height() > 0);
    assert("Invalid buffer age" && target->age <= sizeof(target->m_damageRing)/sizeof(target->m_damageRing[0]));
    assert("Invalid scale factor" && target->scale > 0.f);
    assert("Invalid viewport" && !target->viewport.isEmpty());
    assert("Invalid dstRect" && !target->dstRect.isEmpty());
    assert("Root node is nullptr" && target->root);
    t = target;
}

void AKScene::updateMatrix() noexcept
{
    SkMatrix viewportMatrix;
    viewportMatrix.preScale(t->scale, t->scale);
    SkPoint trans ( -t->viewport.x() - t->root->globalRect().left(), -t->viewport.y() - t->root->globalRect().top());
    switch (t->transform) {
    case AK::AKTransform::Normal:
        break;
    case AK::AKTransform::Rotated90:
        viewportMatrix.preRotate(-90.f);
        trans.fX -= t->viewport.width();
        break;
    case AK::AKTransform::Rotated180:
        viewportMatrix.preRotate(-180.f);
        trans.fX -= t->viewport.width();
        trans.fY -= t->viewport.height();
        break;
    case AK::AKTransform::Rotated270:
        viewportMatrix.preRotate(90.f);
        trans.fY -= t->viewport.height();
        break;
    case AK::AKTransform::Flipped:
        viewportMatrix.preScale(-1.f, 1.f);
        trans.fX -= t->viewport.width();
        break;
    case AK::AKTransform::Flipped90:
        viewportMatrix.preRotate(-90.f);
        viewportMatrix.preScale(-1.f, 1.f);
        break;
    case AK::AKTransform::Flipped180:
        viewportMatrix.preScale(1.f, -1.f);
        trans.fY -= t->viewport.height();
        break;
    case AK::AKTransform::Flipped270:
        viewportMatrix.preRotate(90.f);
        viewportMatrix.preScale(-1.f, 1.f);
        trans.fY -= t->viewport.height();
        trans.fX -= t->viewport.width();
        break;
    }

    viewportMatrix.preTranslate(trans.x(), trans.y());

    t->m_matrix.setIdentity();

    if (is90Transform(t->transform))
        t->m_xyScale = {
            float(t->dstRect.width())/(t->viewport.height()),
            float(t->dstRect.height())/(t->viewport.width())};
    else
        t->m_xyScale = {
            float(t->dstRect.width())/(t->viewport.width()),
            float(t->dstRect.height())/(t->viewport.height())};

    t->m_matrix.preScale(t->m_xyScale.x(), t->m_xyScale.y());
    t->m_matrix.preTranslate(t->dstRect.x(), t->dstRect.y());
    t->m_matrix.preConcat(viewportMatrix);
    c->setMatrix(t->m_matrix);

    /*
    SkRegion exposedViewport;

    exposedViewport.op(
        SkIRect(
            t->viewport.fLeft,
            t->viewport.fTop,
            t->viewport.fRight,
            t->viewport.fBottom),
        SkRegion::Op::kUnion_Op);

    exposedViewport.op(
        SkIRect(
            t->m_prevViewport.fLeft,
            t->m_prevViewport.fTop,
            t->m_prevViewport.fRight,
            t->m_prevViewport.fBottom),
        SkRegion::Op::kDifference_Op);

    t->m_damage.op(exposedViewport, SkRegion::Op::kUnion_Op);*/

    t->m_prevViewport = t->viewport;
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

    node->m_globalRect = SkIRect::MakeXYWH(
        node->layoutGetLeft() + node->parent()->globalRect().x(),
        node->layoutGetTop() + node->parent()->globalRect().y(),
        node->layoutGetWidth(),
        node->layoutGetHeight());

    SkIRect clip;

    if (node->isVisible())
        clip = node->m_globalRect;

    SkIRect localRect = SkIRect::MakeXYWH(
        node->m_globalRect.x() - t->m_globalIViewport.x(),
        node->m_globalRect.y() - t->m_globalIViewport.y(),
        node->m_globalRect.width(),
        node->m_globalRect.height());

    if (node->parent() == t->root)
    {
        if (!clip.intersect(t->m_globalIViewport))
            clip.setEmpty();
    }
    else if (!clip.intersect(node->parent()->t->prevClip))
        clip.setEmpty();

    node->m_insideLastTarget = SkIRect::Intersects(node->m_globalRect, t->m_globalIViewport);

    if ((node->caps() & AKNode::Bake) && !clip.isEmpty())
    {
        AKBakeable *bake { static_cast<AKBakeable*>(node) };
        const bool surfaceChanged { bake->updateBakeStorage() };
        SkCanvas *canvas { bake->t->bake.surface->getCanvas() };
        canvas->save();
        canvas->setMatrix(SkMatrix::Scale(t->m_xyScale.x(), t->m_xyScale.y()));
        bake->onBake(canvas,
            SkRect::MakeXYWH(
                clip.x() - node->m_globalRect.x(),
                clip.y() - node->m_globalRect.y(),
                clip.width(), clip.height()),
            surfaceChanged);
        canvas->restore();
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

    if (!(node->caps() & AKNode::Scene))
        for (auto it = node->children().crbegin(); it != node->children().crend(); it++)
            calculateNewDamage(*it);

    if ((node->caps() & AKNode::Caps::Render) == 0)
        return;

    AKRenderable *rend { static_cast<AKRenderable*>(node) };
    rend->m_renderedOnLastTarget = !t->m_opaque.contains(clip) && !clip.isEmpty() && !node->m_globalRect.isEmpty();

    if (!rend->m_renderedOnLastTarget)
        return;

    rend->t->opaqueOverlay = t->m_opaque;

    rend->t->opaque = rend->opaqueRegion();
    rend->t->opaque.translate(node->m_globalRect.x(), node->m_globalRect.y());
    rend->t->opaque.op(clip, SkRegion::kIntersect_Op);
    t->m_opaque.op(rend->t->opaque, SkRegion::kUnion_Op);
    rend->t->translucent.setRect(clip);
    rend->t->translucent.op(rend->t->opaque, SkRegion::kDifference_Op);
}

void AKScene::updateDamageRing() noexcept
{
    t->m_damage.translate(-t->m_globalIViewport.x(), -t->m_globalIViewport.y());

    if (t->age == 0)
    {
        t->m_damage.setRect(SkIRect::MakeXYWH(0, 0, t->m_globalIViewport.width(), t->m_globalIViewport.height()));
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

    if (t->outDamageRegion)
        *t->outDamageRegion = t->m_damage;

    if (t->outOpaqueRegion)
    {
        *t->outOpaqueRegion = t->m_opaque;
        t->outOpaqueRegion->translate(-t->m_globalIViewport.x(), -t->m_globalIViewport.y());
    }

    t->m_damage.translate(t->m_globalIViewport.x(), t->m_globalIViewport.y());

    if (t->m_damageIndex == 3)
        t->m_damageIndex = 0;
    else
        t->m_damageIndex++;
}

void AKScene::renderOpaque(AKNode *node)
{
    if (!(node->caps() & AKNode::Scene))
        for (auto it = node->children().crbegin(); it != node->children().crend(); it++)
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

    if (!(node->caps() & AKNode::Scene))
        for (AKNode *child : node->children())
            renderTranslucent(child);
}
