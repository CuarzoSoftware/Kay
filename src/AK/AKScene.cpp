#include <AK/AKScene.h>
#include <AK/AKLog.h>
#include <AK/nodes/AKRenderable.h>
#include <AK/nodes/AKBakeable.h>
#include <AK/nodes/AKSubScene.h>
#include <AK/effects/AKBackgroundEffect.h>
#include <AK/AKSurface.h>
#include <AK/AKPainter.h>
#include <AK/AKApplication.h>
#include <AK/AKGLContext.h>
#include <cassert>
#include <algorithm>
#include <yoga/Yoga.h>

#include <include/core/SkCanvas.h>
#include <include/gpu/ganesh/gl/GrGLBackendSurface.h>
#include <include/gpu/ganesh/GrDirectContext.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>

#include <AK/events/AKPointerMoveEvent.h>
#include <AK/events/AKPointerEnterEvent.h>
#include <AK/events/AKPointerLeaveEvent.h>
#include <AK/events/AKPointerButtonEvent.h>
#include <AK/events/AKKeyboardKeyEvent.h>
#include <AK/events/AKWindowStateEvent.h>
#include <AK/events/AKRenderEvent.h>
#include <AK/events/AKBakeEvent.h>

using namespace AK;

AKScene::AKScene() noexcept
{
    theme();
    m_isSubScene = false;
    m_win = std::make_unique<Window>();

    m_win->keyDelayTimer.setCallback([this](AKTimer *) {
        if (akKeyboard().keyRepeatRateMs() > 0 && !akKeyboard().pressedKeyCodes().empty() && m_win->repeatedKey == akKeyboard().pressedKeyCodes().back())
            m_win->keyRepeatTimer.start(0);
    });

    m_win->keyRepeatTimer.setCallback([this](AKTimer *timer) {
        if (akKeyboard().keyRepeatRateMs() > 0 && !akKeyboard().pressedKeyCodes().empty() && m_win->repeatedKey == akKeyboard().pressedKeyCodes().back())
        {
            event(AKKeyboardKeyEvent(m_win->repeatedKey, AKKeyboardKeyEvent::Pressed));
            timer->start(akKeyboard().keyRepeatRateMs());
        }
    });
}

AKSceneTarget *AK::AKScene::createTarget() noexcept
{
    m_targets.emplace_back(new AKSceneTarget(this));
    return m_targets.back();
}

bool AKScene::destroyTarget(AKSceneTarget *target)
{
    if (std::find(m_targets.begin(), m_targets.end(), target) == m_targets.end())
        return false;

    delete target;
    return true;
}

bool AKScene::render(AKSceneTarget *target)
{
    validateTarget(target);

    c = target->surface()->getCanvas();
    c->save();

    updateMatrix();

    const bool isNestedScene = (root()->parent() && isSubScene());

    t->m_globalIViewport = SkRect::MakeXYWH(
                               t->viewport().x() + float(root()->globalRect().x()),
                               t->viewport().y() + float(root()->globalRect().y()),
                               t->viewport().width(), t->viewport().height()).roundOut();

    if (!isNestedScene)
    {
        root()->m_globalRect = SkRect::MakeWH(t->viewport().width(), t->viewport().height()).roundOut();
        root()->m_rect = root()->m_globalRect;
        root()->m_flags.setFlag(AKNode::ChildrenNeedScaleUpdate, t->m_bakedComponentsScale != t->m_prevBakedComponentsScale);
        root()->layout().apply(target->renderCalculatesLayout());
        root()->m_flags.remove(AKNode::ChildrenNeedScaleUpdate);
    }

    for (auto it = root()->children().rbegin(); it != root()->children().rend(); it++)
        notifyBegin(*it);

    for (Int64 i = root()->children().size() - 1; i >= 0;)
    {
        if (root()->children()[i]->m_flags.check(AKNode::Skip))
        {
            i--;
            continue;
        }

        const int skip = root()->children()[i]->backgroundEffects().size();
        calculateNewDamage(root()->children()[i]);
        i -= 1 - skip;
    }

    auto skContext { akApp()->glContext()->skContext() };
    skContext->resetContext();
    skContext->flush();

    m_painter->bindProgram();
    m_painter->bindTarget(t);

    updateDamageRing();
    renderBackground();

    for (size_t i = 0; i < root()->children().size();)
    {
        if (root()->children()[i]->m_flags.check(AKNode::Skip))
        {
            i++;
            continue;
        }

        renderNodes(root()->children()[i]);
        root()->children()[i]->t->changes.reset();

        if (root()->children()[i]->caps() & AKNode::BackgroundEffect)
            root()->children()[i]->setParent(nullptr);
        else
            i++;
    }

    t->m_prevBakedComponentsScale = t->m_bakedComponentsScale;
    t->m_isDirty = false;
    t->m_needsFullRepaint = false;
    t->m_damage.setEmpty();
    t->m_opaque.setEmpty();
    t->m_prevClip.setEmpty();
    t->m_translucent.setEmpty();
    t->m_bdts.clear();
    c->restore();

    if (!isNestedScene)
        akApp()->processAnimations();

    return true;
}

void AKScene::validateTarget(AKSceneTarget *target) noexcept
{
    assert("AKSceneTarget is nullptr" && target);
    assert("AKSceneTarget wasn't created by this scene" && target->m_scene == this);
    assert("AKSceneTarget has no surface" && target->m_surface);
    assert("Invalid surface size" && target->m_surface->width() > 0 && target->m_surface->height() > 0);
    assert("Invalid viewport" && !target->viewport().isEmpty());
    assert("Invalid dstRect" && !target->dstRect().isEmpty());
    assert("Root node is nullptr" && root());
    t = target;

    if (target->age() > AK_MAX_BUFFER_AGE || target->m_needsFullRepaint)
        target->setAge(0);

    auto skTarget = SkSurfaces::GetBackendRenderTarget(t->m_surface.get(), SkSurfaces::BackendHandleAccess::kFlushRead);
    GrGLFramebufferInfo fbInfo;
    GrBackendRenderTargets::GetGLFramebufferInfo(skTarget, &fbInfo);
    t->m_fbId = fbInfo.fFBOID;
    m_painter = akApp()->glContext()->painter();
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

void AKScene::createOrAssignTargetDataForNode(AKNode *node) noexcept
{
    node->t = &node->m_targets[t];

    if (!node->t->target)
    {
        node->t->target = t;
        t->AKObject::on.destroyed.subscribe(node, [node](AKObject *object){
            AKSceneTarget *target { static_cast<AKSceneTarget*>(object) };
            node->m_targets.erase(target);
        });
        node->t->clientDamage.setRect(AK_IRECT_INF);
        node->m_intersectedTargets.insert(t);
    }
    else
    {
        /*node->t->opaque.setEmpty();
        node->t->translucent.setEmpty();
        node->t->opaqueOverlay.setEmpty();*/
    }
}

void AKScene::notifyBegin(AKNode *node)
{
    createOrAssignTargetDataForNode(node);

    const bool visible { SkIRect::Intersects(node->globalRect(), t->m_globalIViewport) };

    if (((!visible && node->childrenClippingEnabled()) || !node->visible()) && node->t->prevLocalClip.isEmpty())
    {
        node->m_flags.add(AKNode::Skip);
        return;
    }

    node->m_flags.remove(AKNode::Skip);

    if (!(node->caps() & AKNode::Scene))
        for (auto it = node->children().rbegin(); it != node->children().rend(); it++)
            notifyBegin(*it);

    if (visible)
        node->onSceneBegin();
}

void AKScene::calculateNewDamage(AKNode *node)
{
    createOrAssignTargetDataForNode(node);
    node->m_flags.remove(AKNode::RenderedOnLastTarget);

    SkRegion clip;
    AKNode *clipper { nullptr };
    AKRenderable *renderable { dynamic_cast<AKRenderable*>(node) };
    AKBakeable *bakeable { dynamic_cast<AKBakeable*>(node) };
    AKBackgroundEffect *backgroundEffect { dynamic_cast<AKBackgroundEffect*>(node) };
    bool hasBDT { false };

    node->m_overlayBdts = t->m_bdts;

    if (bakeable)
        static_cast<AKBakeable*>(node)->m_onBakeGeneratedDamage = false;

    if (backgroundEffect)
    {
        backgroundEffect->onSceneCalculatedRect();

        backgroundEffect->m_globalRect = SkIRect::MakeXYWH(
            backgroundEffect->effectRect.x() + backgroundEffect->targetNode()->globalRect().x(),
            backgroundEffect->effectRect.y() + backgroundEffect->targetNode()->globalRect().y(),
            backgroundEffect->effectRect.width(),
            backgroundEffect->effectRect.height());

        backgroundEffect->m_rect = SkIRect::MakeXYWH(
            backgroundEffect->m_globalRect.x() - root()->m_globalRect.x(),
            backgroundEffect->m_globalRect.y() - root()->m_globalRect.y(),
            backgroundEffect->m_globalRect.width(),
            backgroundEffect->m_globalRect.height());

        node->t->visible = node->visible() && backgroundEffect->targetNode()->t->visible;
    }
    else
    {
        bool parentIsVisible { node->parent() && !node->parent()->parent() && node->parent()->visible() };
        if (node->parent() && node->parent()->parent())
            parentIsVisible = node->parent()->t->visible;
        node->t->visible = node->visible() && parentIsVisible;
    }

    node->bdt.damage.setEmpty();

    if (node->bdt.enabled)
    {
        const SkScalar scale { SkScalar(t->bakedComponentsScale()) * node->bdt.q };
        SkISize size { t->viewport().round().size() };
        const int modW { size.fWidth % node->bdt.divisibleBy };
        const int modH { size.fHeight % node->bdt.divisibleBy };

        if (modW != 0)
            size.fWidth += node->bdt.divisibleBy - modW;
        if (modH != 0)
            size.fHeight += node->bdt.divisibleBy - modH;

        if (node->bdt.surfaces.contains(t))
            node->bdt.surfaces[t]->resize(size, scale, false);
        else
            node->bdt.surfaces[t] = AKSurface::Make(size, scale, false);

        node->bdt.currentSurface = node->bdt.surfaces[t];
        node->bdt.currentSurface->setViewportPos(t->viewport().x(), t->viewport().y());
        hasBDT = true;
        node->bdt.node = node;
        node->bdt.reactiveRectTranslated = node->bdt.reactiveRect.makeOffset(node->m_rect.x(), node->m_rect.y());
        node->bdt.repaintAnyway.translate(node->m_rect.x(), node->m_rect.y(), &node->bdt.repaintAnywayTranslated);
        // reactive.op(clip, SkRegion::Op::kIntersect_Op); // remove?
        t->m_bdts.push_back(&node->bdt);
        t->m_opaque.op(node->bdt.reactiveRectTranslated, SkRegion::Op::kDifference_Op);
    }

    node->m_intersectedTargets.clear();
    for (AKSceneTarget *target : targets())
        if (SkIRect::Intersects(node->globalRect(), target->m_globalIViewport))
            node->m_intersectedTargets.insert(target);

    if (node->t->visible)
        clip.setRect(node->m_rect);

    clipper = node->closestClipperParent();

    if (clipper == root())
        clip.op(t->viewport().roundOut(), SkRegion::Op::kIntersect_Op);
    else
        clip.op(clipper->t->prevLocalClip, SkRegion::Op::kIntersect_Op);

    clip.op(t->m_opaque, SkRegion::Op::kDifference_Op);
    node->t->prevLocalClip.op(t->m_opaque, SkRegion::Op::kDifference_Op);
    node->m_flags.setFlag(AKNode::InsideLastTarget, SkIRect::Intersects(node->m_rect, t->viewport().roundOut()));

    if (bakeable && !clip.isEmpty() &&
        (node->t->changes.any()
            || !node->t->clientDamage.isEmpty()
            || !bakeable->surface()
            || bakeable->surface()->scale() != node->scale()))
    {
        SkRegion clipRegion = clip;
        clipRegion.op(t->m_prevClip, SkRegion::Op::kIntersect_Op);

        if (!clipRegion.isEmpty())
        {
            clipRegion.translate(-bakeable->m_rect.x(), -bakeable->m_rect.y());
            bool surfaceChanged;

            if (bakeable->surface())
            {
                surfaceChanged = bakeable->surface()->scale() != node->scale();
                surfaceChanged |= bakeable->m_surface->resize(
                    bakeable->globalRect().size(),
                    node->scale(), true);
            }
            else
            {
                surfaceChanged = true;
                bakeable->m_surface = AKSurface::Make(
                    bakeable->globalRect().size(),
                    node->scale(), true);
            }

            const AKBakeEvent event (
                node->t->changes,
                *t,
                clipRegion,
                node->t->clientDamage,
                bakeable->opaqueRegion,
                *bakeable->m_surface.get());

            if (surfaceChanged)
            {
                bakeable->t->changes.set(AKRenderable::CHSize);
                event.damage.setRect(AK_IRECT_INF);
            }

            event.canvas().save();
            event.canvas().scale(event.surface.scale(), event.surface.scale());
            akApp()->sendEvent(event, *bakeable);
            event.canvas().restore();
            bakeable->m_onBakeGeneratedDamage = !event.damage.isEmpty();
        }
    }

    /* Must be called after onBake. If called before, any added damage
     * will cause unnecessary rebaking. */
    if (renderable)
        renderable->handleCommonChanges();
    else
        goto skipDamage;

    if (node->m_rect == node->t->prevLocalRect)
    {
        node->t->prevLocalClip.op(clip, SkRegion::Op::kXOR_Op);

        addNodeDamage(*node, node->t->prevLocalClip);
        //t->m_damage.op(node->t->prevLocalClip, SkRegion::Op::kUnion_Op);

        node->t->clientDamage.translate(
            node->m_rect.x(),
            node->m_rect.y());

        node->t->clientDamage.op(clip, SkRegion::Op::kIntersect_Op);
        addNodeDamage(*node, node->t->clientDamage);
        //t->m_damage.op(node->t->clientDamage, SkRegion::Op::kUnion_Op);
    }
    else
    {
        // Both current and prev clip need to be repainted
        addNodeDamage(*node, node->t->prevLocalClip);
        //t->m_damage.op(node->t->prevLocalClip, SkRegion::Op::kUnion_Op);

        addNodeDamage(*node, clip);
        //t->m_damage.op(clip, SkRegion::Op::kUnion_Op);
        node->t->changes.set(AKRenderable::CHSize);
    }

skipDamage:
    node->t->clientDamage.setEmpty();
    node->t->prevLocalClip = clip;
    node->t->prevLocalRect = node->m_rect;

    for (auto *backgroundEffect : node->backgroundEffects())
    {
        if (backgroundEffect->stackPosition() == AKBackgroundEffect::Behind)
            backgroundEffect->insertBefore(node);
        else
            backgroundEffect->insertBefore(node->parent()->children().front());
    }

    auto bdtIt = std::find(t->m_bdts.begin(), t->m_bdts.end(), &node->bdt);

    if (hasBDT)
    {
        if (bdtIt != t->m_bdts.end())
            t->m_bdts.erase(bdtIt);
    }

    if (!(node->caps() & AKNode::Scene))
        for (Int64 i = node->children().size() - 1; i >= 0;)
        {
            if (node->children()[i]->m_flags.check(AKNode::Skip))
            {
                i--;
                continue;
            }

            const int skip = node->children()[i]->backgroundEffects().size();
            calculateNewDamage(node->children()[i]);
            i -= 1 - skip;
        }

    if (hasBDT)
        t->m_bdts.insert(bdtIt, &node->bdt);

    if (!renderable)
        return;

    node->m_flags.setFlag(AKNode::RenderedOnLastTarget, !t->m_opaque.contains(clip) && !clip.isEmpty() && !node->m_globalRect.isEmpty());

    if (!renderable->renderedOnLastTarget())
        return;

    renderable->t->opaqueOverlay = t->m_opaque;

    switch (renderable->m_colorHint)
    {
    case AKRenderable::ColorHint::Opaque:
        renderable->t->opaque = clip;
        break;
    case AKRenderable::ColorHint::Translucent:
        renderable->t->opaque.setEmpty();
        break;
    case AKRenderable::ColorHint::UseRegion:
        renderable->opaqueRegion.translate(node->m_rect.x(), node->m_rect.y(), &renderable->t->opaque);
        renderable->t->opaque.op(clip, SkRegion::kIntersect_Op);
        break;
    }

    t->m_opaque.op(renderable->t->opaque, SkRegion::kUnion_Op);
    renderable->t->translucent = clip;
    renderable->t->translucent.op(renderable->t->opaque, SkRegion::kDifference_Op);
}

void AKScene::updateDamageRing() noexcept
{
    updateDamageTrackers();

    if (t->age() == 0)
    {
        t->m_damage.setRect(AK_IRECT_INF);
        t->m_damageRing[t->m_damageIndex] = t->m_damage;
    }
    else
    {
        if (t->inDamageRegion)
            t->m_damage.op(*t->inDamageRegion, SkRegion::Op::kUnion_Op);

        t->m_damageRing[t->m_damageIndex] = t->m_damage;

        for (UInt32 i = 1; i < t->age(); i++)
        {
            Int32 damageIndex = t->m_damageIndex - i;

            if (damageIndex < 0)
                damageIndex = AK_MAX_BUFFER_AGE + damageIndex;

            t->m_damage.op(t->m_damageRing[damageIndex], SkRegion::Op::kUnion_Op);
        }
    }

    t->m_bdts.clear();
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

void AKScene::updateDamageTrackers() noexcept
{
    for (auto *bdt : t->m_bdts)
    {
        if (bdt->damage.isEmpty())
            continue;

        bdt->damage.op(bdt->node->sceneRect(), SkRegion::Op::kIntersect_Op);
        bdt->damage.op(t->m_prevViewport, SkRegion::Op::kIntersect_Op);

        if (bdt->r != 0)
        {
            SkRegion outset;
            SkRegion::Iterator it (bdt->damage);

            while (!it.done())
            {
                outset.op(it.rect().makeOutset(bdt->r, bdt->r), SkRegion::Op::kUnion_Op);
                it.next();
            }

            bdt->damage = outset;
        }

        bdt->damage.op(bdt->node->sceneRect(), SkRegion::Op::kIntersect_Op);
        bdt->damage.op(t->m_prevViewport, SkRegion::Op::kIntersect_Op);
        t->m_damage.op(bdt->damage, SkRegion::Op::kUnion_Op);
    }
}

void AKScene::renderBackground() noexcept
{
    SkRegion background { t->m_damage };
    background.op(t->m_opaque, SkRegion::Op::kDifference_Op);
    SkRegion aux, anyway;

    for (auto bdt = t->m_bdts.rbegin(); bdt != t->m_bdts.rend(); bdt++)
    {
        if (!aux.op((*bdt)->reactiveRectTranslated, background, SkRegion::kIntersect_Op))
            continue;

        anyway.op((*bdt)->repaintAnywayTranslated, background, SkRegion::kIntersect_Op);
        renderBackgroundOnTarget(*(*bdt)->surfaces[t].get(), aux);
        background.op(aux, SkRegion::kDifference_Op);
        background.op(anyway, SkRegion::kUnion_Op);
    }

    renderBackgroundOnTarget(*t, background);
}

void AKScene::renderBackgroundOnTarget(AKTarget &target, SkRegion &region) noexcept
{
    const SkColor4f clearColor { SkColor4f::FromColor(t->clearColor()) };
    m_painter->bindTarget(&target);
    m_painter->enableAutoBlendFunc(true);
    m_painter->setColorFactor(1.f, 1.f, 1.f, 1.f);
    m_painter->setAlpha(clearColor.fA);
    m_painter->setColor(clearColor);
    m_painter->bindColorMode();
    glDisable(GL_BLEND);
    m_painter->drawRegion(region);
}

void AKScene::renderNodes(AKNode *node)
{
    AKRenderable *rend;
    SkRegion aux, anyway;

    if (node->m_flags.check(AKNode::Skip))
        return;

    if (!node->caps() || !node->renderedOnLastTarget())
        goto renderChildren;

    rend = static_cast<AKRenderable*>(node);

    if (node->t->translucent.isEmpty())
        goto renderOpaque;

    rend->t->translucent.op(t->m_damage, SkRegion::kIntersect_Op);
    rend->t->translucent.op(rend->t->opaqueOverlay, SkRegion::kDifference_Op);

    if (node->t->translucent.isEmpty())
        goto renderOpaque;

    for (auto bdt = rend->m_overlayBdts.rbegin(); bdt != rend->m_overlayBdts.rend(); bdt++)
    {
        if (!aux.op((*bdt)->reactiveRectTranslated, node->t->translucent, SkRegion::kIntersect_Op))
            continue;

        anyway.op((*bdt)->repaintAnywayTranslated, node->t->translucent, SkRegion::kIntersect_Op);
        renderNodeTranslucentOnTarget(rend, *(*bdt)->surfaces[t].get(), aux);
        node->t->translucent.op(aux, SkRegion::kDifference_Op);
        node->t->translucent.op(anyway, SkRegion::kUnion_Op);
    }

    renderNodeTranslucentOnTarget(rend, *t, node->t->translucent);
    renderOpaque:

    if (node->t->opaque.isEmpty())
        goto renderChildren;

    rend->t->opaque.op(t->m_damage, SkRegion::kIntersect_Op);
    rend->t->opaque.op(rend->t->opaqueOverlay, SkRegion::kDifference_Op);

    if (rend->t->opaque.isEmpty())
        goto renderChildren;

    for (auto bdt = rend->m_overlayBdts.rbegin(); bdt != rend->m_overlayBdts.rend(); bdt++)
    {
        if (!aux.op((*bdt)->reactiveRectTranslated, node->t->opaque, SkRegion::kIntersect_Op))
            continue;

        anyway.op((*bdt)->repaintAnywayTranslated, node->t->opaque, SkRegion::kIntersect_Op);
        renderNodeOpaqueOnTarget(rend, *(*bdt)->surfaces[t].get(), aux);
        node->t->opaque.op(aux, SkRegion::kDifference_Op);
        node->t->opaque.op(anyway, SkRegion::kUnion_Op);
    }

    renderNodeOpaqueOnTarget(rend, *t, node->t->opaque);

    renderChildren:

    if (!(node->caps() & AKNode::Scene))
        for (size_t i = 0; i < node->children().size();)
        {
            if (node->children()[i]->m_flags.check(AKNode::Skip))
            {
                i++;
                continue;
            }

            renderNodes(node->children()[i]);
            node->children()[i]->t->changes.reset();
            YGNodeSetHasNewLayout(node->m_layout.m_node, false);

            if (node->children()[i]->caps() & AKNode::BackgroundEffect)
                node->children()[i]->setParent(nullptr);
            else
                i++;
        }
}

    void AKScene::renderNodeTranslucentOnTarget(AKRenderable *node, AKTarget &target, SkRegion &region) noexcept
    {
        m_painter->bindTarget(&target);
        m_painter->setParamsFromRenderable(node);
        glEnable(GL_BLEND);
        akApp()->sendEvent(AKRenderEvent(target, region, node->sceneRect(), *m_painter.get()), *node);
    }

    void AKScene::renderNodeOpaqueOnTarget(AKRenderable *node, AKTarget &target, SkRegion &region) noexcept
    {
        m_painter->bindTarget(&target);
        m_painter->setParamsFromRenderable(node);
        glDisable(GL_BLEND);
        akApp()->sendEvent(AKRenderEvent(target, region, node->sceneRect(), *m_painter.get()), *node);
    }

    void AKScene::setRoot(AKNode *node) noexcept
    {
        if (node == m_root)
            return;

        if (m_root)
        {
            m_root->m_flags.remove(AKNode::IsRoot);
            if (!m_isSubScene)
                m_root->setScene(nullptr);
        }

        m_root.reset(node);

        if (m_root && !m_isSubScene)
        {
            m_root->m_flags.add(AKNode::IsRoot);
            m_root->setScene(this);
        }

        for (AKSceneTarget *t : m_targets)
        {
            t->m_needsFullRepaint = true;
            t->markDirty();
        }
    }

    AKNode *AKScene::nodeAt(const SkPoint &pos) const noexcept
    {
        if (!m_root)
            return nullptr;

        const SkIPoint ipos(pos.x(), pos.y());
        AKNode::RIterator it { m_root->bottommostRightChild() };
        AKNode *clipper, *topmostInvisibleParent;

        while (!it.done())
        {
            topmostInvisibleParent = it.node()->topmostInvisibleParent();

            if (topmostInvisibleParent)
            {
                it.jumpTo(topmostInvisibleParent);
                continue;
            }

            if (!it.node()->visible())
            {
                it.next();
                continue;
            }

            if (!it.node()->globalRect().contains(ipos.x(), ipos.y()))
            {
                it.next();
                continue;
            }

            if (it.node()->inputRegion() && !it.node()->inputRegion()->contains(
                    ipos.x() - it.node()->globalRect().x(),
                    ipos.y() - it.node()->globalRect().y()))
            {
                it.next();
                continue;
            }

            clipper = it.node()->closestClipperParent();

            if (clipper)
            {
                if (!clipper->globalRect().contains(ipos.x(), ipos.y()))
                {
                    it.next();
                    continue;
                }

                if (clipper->inputRegion() && !clipper->inputRegion()->contains(
                        ipos.x() - clipper->globalRect().x(),
                        ipos.y() - clipper->globalRect().y()))
                {
                    it.next();
                    continue;
                }
            }

            return it.node();
        }

        return nullptr;
    }

    static AKNode *searchKeyboardFocusable(AKNode *node) noexcept
    {
        if (!node)
            return nullptr;

        AKNode *found;
        for (AKNode *child : node->children())
        {
            if (child->isKeyboardFocusable())
                return child;

            found = searchKeyboardFocusable(child);

            if (found)
                return found;
        }

        return nullptr;
    }

    AKNode *AKScene::nextKeyboardFocusable() const noexcept
    {
        if (!keyboardFocus())
            return nullptr;

        AKNode *found;
        AKNode::Iterator it { keyboardFocus() };
        it.next();

        while (!it.done())
        {
            if (it.node()->isKeyboardFocusable())
                return it.node();
            else if (it.node()->parent() == keyboardFocus()->parent())
            {
                found = searchKeyboardFocusable(it.node());

                if (found)
                    return found;
            }

            it.next();
        }

        it.reset(root()->bottommostLeftChild());

        while (!it.done())
        {
            if (it.node()->isKeyboardFocusable())
                return it.node();

            it.next();
        }

        return nullptr;
    }

    bool AKScene::event(const AKEvent &event)
    {
        if (!m_root || m_isSubScene)
            return AKObject::event(event);

        m_win->e = &event;

        switch (event.type()) {
        case AKEvent::PointerEnter:
        {
            const AKPointerEnterEvent &enterEvent { static_cast<const AKPointerEnterEvent &>(event) };
            const AKPointerMoveEvent moveEvent { enterEvent.pos(), { 0.f, 0.f }, { 0.f, 0.f }, enterEvent.serial(), enterEvent.ms(), enterEvent.us(), enterEvent.device() };
            m_win->e = &moveEvent;
            handlePointerMoveEvent();
            break;
        }
        case AKEvent::PointerMove:
            handlePointerMoveEvent();
            break;
        case AKEvent::PointerLeave:
            handlePointerLeaveEvent();
            break;
        case AKEvent::PointerButton:
            handlePointerButtonEvent();
            break;
        case AKEvent::KeyboardKey:
            handleKeyboardKeyEvent();
            break;
        case AKEvent::WindowState:
            handleWindowStateEvent();
            break;
        default:
            return AKObject::event(event);
            break;
        }

        return true;
    }

    void AKScene::addNodeDamage(AKNode &, const SkRegion &damage) noexcept
    {
        t->m_damage.op(damage, SkRegion::Op::kUnion_Op);

        for (auto &bdt : t->m_bdts)
            if (SkIRect::Intersects(bdt->reactiveRectTranslated, damage.getBounds()))
                bdt->damage.op(damage, SkRegion::Op::kUnion_Op);
    }

    void AKScene::handlePointerMoveEvent()
    {
        auto &event { *static_cast<const AKPointerMoveEvent*>(m_win->e) };
        const AKPointerEnterEvent enterEvent(event.pos(), event.serial(), event.ms(), event.us(), event.device());
        const AKPointerLeaveEvent leaveEvent(event.pos(), event.serial(), event.ms(), event.us(), event.device());
        akApp()->pointer().m_pos = event.pos();
        akApp()->pointer().m_windowFocus.reset(this);
        m_root->removeFlagsAndPropagate(AKNode::Notified | AKNode::ChildHasPointerFocus);
        m_win->pointerFocus.reset(nodeAt(event.pos()));

        if (m_win->pointerFocus)
            m_win->pointerFocus->setFlagsAndPropagateToParents(AKNode::ChildHasPointerFocus, true);

        AKNode::RIterator it { nullptr };

    retry:
        it.reset(m_root->bottommostRightChild());
        m_treeChanged = false;

        while (!it.done())
        {
            if (it.node()->m_flags.check(AKNode::Notified))
            {
                it.next();
                continue;
            }

            it.node()->m_flags.add(AKNode::Notified);

            if (it.node()->pointerGrabEnabled())
            {
                akApp()->sendEvent(*m_win->e, *it.node());
            }
            else
            {
                if (it.node()->m_flags.check(AKNode::ChildHasPointerFocus))
                {
                    if (it.node()->m_flags.check(AKNode::HasPointerFocus))
                        akApp()->sendEvent(*m_win->e, *it.node());
                    else
                    {
                        it.node()->m_flags.add(AKNode::HasPointerFocus);
                        akApp()->sendEvent(enterEvent, *it.node());
                    }
                }
                else if (it.node()->m_flags.check(AKNode::HasPointerFocus))
                {
                    it.node()->m_flags.remove(AKNode::HasPointerFocus);
                    akApp()->sendEvent(leaveEvent, *it.node());
                }
            }

            if (m_treeChanged)
                goto retry;

            it.next();
        }
    }

    void AKScene::handlePointerLeaveEvent()
    {
        auto &event { *static_cast<const AKPointerLeaveEvent*>(m_win->e) };
        akApp()->pointer().m_pos = event.pos();

        if (akApp()->pointer().m_windowFocus == this)
            akApp()->pointer().m_windowFocus.reset();

        m_root->removeFlagsAndPropagate(AKNode::Notified);

        AKNode::RIterator it { nullptr };

    retry:
        it.reset(m_root->bottommostRightChild());
        m_treeChanged = false;

        while (!it.done())
        {
            if (it.node()->m_flags.check(AKNode::Notified))
            {
                it.next();
                continue;
            }

            it.node()->m_flags.add(AKNode::Notified);

            if (it.node()->m_flags.check(AKNode::HasPointerFocus))
            {
                it.node()->m_flags.remove(AKNode::HasPointerFocus);
                akApp()->sendEvent(event, *it.node());
            }

            if (m_treeChanged)
                goto retry;

            it.next();
        }
    }

    void AKScene::handlePointerButtonEvent()
    {
        m_root->removeFlagsAndPropagate(AKNode::Notified);
        AKNode::RIterator it { nullptr };

    retry:
        it.reset(m_root->bottommostRightChild());
        m_treeChanged = false;

        while (!it.done())
        {
            if (it.node()->m_flags.check(AKNode::Notified))
            {
                it.next();
                continue;
            }

            it.node()->m_flags.add(AKNode::Notified);

            if (it.node()->m_flags.check(AKNode::HasPointerFocus | AKNode::PointerGrab))
                akApp()->sendEvent(*m_win->e, *it.node());

            if (m_treeChanged)
                goto retry;

            it.next();
        }
    }

    void AKScene::handleKeyboardKeyEvent()
    {
        if (akKeyboard().keyRepeatRateMs() == 0 || akKeyboard().pressedKeyCodes().empty())
        {
            m_win->keyDelayTimer.stop(false);
            m_win->keyRepeatTimer.stop(false);
            m_win->repeatedKey = -1;
        }
        else if (akKeyboard().pressedKeyCodes().back() != m_win->repeatedKey)
        {
            m_win->keyRepeatTimer.stop(false);
            m_win->repeatedKey = akKeyboard().pressedKeyCodes().back();
            m_win->keyDelayTimer.start(akKeyboard().keyRepeatDelayMs());
        }

        if (keyboardFocus())
            akApp()->sendEvent(*m_win->e, *keyboardFocus());

    }

    void AKScene::handleWindowStateEvent()
    {
        const AKWindowStateEvent &event { static_cast<const AKWindowStateEvent &>(*m_win->e) };
        m_win->windowState = event.states();
        m_root->removeFlagsAndPropagate(AKNode::Notified);
        AKNode::RIterator it { nullptr };

    retry:
        it.reset(m_root->bottommostRightChild());
        m_treeChanged = false;

        while (!it.done())
        {
            if (it.node()->m_flags.check(AKNode::Notified))
            {
                it.next();
                continue;
            }

            it.node()->m_flags.add(AKNode::Notified);
            akApp()->sendEvent(*m_win->e, *it.node());

            if (m_treeChanged)
                goto retry;

            it.next();
        }
    }
