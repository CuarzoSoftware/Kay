#include "include/gpu/ganesh/SkSurfaceGanesh.h"
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
#include <yoga/Yoga.h>
#include <include/core/SkCanvas.h>
#include <include/gpu/GrDirectContext.h>

#include <AK/events/AKPointerMoveEvent.h>
#include <AK/events/AKPointerEnterEvent.h>
#include <AK/events/AKPointerLeaveEvent.h>
#include <AK/events/AKPointerButtonEvent.h>

using namespace AK;

AKScene::AKScene() noexcept
{
    theme();
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

    auto skContext { AKApp()->glContext()->skContext() };
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
    t->m_reactive.clear();

    c->restore();

    if (!isNestedScene)
        for (AKNode *node : AKApp()->animated)
                node->repaint();

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
    assert("Root node is nullptr" && root());
    t = target;

    if (target->age() > AK_MAX_BUFFER_AGE || target->m_needsFullRepaint)
        target->setAge(0);

    auto skTarget = SkSurfaces::GetBackendRenderTarget(t->m_surface.get(), SkSurfaces::BackendHandleAccess::kFlushRead);
    GrGLFramebufferInfo fbInfo;
    skTarget.getGLFramebufferInfo(&fbInfo);
    t->m_fbId = fbInfo.fFBOID;
    m_painter = AKApp()->glContext()->painter();
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
            AKTarget *target { static_cast<AKTarget*>(object) };
            node->m_targets.erase(target);
        });
        node->t->clientDamage.setRect(AK_IRECT_INF);
        node->m_intersectedTargets.push_back(t);
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

    if (bakeable)
        static_cast<AKBakeable*>(node)->m_onBakeGeneratedDamage = false;

    if (renderable)
        renderable->handleCommonChanges();

    const bool hasNoChanges = t->m_damage.isEmpty() &&
                              node->t->changes.none() &&
                              node->t->clientDamage.isEmpty();

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
        if (node->t->changes.test(AKNode::Chg_LayoutPos) || node->t->changes.test(AKNode::Chg_LayoutSize))
        {
            node->m_rect = SkIRect::MakeXYWH(
                node->m_globalRect.x() - root()->m_globalRect.x(),
                node->m_globalRect.y() - root()->m_globalRect.y(),
                node->m_globalRect.width(),
                node->m_globalRect.height());
        }

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

    if (!hasNoChanges)
    {
        node->m_intersectedTargets.clear();
        for (AKTarget *target : targets())
            if (SkIRect::Intersects(node->globalRect(), target->m_globalIViewport))
                node->m_intersectedTargets.push_back(target);
    }

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
                    SkSize::Make(bakeable->globalRect().size()),
                    node->scale(), true);
            }
            else
            {
                surfaceChanged = true;
                bakeable->m_surface = AKSurface::Make(
                    SkSize::Make(bakeable->globalRect().size()),
                    node->scale(), true);
            }

            AKBakeable::OnBakeParams params
            {
                .clip = &clipRegion,
                .damage = &bakeable->t->clientDamage,
                .opaque = &bakeable->opaqueRegion,
                .surface = bakeable->m_surface
            };

            if (surfaceChanged)
            {
                bakeable->t->changes.set(AKRenderable::Chg_Size);
                params.damage->setRect(AK_IRECT_INF);
            }

            SkCanvas &canvas { *params.surface->surface()->getCanvas() };
            canvas.save();
            canvas.scale(params.surface->scale(), params.surface->scale());
            bakeable->onBake(&params);
            canvas.restore();
            bakeable->m_onBakeGeneratedDamage = !params.damage->isEmpty();

            //params.surface->surface()->recordingContext()->asDirectContext()->resetContext();
            //params.surface->surface()->flush();
        }
    }


    if (!renderable)
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
            if (node->children()[i]->m_flags.check(AKNode::Skip))
            {
                i--;
                continue;
            }

            const int skip = node->children()[i]->backgroundEffects().size();
            calculateNewDamage(node->children()[i]);
            i -= 1 - skip;
        }

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
    m_painter->enableAutoBlendFunc(true);
    m_painter->setColorFactor(1.f, 1.f, 1.f, 1.f);
    m_painter->setAlpha(clearColor.fA);
    m_painter->setColor(clearColor);
    m_painter->bindColorMode();
    glDisable(GL_BLEND);
    m_painter->drawRegion(background);
}

void AKScene::renderNodes(AKNode *node)
{
    AKRenderable *rend;

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

    m_painter->setParamsFromRenderable(rend);
    glEnable(GL_BLEND);
    rend->onRender(m_painter.get(), node->t->translucent, rend->sceneRect());

    renderOpaque:

    if (node->t->opaque.isEmpty())
        goto renderChildren;

    rend->t->opaque.op(t->m_damage, SkRegion::kIntersect_Op);
    rend->t->opaque.op(rend->t->opaqueOverlay, SkRegion::kDifference_Op);

    if (rend->t->opaque.isEmpty())
        goto renderChildren;

    m_painter->setParamsFromRenderable(rend);
    glDisable(GL_BLEND);
    rend->onRender(m_painter.get(), node->t->opaque, rend->sceneRect());

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

        for (AKTarget *t : m_targets)
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
        AKNode::RIterator it { m_root->bottommostChild() };
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

    void AKScene::postEvent(const AKEvent &event)
    {
        if (!m_root) return;
        e = &event;

        switch (event.type()) {
        case AKEvent::Type::Pointer:
            switch (event.subtype()) {
            case AKEvent::Subtype::Move:
                handlePointerMoveEvent();
                break;
            case AKEvent::Subtype::Button:
                handlePointerButtonEvent();
                break;
            default:
                break;
            }
            break;
        case AKEvent::Type::Keyboard:
            switch (event.subtype()) {
            case AKEvent::Subtype::Key:
                handleKeyboardKeyEvent();
                break;
            default:
                break;
            }
            break;
        case AKEvent::Type::State:
            switch (event.subtype()) {
            case AKEvent::Subtype::Activated:
                handleStateActivatedEvent();
                break;
            case AKEvent::Subtype::Deactivated:
                handleStateDeactivatedEvent();
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }

    void AKScene::handlePointerMoveEvent()
    {
        auto &event { *static_cast<const AKPointerMoveEvent*>(e) };
        const AKPointerEnterEvent enterEvent(event.pos(), event.serial(), event.ms(), event.us(), event.device());
        const AKPointerLeaveEvent leaveEvent(event.pos(), event.serial(), event.ms(), event.us(), event.device());
        m_root->removeFlagsAndPropagate(AKNode::Notified | AKNode::ChildHasPointerFocus);
        m_pointerFocus.reset(nodeAt(event.pos()));

        if (m_pointerFocus)
            m_pointerFocus->setFlagsAndPropagateToParents(AKNode::ChildHasPointerFocus, true);

        AKNode::RIterator it { nullptr };

    retry:
        it.reset(m_root->bottommostChild());
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
                it.node()->onEvent(*e);
            }
            else
            {
                if (it.node()->m_flags.check(AKNode::ChildHasPointerFocus))
                {
                    if (it.node()->m_flags.check(AKNode::HasPointerFocus))
                        it.node()->onEvent(*e);
                    else
                    {
                        it.node()->m_flags.add(AKNode::HasPointerFocus);
                        it.node()->onEvent(enterEvent);
                    }
                }
                else if (it.node()->m_flags.check(AKNode::HasPointerFocus))
                {
                    it.node()->m_flags.remove(AKNode::HasPointerFocus);
                    it.node()->onEvent(leaveEvent);
                }
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
        it.reset(m_root->bottommostChild());
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
                it.node()->onEvent(*e);

            if (m_treeChanged)
                goto retry;

            it.next();
        }
    }

    void AKScene::handleKeyboardKeyEvent()
    {
        m_root->removeFlagsAndPropagate(AKNode::Notified);
        AKNode::RIterator it { nullptr };

    retry:
        it.reset(m_root->bottommostChild());
        m_treeChanged = false;

        while (!it.done())
        {
            if (it.node()->m_flags.check(AKNode::Notified))
            {
                it.next();
                continue;
            }

            it.node()->m_flags.add(AKNode::Notified);
            it.node()->onEvent(*e);

            if (m_treeChanged)
                goto retry;

            it.next();
        }
    }

    void AKScene::handleStateActivatedEvent()
    {
        if (m_activated)
            return;

        m_activated = true;
        m_root->removeFlagsAndPropagate(AKNode::Notified);
        AKNode::RIterator it { nullptr };

    retry:
        it.reset(m_root->bottommostChild());
        m_treeChanged = false;

        while (!it.done())
        {
            if (it.node()->m_flags.check(AKNode::Notified))
            {
                it.next();
                continue;
            }

            it.node()->m_flags.add(AKNode::Notified);
            it.node()->onEvent(*e);

            if (m_treeChanged)
                goto retry;

            it.next();
        }
    }

    void AKScene::handleStateDeactivatedEvent()
    {
        if (!m_activated)
            return;

        m_activated = false;
        m_root->removeFlagsAndPropagate(AKNode::Notified);
        AKNode::RIterator it { nullptr };

    retry:
        it.reset(m_root->bottommostChild());
        m_treeChanged = false;

        while (!it.done())
        {
            if (it.node()->m_flags.check(AKNode::Notified))
            {
                it.next();
                continue;
            }

            it.node()->m_flags.add(AKNode::Notified);
            it.node()->onEvent(*e);

            if (m_treeChanged)
                goto retry;

            it.next();
        }
    }
