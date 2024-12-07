#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include <AK/AKScene.h>
#include <AK/nodes/AKRenderable.h>
#include <AK/nodes/AKBakeable.h>
#include <AK/effects/AKBackgroundEffect.h>
#include <AK/AKSurface.h>
#include <AK/AKPainter.h>
#include <cassert>
#include <yoga/Yoga.h>
#include <include/core/SkCanvas.h>
#include <include/gpu/GrDirectContext.h>

using namespace AK;

AKTarget *AK::AKScene::createTarget(std::shared_ptr<AKPainter> painter) noexcept
{
    m_targets.emplace_back(new AKTarget(this, painter));
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

    target->surface->recordingContext()->asDirectContext()->resetContext();
    c = target->surface->getCanvas();
    c->save();

    updateMatrix();

    for (Int64 i = t->root->children().size() - 1; i >= 0;)
    {
        const bool noBackroundEffect { t->root->children()[i]->backgroundEffect() == nullptr };

        calculateNewDamage(t->root->children()[i]);

        if (noBackroundEffect)
            i--;
    }
    target->surface->recordingContext()->asDirectContext()->flushAndSubmit();
    target->painter()->bindProgram();
    target->painter()->bindTarget(t);

    updateDamageRing();

    glDisable(GL_BLEND);

    for (auto it = t->root->children().crbegin(); it != t->root->children().crend(); it++)
        renderOpaque(*it);

    renderBackground();

    glEnable(GL_BLEND);

    for (size_t i = 0; i < t->root->children().size();)
    {
        renderTranslucent(t->root->children()[i]);

        t->root->children()[i]->t->changes.reset();

        if (t->root->children()[i]->caps() & AKNode::BackgroundEffect)
            t->root->children()[i]->setParent(nullptr);
        else
            i++;
    }

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

    auto skTarget = SkSurfaces::GetBackendRenderTarget(t->surface.get(), SkSurfaces::BackendHandleAccess::kFlushRead);
    GrGLFramebufferInfo fbInfo;
    skTarget.getGLFramebufferInfo(&fbInfo);
    t->m_fbId = fbInfo.fFBOID;
    t->painter()->bindTarget(t);
}

static void addMargin(SkRegion &region, Int32 margin) noexcept
{
    SkRegion damage = region;
    region.setEmpty();

    SkRegion::Iterator it(damage);
    while (!it.done())
    {
        region.op(
            SkIRect(it.rect().fLeft - margin,
                    it.rect().fTop - margin,
                    it.rect().fRight + margin,
                    it.rect().fBottom + margin), SkRegion::Op::kUnion_Op);
        it.next();
    }
}

void AKScene::updateMatrix() noexcept
{
    const bool isNestedScene = (t->root->parent() && t->m_isSubScene);

    if (!isNestedScene)
    {
        YGNodeCalculateLayout(t->root->layout().m_node,
                              YGUndefined,
                              YGUndefined,
                              YGDirectionInherit);

        t->root->m_globalRect = SkRect::MakeWH(t->viewport.width(), t->viewport.height()).roundOut();
        t->root->m_rect = t->root->m_globalRect;
    }

    t->m_globalIViewport = SkRect::MakeXYWH(
        t->viewport.x() + float(t->root->globalRect().x()),
        t->viewport.y() + float(t->root->globalRect().y()),
        t->viewport.width(), t->viewport.height()).roundOut();

    SkMatrix viewportMatrix;
    viewportMatrix.preScale(t->scale, t->scale);
    SkPoint trans ( -t->viewport.x(), -t->viewport.y());
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

    t->m_matrix.preTranslate(t->dstRect.x(), t->dstRect.y());
    t->m_matrix.preScale(t->m_xyScale.x(), t->m_xyScale.y());
    t->m_matrix.preConcat(viewportMatrix);
    c->setMatrix(t->m_matrix);

    /*
    UInt32 prevDamageIndex;

    if (t->m_damageIndex == 0)
        prevDamageIndex = 3;
    else
        prevDamageIndex = t->m_damageIndex - 1;

    // Damage exposed region that has never been painted
    const SkRegion removedNodesDamage { t->m_damage };
    t->m_damage = t->m_damageRing[prevDamageIndex];
    t->m_damage.op(t->m_prevClip, SkRegion::Op::kDifference_Op);
    t->m_damage.op(removedNodesDamage, SkRegion::Op::kUnion_Op);*/

    t->m_prevViewport = t->viewport.roundOut();

    if (t->inClipRegion)
        t->m_prevClip = *t->inClipRegion;
    else
        t->m_prevClip.setRect(t->viewport.roundOut());
}

void AKScene::calculateNewDamage(AKNode *node)
{
    node->t = &node->m_targets[t];

    if (!node->t->target)
    {
        node->t->target = t;
        t->m_nodes.push_back(node);
        node->t->targetLink = t->m_nodes.size() - 1;
    }

    if (node->caps() & AKNode::BackgroundEffect)
    {
        AKBackgroundEffect &backgroundEffect { *static_cast<AKBackgroundEffect*>(node) };

        node->onSceneBegin();

        backgroundEffect.m_globalRect = SkIRect::MakeXYWH(
            backgroundEffect.rect.x() + backgroundEffect.targetNode()->globalRect().x(),
            backgroundEffect.rect.y() + backgroundEffect.targetNode()->globalRect().y(),
            backgroundEffect.rect.width(),
            backgroundEffect.rect.height());

        backgroundEffect.m_rect = SkIRect::MakeXYWH(
            backgroundEffect.m_globalRect.x() - t->root->m_globalRect.x(),
            backgroundEffect.m_globalRect.y() - t->root->m_globalRect.y(),
            backgroundEffect.m_globalRect.width(),
            backgroundEffect.m_globalRect.height());
    }
    else
    {
        if (node->layout().display() == YGDisplayNone)
        {
            node->m_globalRect = SkRect::MakeXYWH(
                node->layout().position(YGEdgeLeft).value + float(node->parent()->globalRect().x()),
                node->layout().position(YGEdgeTop).value + float(node->parent()->globalRect().y()),
                node->layout().width().value,
                node->layout().height().value).roundOut();
        }
        else
        {
            node->m_globalRect = SkRect::MakeXYWH(
                 node->layout().calculatedLeft() + float(node->parent()->globalRect().x()),
                 node->layout().calculatedTop() + float(node->parent()->globalRect().y()),
                 node->layout().calculatedWidth(),
                 node->layout().calculatedHeight()).roundOut();
        }

        node->m_rect = SkIRect::MakeXYWH(
            node->m_globalRect.x() - t->root->m_globalRect.x(),
            node->m_globalRect.y() - t->root->m_globalRect.y(),
            node->m_globalRect.width(),
            node->m_globalRect.height());

        node->onSceneBegin();
    }

    if (node->backgroundEffect())
    {
        if (node->backgroundEffect()->stackPosition() == AKBackgroundEffect::Behind)
            node->backgroundEffect()->insertBefore(node);
        else
            node->backgroundEffect()->insertBefore(node->parent()->children().front());
    }

    SkRegion clip;

    if (node->visible())
        clip.setRect(node->m_rect);

    AKNode *clipper { node->closestClipperParent() };

    if (clipper == t->root)
        clip.op(t->viewport.roundOut(), SkRegion::Op::kIntersect_Op);
    else
        clip.op(clipper->t->prevLocalClip, SkRegion::Op::kIntersect_Op);

    clip.op(t->m_opaque, SkRegion::Op::kDifference_Op);
    node->t->prevLocalClip.op(t->m_opaque, SkRegion::Op::kDifference_Op);
    node->m_insideLastTarget = SkIRect::Intersects(node->m_rect, t->viewport.roundOut());

    if ((node->caps() & AKNode::Bake) && !clip.isEmpty())
    {
        auto *bakeable { static_cast<AKBakeable*>(node) };

        SkRegion clipRegion = clip;
        clipRegion.op(t->m_prevClip, SkRegion::Op::kIntersect_Op);
        clipRegion.translate(-bakeable->m_rect.x(), -bakeable->m_rect.y());

        if (!clipRegion.isEmpty())
        {
            bool surfaceChanged;

            if (bakeable->t->bake)
            {
                surfaceChanged = bakeable->t->bake->resize(
                    SkSize::Make(bakeable->rect().size()),
                    t->m_xyScale);
            }
            else
            {
                surfaceChanged = true;
                bakeable->t->bake = AKSurface::Make(
                    t->surface->recordingContext(),
                    SkSize::Make(bakeable->rect().size()),
                    t->m_xyScale, true);
            }

            AKBakeable::OnBakeParams params
            {
                .clip = &clipRegion,
                .damage = &bakeable->t->clientDamage,
                .opaque = &bakeable->opaqueRegion,
                .surface = bakeable->t->bake
            };

            if (surfaceChanged)
                params.damage->setRect(AK_IRECT_INF);

            SkCanvas &canvas { *params.surface->surface()->getCanvas() };
            canvas.save();
            canvas.scale(params.surface->scale().x(), params.surface->scale().y());
            bakeable->onBake(&params);
            canvas.restore();
            //params.surface->surface()->flush();
        }
    }

    if (node->m_rect.fLeft == node->t->prevLocalRect.fLeft && node->m_rect.fTop == node->t->prevLocalRect.fTop)
    {
        node->t->prevLocalClip.op(clip, SkRegion::Op::kXOR_Op);
        t->m_damage.op(node->t->prevLocalClip, SkRegion::Op::kUnion_Op);

        node->t->clientDamage.translate(
            node->m_rect.x(),
            node->m_rect.y());

        node->t->clientDamage.op(clip, SkRegion::Op::kIntersect_Op);
        t->m_damage.op(node->t->clientDamage, SkRegion::Op::kUnion_Op);
    }
    else
    {
        // Both current and prev clip need to be repainted
        t->m_damage.op(node->t->prevLocalClip, SkRegion::Op::kUnion_Op);
        t->m_damage.op(clip, SkRegion::Op::kUnion_Op);
    }

    node->t->clientDamage.setEmpty();
    node->t->prevLocalClip = clip;
    node->t->prevLocalRect = node->m_rect;
    node->t->prevRect = node->m_globalRect;

    if (!(node->caps() & AKNode::Scene))
        for (Int64 i = node->children().size() - 1; i >= 0;)
        {
            const bool noBackroundEffect { node->children()[i]->backgroundEffect() == nullptr };

            calculateNewDamage(node->children()[i]);

            if (noBackroundEffect)
                i--;
        }

    if ((node->caps() & AKNode::Caps::Render) == 0)
        return;

    AKRenderable *rend { static_cast<AKRenderable*>(node) };
    rend->m_renderedOnLastTarget = !t->m_opaque.contains(clip) && !clip.isEmpty() && !node->m_globalRect.isEmpty();

    if (!rend->m_renderedOnLastTarget)
        return;

    rend->t->opaqueOverlay = t->m_opaque;

    switch (rend->colorHint())
    {
    case AKRenderable::ColorHint::Opaque:
        rend->t->opaque = clip;
        break;
    case AKRenderable::ColorHint::Translucent:
        rend->t->opaque.setEmpty();
        break;
    case AKRenderable::ColorHint::UseOpaqueRegion:
        rend->opaqueRegion.translate(node->m_rect.x(), node->m_rect.y(), &rend->t->opaque);
        rend->t->opaque.op(clip, SkRegion::kIntersect_Op);
        break;
    }

    t->m_opaque.op(rend->t->opaque, SkRegion::kUnion_Op);
    rend->t->translucent = clip;
    rend->t->translucent.op(rend->t->opaque, SkRegion::kDifference_Op);
}

void AKScene::updateDamageRing() noexcept
{
    if (t->age == 0)
    {
        t->m_damage.setRect(AK_IRECT_INF);
        t->m_damageRing[t->m_damageIndex] = t->m_damage;
    }
    else
    {
        if (t->inDamageRegion)
            t->m_damage.op(*t->inDamageRegion, SkRegion::Op::kUnion_Op);

        t->m_damageRing[t->m_damageIndex] = t->m_damage;

        for (UInt32 i = 1; i < t->age; i++)
        {
            Int32 damageIndex = t->m_damageIndex - i;

            if (damageIndex < 0)
                damageIndex = 4 + damageIndex;

            t->m_damage.op(t->m_damageRing[damageIndex], SkRegion::Op::kUnion_Op);
        }
    }

    t->m_damage.op(t->m_prevViewport, SkRegion::Op::kIntersect_Op);
    t->m_opaque.op(t->m_prevViewport, SkRegion::Op::kIntersect_Op);

    if (t->inClipRegion)
        t->m_damage.op(*t->inClipRegion, SkRegion::Op::kIntersect_Op);

    if (t->outDamageRegion)
        *t->outDamageRegion = t->m_damage;

    if (t->outOpaqueRegion)
        *t->outOpaqueRegion = t->m_opaque;

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

    t->painter()->setParamsFromRenderable(rend);
    rend->onRender(t->painter().get(), node->t->opaque);
}

void AKScene::renderBackground() noexcept
{
    SkRegion background { t->m_damage };
    background.op(t->m_opaque, SkRegion::Op::kDifference_Op);
    t->painter()->enableAutoBlendFunc(true);
    t->painter()->setColorFactor(1.f, 1.f, 1.f, 1.f);
    t->painter()->setAlpha(m_clearColor.fA);
    t->painter()->setColor(m_clearColor);
    t->painter()->bindColorMode();
    t->painter()->drawRegion(background);
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

    t->painter()->setParamsFromRenderable(rend);
    rend->onRender(t->painter().get(), node->t->translucent);

    skip:

    if (!(node->caps() & AKNode::Scene))
        for (size_t i = 0; i < node->children().size();)
        {
            renderTranslucent(node->children()[i]);

            node->children()[i]->t->changes.reset();

            if (node->children()[i]->caps() & AKNode::BackgroundEffect)
                node->children()[i]->setParent(nullptr);
            else
                i++;
        }
}
