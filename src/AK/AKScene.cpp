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

    c = target->surface()->getCanvas();
    c->save();

    updateMatrix();

    for (auto it = t->root()->children().rbegin(); it != t->root()->children().rend(); it++)
        notifyBegin(*it);

    const bool isNestedScene = (t->root()->parent() && t->m_isSubScene);

    if (!isNestedScene)
    {
        //YGConfigSetPointScaleFactor(t->m_yogaConfig, t->xyScale().x());
        //YGNodeSetConfig(t->root()->layout().m_node, t->m_yogaConfig);
        YGNodeCalculateLayout(t->root()->layout().m_node,
                              YGUndefined,
                              YGUndefined,
                              YGDirectionInherit);

        t->root()->m_globalRect = SkRect::MakeWH(t->viewport().width(), t->viewport().height()).roundOut();
        t->root()->m_rect = t->root()->m_globalRect;
    }

    t->m_globalIViewport = SkRect::MakeXYWH(
                               t->viewport().x() + float(t->root()->globalRect().x()),
                               t->viewport().y() + float(t->root()->globalRect().y()),
                               t->viewport().width(), t->viewport().height()).roundOut();

    for (Int64 i = t->root()->children().size() - 1; i >= 0;)
    {
        const int skip = t->root()->children()[i]->backgroundEffects().size();
        calculateNewDamage(t->root()->children()[i]);
        i -= 1 - skip;
    }

    target->surface()->recordingContext()->asDirectContext()->resetContext();
    target->surface()->recordingContext()->asDirectContext()->flush();
    target->painter()->bindProgram();
    target->painter()->bindTarget(t);

    updateDamageRing();
    renderBackground();

    for (size_t i = 0; i < t->root()->children().size();)
    {
        renderNodes(t->root()->children()[i]);

        t->root()->children()[i]->t->changes.reset();

        if (t->root()->children()[i]->caps() & AKNode::BackgroundEffect)
            t->root()->children()[i]->setParent(nullptr);
        else
            i++;
    }

    t->m_isDirty = false;
    t->m_needsFullRepaint = false;
    t->m_damage.setEmpty();
    t->m_opaque.setEmpty();
    c->restore();
    return true;
}

void AKScene::validateTarget(AKTarget *target) noexcept
{
    assert("AKTarget is nullptr" && target);
    assert("AKTarget wasn't created by this scene" && target->m_scene == this);
    assert("AKTarget has no surface" && target->m_surface);
    assert("Invalid surface size" && target->m_surface->width() > 0 && target->m_surface->height() > 0);
    assert("Invalid viewport" && !target->viewport().isEmpty());
    assert("Invalid dstRect" && !target->dstRect().isEmpty());
    assert("Root node is nullptr" && target->root());
    t = target;

    if (target->age() > AK_MAX_BUFFER_AGE || target->m_needsFullRepaint)
        target->setAge(0);

    auto skTarget = SkSurfaces::GetBackendRenderTarget(t->m_surface.get(), SkSurfaces::BackendHandleAccess::kFlushRead);
    GrGLFramebufferInfo fbInfo;
    skTarget.getGLFramebufferInfo(&fbInfo);
    t->m_fbId = fbInfo.fFBOID;
    t->painter()->bindTarget(t);
}

void AKScene::updateMatrix() noexcept
{
    SkMatrix viewportMatrix;
    SkPoint trans ( -t->viewport().x(), -t->viewport().y());
    switch (t->transform()) {
    case AK::AKTransform::Normal:
        break;
    case AK::AKTransform::Rotated90:
        viewportMatrix.preRotate(-90.f);
        trans.fX -= t->viewport().width();
        break;
    case AK::AKTransform::Rotated180:
        viewportMatrix.preRotate(-180.f);
        trans.fX -= t->viewport().width();
        trans.fY -= t->viewport().height();
        break;
    case AK::AKTransform::Rotated270:
        viewportMatrix.preRotate(90.f);
        trans.fY -= t->viewport().height();
        break;
    case AK::AKTransform::Flipped:
        viewportMatrix.preScale(-1.f, 1.f);
        trans.fX -= t->viewport().width();
        break;
    case AK::AKTransform::Flipped90:
        viewportMatrix.preRotate(-90.f);
        viewportMatrix.preScale(-1.f, 1.f);
        break;
    case AK::AKTransform::Flipped180:
        viewportMatrix.preScale(1.f, -1.f);
        trans.fY -= t->viewport().height();
        break;
    case AK::AKTransform::Flipped270:
        viewportMatrix.preRotate(90.f);
        viewportMatrix.preScale(-1.f, 1.f);
        trans.fY -= t->viewport().height();
        trans.fX -= t->viewport().width();
        break;
    }

    viewportMatrix.preTranslate(trans.x(), trans.y());

    t->m_matrix.setIdentity();

    if (is90Transform(t->transform()))
        t->m_xyScale = {
            float(t->dstRect().width())/(t->viewport().height()),
            float(t->dstRect().height())/(t->viewport().width())};
    else
        t->m_xyScale = {
            float(t->dstRect().width())/(t->viewport().width()),
            float(t->dstRect().height())/(t->viewport().height())};

    t->m_matrix.preTranslate(t->dstRect().x(), t->dstRect().y());
    t->m_matrix.preScale(t->m_xyScale.x(), t->m_xyScale.y());
    t->m_matrix.preConcat(viewportMatrix);
    c->setMatrix(t->m_matrix);

    t->m_prevViewport = t->viewport().roundOut();

    if (t->inClipRegion)
        t->m_prevClip = *t->inClipRegion;
    else
        t->m_prevClip.setRect(t->viewport().roundOut());
}

void AKScene::notifyBegin(AKNode *node)
{
    node->t = &node->m_targets[t];

    if (!node->t->target)
    {
        node->t->target = t;
        t->AKObject::on.destroyed.subscribe(node, [node](AKObject *object){
            AKTarget *target { static_cast<AKTarget*>(object) };
            node->m_targets.erase(target);
        });
        node->t->clientDamage.setRect(AK_IRECT_INF);
    }

    if (!t->m_isSubScene)
        for (auto it = node->children().rbegin(); it != node->children().rend(); it++)
            notifyBegin(*it);

    node->onSceneBegin();
}

void AKScene::calculateNewDamage(AKNode *node)
{
    node->t = &node->m_targets[t];

    if (!node->t->target)
    {
        node->t->target = t;
        t->AKObject::on.destroyed.subscribe(node, [node](AKObject *object){
            AKTarget *target { static_cast<AKTarget*>(object) };
            node->m_targets.erase(target);
        });
        node->t->clientDamage.setRect(AK_IRECT_INF);
    }

    node->t->onBakeGeneratedDamage = false;

    if (node->caps() & AKNode::BackgroundEffect)
    {
        AKBackgroundEffect &backgroundEffect { *static_cast<AKBackgroundEffect*>(node) };

        node->onLayoutUpdate();

        backgroundEffect.m_globalRect = SkIRect::MakeXYWH(
            backgroundEffect.effectRect.x() + backgroundEffect.targetNode()->globalRect().x(),
            backgroundEffect.effectRect.y() + backgroundEffect.targetNode()->globalRect().y(),
            backgroundEffect.effectRect.width(),
            backgroundEffect.effectRect.height());

        backgroundEffect.m_rect = SkIRect::MakeXYWH(
            backgroundEffect.m_globalRect.x() - t->root()->m_globalRect.x(),
            backgroundEffect.m_globalRect.y() - t->root()->m_globalRect.y(),
            backgroundEffect.m_globalRect.width(),
            backgroundEffect.m_globalRect.height());

        node->t->visible = node->visible() && backgroundEffect.targetNode()->t->visible;
    }
    else
    {        
        node->m_globalRect.fLeft = node->parent()->globalRect().x() + SkScalarFloorToInt(node->layout().calculatedLeft());
        node->m_globalRect.fTop = node->parent()->globalRect().y() + SkScalarFloorToInt(node->layout().calculatedTop());
        node->m_globalRect.fRight = node->m_globalRect.fLeft + SkScalarFloorToInt(node->layout().calculatedWidth());
        node->m_globalRect.fBottom = node->m_globalRect.fTop + SkScalarFloorToInt(node->layout().calculatedHeight());

        node->m_rect = SkIRect::MakeXYWH(
            node->m_globalRect.x() - t->root()->m_globalRect.x(),
            node->m_globalRect.y() - t->root()->m_globalRect.y(),
            node->m_globalRect.width(),
            node->m_globalRect.height());

        node->onLayoutUpdate();

        bool parentIsVisible { node->parent() && !node->parent()->parent() && node->parent()->visible() };
        if (node->parent() && node->parent()->parent())
            parentIsVisible = node->parent()->t->visible;
        node->t->visible = node->visible() && parentIsVisible;
    }

    if (!node->reactiveRegion.isEmpty())
    {
        SkRegion reactive;
        node->reactiveRegion.translate(node->m_rect.x(), node->m_rect.y(), &reactive);
        //reactive.op(clip, SkRegion::Op::kIntersect_Op);
        t->m_reactive.push_back(reactive);
        t->m_opaque.op(reactive, SkRegion::Op::kDifference_Op);
    }

    SkRegion clip;

    if (node->t->visible)
        clip.setRect(node->m_rect);

    AKNode *clipper { node->closestClipperParent() };

    if (clipper == t->root())
        clip.op(t->viewport().roundOut(), SkRegion::Op::kIntersect_Op);
    else
        clip.op(clipper->t->prevLocalClip, SkRegion::Op::kIntersect_Op);

    clip.op(t->m_opaque, SkRegion::Op::kDifference_Op);
    node->t->prevLocalClip.op(t->m_opaque, SkRegion::Op::kDifference_Op);
    node->m_insideLastTarget = SkIRect::Intersects(node->m_rect, t->viewport().roundOut());

    node->m_intersectedTargets.clear();
    for (AKTarget *target : targets())
        if (SkIRect::Intersects(node->globalRect(), target->m_globalIViewport))
            node->m_intersectedTargets.push_back(target);

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
                    t->surface()->recordingContext(),
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
            {
                bakeable->t->changes.set(AKRenderable::Chg_Size);
                params.damage->setRect(AK_IRECT_INF);
            }

            SkCanvas &canvas { *params.surface->surface()->getCanvas() };
            canvas.save();
            canvas.scale(params.surface->scale().x(), params.surface->scale().y());
            bakeable->onBake(&params);
            canvas.restore();
            bakeable->t->onBakeGeneratedDamage = !params.damage->isEmpty();
        }
    }

    const bool isRenderable { (node->caps() & AKNode::Render) != 0 };

    if (isRenderable)
        static_cast<AKRenderable*>(node)->handleCommonChanges();
    else
        goto skipDamage;

    if (node->m_rect == node->t->prevLocalRect)
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
        node->t->changes.set(AKRenderable::Chg_Size);
    }

skipDamage:
    node->t->clientDamage.setEmpty();
    node->t->prevLocalClip = clip;
    node->t->prevLocalRect = node->m_rect;
    node->t->prevRect = node->m_globalRect;

    for (auto *backgroundEffect : node->backgroundEffects())
    {
        if (backgroundEffect->stackPosition() == AKBackgroundEffect::Behind)
            backgroundEffect->insertBefore(node);
        else
            backgroundEffect->insertBefore(node->parent()->children().front());
    }

    if (!(node->caps() & AKNode::Scene))
        for (Int64 i = node->children().size() - 1; i >= 0;)
        {
            const int skip = node->children()[i]->backgroundEffects().size();
            calculateNewDamage(node->children()[i]);
            i -= 1 - skip;
        }

    if (!isRenderable)
        return;

    AKRenderable *rend { static_cast<AKRenderable*>(node) };
    rend->m_renderedOnLastTarget = !t->m_opaque.contains(clip) && !clip.isEmpty() && !node->m_globalRect.isEmpty();

    if (!rend->m_renderedOnLastTarget)
        return;

    rend->t->opaqueOverlay = t->m_opaque;

    switch (rend->m_colorHint)
    {
    case AKRenderable::ColorHint::Opaque:
        rend->t->opaque = clip;
        break;
    case AKRenderable::ColorHint::Translucent:
        rend->t->opaque.setEmpty();
        break;
    case AKRenderable::ColorHint::UseRegion:
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
    if (t->age() == 0)
    {
        t->m_damage.setRect(AK_IRECT_INF);
        t->m_damageRing[t->m_damageIndex] = t->m_damage;
    }
    else
    {
        if (t->inDamageRegion)
            t->m_damage.op(*t->inDamageRegion, SkRegion::Op::kUnion_Op);

        for (auto it = t->m_reactive.begin(); it != t->m_reactive.end(); it++)
        {
            if (t->m_damage.intersects(*it))
            {
                t->m_damage.op(*it, SkRegion::Op::kUnion_Op);
                (*it).setEmpty();
            }
        }

        for (auto it = t->m_reactive.rbegin(); it != t->m_reactive.rend(); it++)
        {
            if (t->m_damage.intersects(*it))
            {
                t->m_damage.op(*it, SkRegion::Op::kUnion_Op);
                (*it).setEmpty();
            }
        }

        t->m_damageRing[t->m_damageIndex] = t->m_damage;

        for (UInt32 i = 1; i < t->age(); i++)
        {
            Int32 damageIndex = t->m_damageIndex - i;

            if (damageIndex < 0)
                damageIndex = AK_MAX_BUFFER_AGE + damageIndex;

            t->m_damage.op(t->m_damageRing[damageIndex], SkRegion::Op::kUnion_Op);
        }
    }

    t->m_reactive.clear();
    t->m_damage.op(t->m_prevViewport, SkRegion::Op::kIntersect_Op);
    t->m_opaque.op(t->m_prevViewport, SkRegion::Op::kIntersect_Op);

    if (t->inClipRegion)
        t->m_damage.op(*t->inClipRegion, SkRegion::Op::kIntersect_Op);

    if (t->outDamageRegion)
        *t->outDamageRegion = t->m_damage;

    if (t->outOpaqueRegion)
        *t->outOpaqueRegion = t->m_opaque;

    if (t->m_damageIndex == AK_MAX_BUFFER_AGE - 1)
        t->m_damageIndex = 0;
    else
        t->m_damageIndex++;
}

void AKScene::renderBackground() noexcept
{
    SkRegion background { t->m_damage };
    const SkColor4f clearColor { SkColor4f::FromColor(t->clearColor()) };
    background.op(t->m_opaque, SkRegion::Op::kDifference_Op);
    t->painter()->enableAutoBlendFunc(true);
    t->painter()->setColorFactor(1.f, 1.f, 1.f, 1.f);
    t->painter()->setAlpha(clearColor.fA);
    t->painter()->setColor(clearColor);
    t->painter()->bindColorMode();
    glDisable(GL_BLEND);
    t->painter()->drawRegion(background);
}

void AKScene::renderNodes(AKNode *node)
{
    AKRenderable *rend;

    if (!node->caps() || !node->m_renderedOnLastTarget)
        goto renderChildren;

    rend = static_cast<AKRenderable*>(node);

    if (node->t->translucent.isEmpty())
        goto renderOpaque;

    rend->t->translucent.op(t->m_damage, SkRegion::kIntersect_Op);
    rend->t->translucent.op(rend->t->opaqueOverlay, SkRegion::kDifference_Op);

    if (node->t->translucent.isEmpty())
        goto renderOpaque;

    t->painter()->setParamsFromRenderable(rend);
    glEnable(GL_BLEND);
    rend->onRender(t->painter().get(), node->t->translucent);

    renderOpaque:

    if (node->t->opaque.isEmpty())
        goto renderChildren;

    rend->t->opaque.op(t->m_damage, SkRegion::kIntersect_Op);
    rend->t->opaque.op(rend->t->opaqueOverlay, SkRegion::kDifference_Op);

    if (rend->t->opaque.isEmpty())
        goto renderChildren;

    t->painter()->setParamsFromRenderable(rend);
    glDisable(GL_BLEND);
    rend->onRender(t->painter().get(), node->t->opaque);

    renderChildren:

    if (!(node->caps() & AKNode::Scene))
        for (size_t i = 0; i < node->children().size();)
        {
            renderNodes(node->children()[i]);

            node->children()[i]->t->changes.reset();

            if (node->children()[i]->caps() & AKNode::BackgroundEffect)
                node->children()[i]->setParent(nullptr);
            else
                i++;
        }
}
