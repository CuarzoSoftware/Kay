#include <CZ/AK/AKScene.h>
#include <CZ/AK/AKLog.h>
#include <CZ/AK/Nodes/AKRenderable.h>
#include <CZ/AK/Nodes/AKBakeable.h>
#include <CZ/AK/Nodes/AKSubScene.h>
#include <CZ/AK/Effects/AKBackgroundEffect.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKApp.h>

#include <CZ/Core/Events/CZPointerMoveEvent.h>
#include <CZ/Core/Events/CZPointerEnterEvent.h>
#include <CZ/Core/Events/CZPointerLeaveEvent.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZKeyboardKeyEvent.h>
#include <CZ/Core/Events/CZWindowStateEvent.h>
#include <CZ/Core/Events/CZKeyboardLeaveEvent.h>

#include <CZ/AK/Events/AKRenderEvent.h>
#include <CZ/AK/Events/AKBakeEvent.h>

#include <CZ/Core/CZCore.h>
#include <CZ/Core/CZKeymap.h>

#include <CZ/Ream/RSurface.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Ream/RPass.h>
#include <CZ/Ream/RCore.h>

#include <cassert>
#include <algorithm>
#include <yoga/Yoga.h>

#include <CZ/skia/core/SkCanvas.h>
#include <CZ/skia/gpu/ganesh/gl/GrGLBackendSurface.h>
#include <CZ/skia/gpu/ganesh/GrDirectContext.h>
#include <CZ/skia/gpu/ganesh/SkSurfaceGanesh.h>

using namespace CZ;

static void EnqueueAndPropagateToParent(const CZEvent &event, AKNode *from, AKNode *to, bool includeTo) noexcept
{
    auto core { CZCore::Get() };

    if (!core) return;

    auto e { event.copy() };

    while (from)
    {
        core->postEvent(e, *from);

        if (includeTo && from == to)
            break;

        from = from->parent();

        if (!includeTo && from == to)
            break;
    }
}

static void EnqueueAndPropagateToChildren(std::shared_ptr<CZCore> core, std::shared_ptr<CZEvent> event, AKNode *root) noexcept
{
    core->postEvent(event, *root);

    for (auto *child : root->children(true))
        EnqueueAndPropagateToChildren(core, event, child);
}

AKScene::AKScene(bool isSubScene) noexcept : m_isSubScene(isSubScene)
{
    if (isSubScene)
        return;

    theme();
    m_win = std::make_unique<Window>();

    m_win->keyDelayTimer.setCallback([this](CZTimer *) {

        auto app { AKApp::Get() };
        auto core { app->core() };

        if (!core || !core->keymap() || core->keymap()->repeatRateMs() <= 0 || !app->keyboard().history().key.isPressed || app->keyboard().history().key.code != m_win->repeatedKey)
            return;

        m_win->keyRepeatTimer.start(0);
    });

    m_win->keyRepeatTimer.setCallback([this](CZTimer *timer) {

        auto app { AKApp::Get() };
        auto core { app->core() };

        if (!core || !core->keymap() || core->keymap()->repeatRateMs() <= 0 || !app->keyboard().history().key.isPressed || app->keyboard().history().key.code != m_win->repeatedKey)
            return;

        CZKeyboardKeyEvent e { app->keyboard().history().key };
        e.isRepeat = true;

        core->sendEvent(e, *this);
        timer->start(core->keymap()->repeatRateMs());
    });
}

std::shared_ptr<AKScene> AKScene::Make() noexcept
{
    auto scene { std::shared_ptr<AKScene>(new AKScene(false)) };
    scene->m_self = scene;
    return scene;
}

AKScene::~AKScene()
{
    auto app { AKApp::Get() };

    CZSafeEventQueue queue;

    if (app->pointer().focus() == this)
        queue.addEvent(std::make_shared<CZPointerLeaveEvent>(), *app);

    if (app->keyboard().focus() == this)
        queue.addEvent(std::make_shared<CZKeyboardLeaveEvent>(), *app);

    queue.dispatch();
    notifyDestruction();
}

std::shared_ptr<AKScene> AKScene::MakeSubScene() noexcept
{
    auto scene { std::shared_ptr<AKScene>(new AKScene(true)) };
    scene->m_self = scene;
    return scene;
}

std::shared_ptr<AKTarget> AKScene::makeTarget() noexcept
{
    return std::shared_ptr<AKTarget>(new AKTarget(m_self.lock()));
}

bool AKScene::validateTarget(std::shared_ptr<AKTarget> target) noexcept
{
    if (!target)
    {
        AKLog(CZError, CZLN, "Invalid AKTarget (nullptr)");
        return false;
    }

    if (this != target->scene().get())
    {
        AKLog(CZError, CZLN, "The AKTarget belongs to another scene");
        return false;
    }

    if (!target->surface)
    {
        AKLog(CZError, CZLN, "The AKTarget has an invalid RSurface");
        return false;
    }

    pass = target->surface->beginPass();

    if (!pass)
    {
        AKLog(CZError, CZLN, "Failed to create RPass");
        return false;
    }

    if (target->age > AK_MAX_BUFFER_AGE || target->m_needsFullRepaint)
        target->age = 0;

    return true;
}

void AKScene::layoutTree() noexcept
{
    // Round and store the RScene viewport just in case the user unsets it layer
    ct->m_worldViewport = ct->surface->geometry().viewport.round();
    ct->m_sceneViewport = ct->m_worldViewport.makeOffset(-ct->m_worldViewport.topLeft());

    // Setup clip
    if (ct->inClip)
    {
        ct->m_clip = *ct->inClip;
        ct->m_clip.op(ct->m_sceneViewport, SkRegion::kIntersect_Op);
    }
    else
        ct->m_clip.setRect(ct->m_sceneViewport);

    const bool isNestedScene { root()->parent() != nullptr && isSubScene() };

    if (!isNestedScene)
    {
        const bool scaleChanged { ct->m_bakedNodesScale != ct->m_prevBakedNodesScale };
        root()->m_flags.setFlag(AKNode::ChildrenNeedScaleUpdate, scaleChanged);
        root()->layout().apply(ct->layoutOnRender, true);
        root()->m_flags.remove(AKNode::ChildrenNeedScaleUpdate);
    }
}

void AKScene::setupInvisibleRegion() noexcept
{
    if (!ct->outInvisible) // The user didn't requested it
        return;

    // Make it empty if the clear color is not 100% translucent
    if (SkColorGetA(ct->m_clearColor) == 0)
        ct->outInvisible->setRect(ct->m_sceneViewport);
    else
        ct->outInvisible->setEmpty();
}

void AKScene::treeNotifyBegin() noexcept
{
    for (auto it = root()->children(true).rbegin(); it != root()->children(true).rend(); it++)
        notifyBegin(*it);
}

void AKScene::notifyBegin(AKNode *node)
{
    createOrAssignTargetDataForNode(node);

    const bool visible { SkIRect::Intersects(node->worldRect(), ct->m_worldViewport) };

    /* We can skip rendering an entire subtree as long as the parent clips its children.
     * If prevSceneClip is not empty it means part of the node was visible in the previous frame, in that case,
     * let the scene handle it one more time to clear that region */
    if (((!visible && node->childrenClippingEnabled()) || !node->visible()) && node->tData->prevSceneClip.isEmpty())
    {
        node->m_flags.add(AKNode::Skip);
        return;
    }

    node->m_flags.remove(AKNode::Skip);

    // Children of an AKSubScene are notified later by the AKSubScene itself
    if (!node->asSubScene())
        for (auto it = node->children(true).rbegin(); it != node->children(true).rend(); it++)
            notifyBegin(*it);

    if (visible)
        node->onSceneBegin();
}

void AKScene::calculateTreeDamage() noexcept
{
    for (Int64 i = root()->children(true).size() - 1; i >= 0;)
    {
        if (root()->children(true)[i]->m_flags.has(AKNode::Skip))
        {
            i--;
            continue;
        }

        /* We need to skip the background effects of the node. These are added  */
        const auto skip { root()->children(true)[i]->backgroundEffects().size() };
        calculateNewDamage(root()->children(true)[i]);
        i -= 1 - skip;
    }
}

bool AKScene::render(std::shared_ptr<AKTarget> target) noexcept
{
    if (!root())
    {
        AKLog(CZError, CZLN, "The AKScene has no root() node");
        return false;
    }

    if (!validateTarget(target))
    {
        AKLog(CZError, CZLN, "Invalid AKTarget");
        return false;
    }

    // Temporarily store the target to prevent passing it around to every function (unset at the end)
    ct = target;

    // AKScene calculates regions and draws relative to the viewport origin
    auto geometry { pass->geometry() };
    geometry.viewport.offsetTo(0, 0);
    pass->setGeometry(geometry);

    layoutTree();
    setupInvisibleRegion();
    treeNotifyBegin();
    calculateTreeDamage();
    updateDamageRing();
    renderBackground();
    renderTree();
    resetTarget();
    pass.reset();
    return true;
}

void AKScene::createOrAssignTargetDataForNode(AKNode *node) noexcept
{
    // Creates or gets the node state on this target
    node->tData = &node->m_targets[ct.get()];

    if (!node->tData->target) // Presented on this target for the first time
    {
        node->tData->target = ct.get();
        node->tData->damage.setRect(AK_IRECT_INF);
        node->m_intersectedTargets.insert(ct.get());

        // Clear the data when the target is destroyed
        ct->CZObject::onDestroy.subscribe(node, [node](CZObject *object) {
            auto *target { static_cast<AKTarget*>(object) };
            node->m_targets.erase(target);
            node->m_intersectedTargets.erase(target);
            node->bdt.m_surfaces.erase(target);
        });
    }
    else
    {
        node->tData->invisble.setEmpty();
    }
}

void AKScene::calculateNewDamage(AKNode *node)
{
    // Called again for background effects and just in case the user added a new node in notifyBegin
    createOrAssignTargetDataForNode(node);

    // Part of the node that ends up visible (relative to the target viewport)
    SkRegion clip;

    // Closest clipper parent
    AKNode *clipper { nullptr };

    // Tells if the node is using a background damage tracker
    bool hasBDT { false };

    // Dynamic cast for later use
    auto *renderable { node->asRenderable() };
    auto *bakeable { node->asBakeable() };
    auto *bgFx { node->asBackgroundEffect() };

    // Clear some flags
    node->m_flags.remove(AKNode::RenderedOnLastTarget);
    if (bakeable)
        static_cast<AKBakeable*>(node)->m_onBakeGeneratedDamage = false;

    if (bgFx)
    {
        // Background effect rects are calculated here because they depend on their target node rects
        bgFx->targetNodeRectCalculated();
        bgFx->m_worldRect = bgFx->effectRect.makeOffset(bgFx->targetNode()->worldRect().topLeft());
        bgFx->m_sceneRect = bgFx->m_worldRect.makeOffset(-ct->m_worldViewport.topLeft());
        bgFx->tData->visible = bgFx->visible() && bgFx->targetNode()->tData->visible;
    }
    else
    {
        node->m_sceneRect = node->m_worldRect.makeOffset(-ct->m_worldViewport.topLeft());
        // Mark the node as visible if visible() is true and its parent is visible too
        // The weird check is to handle the case where the parent is a root node
        bool parentIsVisible { node->parent() && !node->parent()->parent() && node->parent()->visible() };
        if (node->parent() && node->parent()->parent())
            parentIsVisible = node->parent()->tData->visible;
        node->tData->visible = node->visible() && parentIsVisible;
    }

    // Update intersected targets
    node->m_intersectedTargets.clear();
    for (AKTarget *target : targets())
        if (SkIRect::Intersects(node->worldRect(), target->m_worldViewport))
            node->m_intersectedTargets.insert(target);

    /// CALCULATE CLIP ///

    if (node->tData->visible)
        clip.setRect(node->m_sceneRect);
    clipper = node->closestClipperParent();
    if (clipper == root())
        clip.op(ct->m_sceneViewport, SkRegion::Op::kIntersect_Op);
    else
    {
        // Parent nodes are handled first, so prev means this frame
        clip.op(clipper->tData->prevSceneClip, SkRegion::Op::kIntersect_Op);
    }
    // Prevent rendering areas covered by opaque overlay nodes
    clip.op(ct->m_opaque, SkRegion::Op::kDifference_Op);
    // Also subtracted from the previous frame clip to avoid rendering the difference
    node->tData->prevSceneClip.op(ct->m_opaque, SkRegion::Op::kDifference_Op);

    // TODO: Move this?
    node->m_flags.setFlag(AKNode::InsideLastTarget, SkIRect::Intersects(node->m_worldRect, ct->m_worldViewport));

    /// BACKGROUND DAMAGE TRACKER ///

    node->bdt.capturedDamage.setEmpty();
    node->bdt.m_currentSurface.reset();
    if (node->tData->visible && node->bdt.enabled())
    {
        hasBDT = true;
        const SkScalar scale { node->bdt.scale() };
        SkISize size { ct->surface->geometry().dst.roundOut().size() };
        const int modW { size.fWidth % node->bdt.divisibleBy() };
        const int modH { size.fHeight % node->bdt.divisibleBy() };

        if (modW != 0)
            size.fWidth += node->bdt.divisibleBy() - modW;
        if (modH != 0)
            size.fHeight += node->bdt.divisibleBy() - modH;

        // TODO: Optimize all this mess
        if (node->bdt.m_surfaces.contains(ct.get()))
        {
            //AKLog(CZFatal, "BDT RESIZE {},{} {} {}", size.width(), size.height(), scale, true);
            node->bdt.m_surfaces[ct.get()]->resize(size, scale, true);
        }
        else
        {
            //AKLog(CZFatal, "BDT CREATE {},{} {} {}", size.width(), size.height(), scale, false);
            node->bdt.m_surfaces[ct.get()] = RSurface::Make(size, scale, false);
        }

        node->bdt.m_currentSurface = node->bdt.m_surfaces[ct.get()];
        auto geo { node->bdt.m_currentSurface->geometry() };
        geo.viewport.offsetTo(0, 0);
        node->bdt.m_currentSurface->setGeometry(geo);
        node->bdt.m_captureRectTranslated = node->bdt.captureRect().makeOffset(node->m_sceneRect.x(), node->m_sceneRect.y());
        SkRegion additionalAnyway { node->bdt.captureRectTranslated() };
        additionalAnyway.op(clip, SkRegion::Op::kDifference_Op);
        node->bdt.paintAnyway().translate(node->m_sceneRect.x(), node->m_sceneRect.y(), &node->bdt.m_paintAnywayTranslated);
        node->bdt.m_paintAnywayTranslated.op(additionalAnyway, SkRegion::Op::kUnion_Op);
        // reactive.op(clip, SkRegion::Op::kIntersect_Op); // remove?
        ct->m_bdts.push_back(&node->bdt);
        ct->m_opaque.op(node->bdt.captureRectTranslated(), SkRegion::Op::kDifference_Op);
    }

    /// BAKEABLES AND SUBSCENES ///

    const auto needsBake { bakeable && !clip.isEmpty() && (node->tData->changes.any() || !node->tData->damage.isEmpty() || !bakeable->surface() || bakeable->m_surfaceScale != node->scale()) };

    if (needsBake)
    {
        SkRegion clipRegion = clip;
        clipRegion.op(ct->m_clip, SkRegion::Op::kIntersect_Op);

        if (!clipRegion.isEmpty())
        {
            clipRegion.translate(-bakeable->m_sceneRect.x(), -bakeable->m_sceneRect.y());
            bool surfaceChanged;

            if (bakeable->surface())
            {
                surfaceChanged = bakeable->m_surfaceScale != node->scale();
                bakeable->m_surfaceScale = node->scale();
                surfaceChanged |= bakeable->m_surface->resize(
                    bakeable->worldRect().size(),
                    node->scale(), true);
            }
            else
            {
                surfaceChanged = true;
                bakeable->m_surface = RSurface::Make(
                    bakeable->worldRect().size(),
                    node->scale(), true);
            }

            const AKBakeEvent event (
                node->tData->changes,
                *ct,
                clipRegion,
                node->tData->damage,
                bakeable->opaqueRegion,
                bakeable->m_surface);

            if (surfaceChanged)
            {
                bakeable->tData->changes.set(AKRenderable::CHSize);
                event.damage.setRect(AK_IRECT_INF);
            }

            CZCore::Get()->sendEvent(event, *bakeable);
            bakeable->m_onBakeGeneratedDamage = !event.damage.isEmpty();
        }
    }

    if (renderable)
        renderable->handleCommonChanges(); // Generates damage so it must be called after onBake (to prevent unnecessary rebaking)
    else
        goto skipDamage; // Non-renderables do not generate damage

    if (node->m_sceneRect == node->tData->prevSceneRect)
    {
        node->tData->prevSceneClip.op(clip, SkRegion::Op::kXOR_Op);
        addNodeDamage(*node, node->tData->prevSceneClip);
        //t->m_damage.op(node->t->prevLocalClip, SkRegion::Op::kUnion_Op);

        node->tData->damage.translate(node->m_sceneRect.x(), node->m_sceneRect.y());

        node->tData->damage.op(clip, SkRegion::Op::kIntersect_Op);
        addNodeDamage(*node, node->tData->damage);
        //t->m_damage.op(node->t->clientDamage, SkRegion::Op::kUnion_Op);
    }
    else
    {
        // Both current and prev clip need to be repainted
        addNodeDamage(*node, node->tData->prevSceneClip);
        //t->m_damage.op(node->t->prevLocalClip, SkRegion::Op::kUnion_Op);

        addNodeDamage(*node, clip);
        //t->m_damage.op(clip, SkRegion::Op::kUnion_Op);
        node->tData->changes.set(AKRenderable::CHSize);
    }

skipDamage:
    node->tData->damage.setEmpty();
    node->tData->prevSceneClip = clip;
    node->tData->prevSceneRect = node->m_sceneRect;

    for (auto *backgroundEffect : node->backgroundEffects())
    {
        if (backgroundEffect->stackPosition() == AKBackgroundEffect::Behind)
            backgroundEffect->insertBefore(node);
        else
            backgroundEffect->insertBefore(node->parent()->children(true).front());
    }

    auto bdtIt = std::find(ct->m_bdts.begin(), ct->m_bdts.end(), &node->bdt);

    if (hasBDT)
    {
        if (bdtIt != ct->m_bdts.end())
            ct->m_bdts.erase(bdtIt);
    }

    if (!node->asSubScene())
        for (Int64 i = node->children(true).size() - 1; i >= 0;)
        {
            if (node->children(true)[i]->m_flags.has(AKNode::Skip))
            {
                i--;
                continue;
            }

            const int skip = node->children(true)[i]->backgroundEffects().size();
            calculateNewDamage(node->children(true)[i]);
            i -= 1 - skip;
        }

    node->m_overlayBdts = ct->m_bdts;

    if (hasBDT)
        ct->m_bdts.insert(bdtIt, &node->bdt);

    if (!renderable)
        return;

    node->m_flags.setFlag(AKNode::RenderedOnLastTarget, !ct->m_opaque.contains(clip) && !clip.isEmpty() && !node->m_worldRect.isEmpty());

    if (!renderable->renderedOnLastTarget())
        return;

    renderable->tData->opaqueOverlay = ct->m_opaque;

    switch (renderable->m_colorHint)
    {
    case AKRenderable::ColorHint::Opaque:
        renderable->tData->opaque = clip;
        break;
    case AKRenderable::ColorHint::Translucent:
        renderable->tData->opaque.setEmpty();
        break;
    case AKRenderable::ColorHint::UseRegion:
        renderable->opaqueRegion.translate(node->m_sceneRect.x(), node->m_sceneRect.y(), &renderable->tData->opaque);
        renderable->tData->opaque.op(clip, SkRegion::kIntersect_Op);
        break;
    }

    if (renderable->m_opacity <= 0.f || renderable->m_colorFactor.fA <= 0.f)
        renderable->tData->invisble.setRect(node->m_sceneRect);
    else
        renderable->invisibleRegion.translate(node->m_sceneRect.x(), node->m_sceneRect.y(), &renderable->tData->invisble);

    renderable->tData->opaque.op(renderable->tData->invisble, SkRegion::kDifference_Op);
    ct->m_opaque.op(renderable->tData->opaque, SkRegion::kUnion_Op);
    renderable->tData->translucent = clip;
    renderable->tData->translucent.op(renderable->tData->opaque, SkRegion::kDifference_Op);

    if (ct->outInvisible)
    {
        ct->outInvisible->op(renderable->tData->opaque, SkRegion::kDifference_Op);
        ct->outInvisible->op(renderable->tData->translucent, SkRegion::kDifference_Op);
    }
}

void AKScene::updateDamageRing() noexcept
{
    updateDamageTrackers();

    if (ct->age == 0)
    {
        ct->m_damage.setRect(AK_IRECT_INF);
        ct->m_damageRing[ct->m_damageIndex] = ct->m_damage;
    }
    else
    {
        if (ct->inDamage)
            ct->m_damage.op(*ct->inDamage, SkRegion::Op::kUnion_Op);

        ct->m_damageRing[ct->m_damageIndex] = ct->m_damage;

        for (UInt32 i = 1; i < ct->age; i++)
        {
            Int32 damageIndex = ct->m_damageIndex - i;

            if (damageIndex < 0)
                damageIndex = AK_MAX_BUFFER_AGE + damageIndex;

            ct->m_damage.op(ct->m_damageRing[damageIndex], SkRegion::Op::kUnion_Op);
        }
    }

    ct->m_damage.op(ct->m_sceneViewport, SkRegion::Op::kIntersect_Op);
    ct->m_opaque.op(ct->m_sceneViewport, SkRegion::Op::kIntersect_Op);

    if (ct->inClip)
        ct->m_damage.op(*ct->inClip, SkRegion::Op::kIntersect_Op);

    if (ct->outDamage)
        *ct->outDamage = ct->m_damage;

    if (ct->outOpaque)
        *ct->outOpaque = ct->m_opaque;

    if (ct->m_damageIndex == AK_MAX_BUFFER_AGE - 1)
        ct->m_damageIndex = 0;
    else
        ct->m_damageIndex++;
}

void AKScene::updateDamageTrackers() noexcept
{
    for (auto &bdt : ct->m_bdts)
    {
        if (bdt->capturedDamage.isEmpty())
            continue;

        bdt->capturedDamage.op(bdt->node().m_sceneRect, SkRegion::Op::kIntersect_Op);
        bdt->capturedDamage.op(ct->m_sceneViewport, SkRegion::Op::kIntersect_Op);

        if (bdt->damageOutset() != 0)
        {
            SkRegion outset;
            SkRegion::Iterator it (bdt->capturedDamage);

            while (!it.done())
            {
                outset.op(it.rect().makeOutset(bdt->damageOutset(), bdt->damageOutset()), SkRegion::Op::kUnion_Op);
                it.next();
            }

            bdt->capturedDamage = outset;
        }

        bdt->capturedDamage.op(bdt->node().m_sceneRect, SkRegion::Op::kIntersect_Op);
        bdt->capturedDamage.op(ct->m_sceneViewport, SkRegion::Op::kIntersect_Op);
        ct->m_damage.op(bdt->capturedDamage, SkRegion::Op::kUnion_Op);
    }

    // Prev bdts that are now hidden
    bool found;
    while (!ct->m_bdtsPrev.empty())
    {
        found = false;
        for (auto &bdt : ct->m_bdts)
        {
            if (bdt.get() == ct->m_bdtsPrev.back().get())
            {
                found = true;
                break;
            }
        }

        if (!found)
            ct->m_damage.op(ct->m_bdtPrevRectsTranslated.back(), SkRegion::Op::kUnion_Op);

        ct->m_bdtsPrev.pop_back();
        ct->m_bdtPrevRectsTranslated.pop_back();
    }

    // Store the current ones
    ct->m_bdtsPrev = ct->m_bdts;
    for (auto &bdt : ct->m_bdtsPrev)
        ct->m_bdtPrevRectsTranslated.emplace_back(bdt->captureRectTranslated());
}

void AKScene::renderBackground() noexcept
{
    SkRegion background { ct->m_damage };
    background.op(ct->m_opaque, SkRegion::Op::kDifference_Op);
    SkRegion aux, anyway;

    for (auto bdt = ct->m_bdts.rbegin(); bdt != ct->m_bdts.rend(); bdt++)
    {
        (*bdt)->capturedDamage.setEmpty();

        if (!aux.op((*bdt)->captureRectTranslated(), background, SkRegion::kIntersect_Op))
            continue;

        (*bdt)->capturedDamage.op(aux, SkRegion::Op::kUnion_Op);
        anyway.op((*bdt)->m_paintAnywayTranslated, background, SkRegion::kIntersect_Op);

        auto pass { (*bdt)->m_surfaces[ct.get()]->beginPass(RPassCap_Painter) };
        backgroundPass(pass, aux);
        background.op(aux, SkRegion::kDifference_Op);
        background.op(anyway, SkRegion::kUnion_Op);
    }

    backgroundPass(pass, background);
}

void AKScene::renderTree() noexcept
{
    for (size_t i = 0; i < root()->children(true).size();)
    {
        if (root()->children(true)[i]->m_flags.has(AKNode::Skip))
        {
            i++;
            continue;
        }

        renderNodes(root()->children(true)[i]);
        root()->children(true)[i]->tData->changes.reset();

        if (root()->children(true)[i]->asBackgroundEffect())
            root()->children(true)[i]->setParent(nullptr);
        else
            i++;
    }
}

void AKScene::resetTarget() noexcept
{
    ct->m_bdts.clear();
    ct->m_prevBakedNodesScale = ct->m_bakedNodesScale;
    ct->m_isDirty = false;
    ct->m_needsFullRepaint = false;
    ct->m_damage.setEmpty();
    ct->m_opaque.setEmpty();
    ct->m_clip.setEmpty();
    ct->m_translucent.setEmpty();
    ct->m_bdts.clear();
    ct.reset();
}

void AKScene::backgroundPass(std::shared_ptr<RPass> pass, SkRegion &region) noexcept
{
    pass->save();
    auto *p { pass->getPainter() };
    p->setOption(RPainter::ColorIsPremult, true);
    p->setBlendMode(RBlendMode::Src);
    p->setColor(ct->clearColor());
    p->drawColor(region);
    pass->restore();
}

void AKScene::renderNodes(AKNode *node)
{
    auto *rend { node->asRenderable() };
    SkRegion aux, anyway;

    if (node->m_flags.has(AKNode::Skip))
        return;

    if (!rend || !node->renderedOnLastTarget())
        goto renderChildren;

    if (node->tData->translucent.isEmpty())
        goto renderOpaque;

    rend->tData->translucent.op(rend->tData->invisble, SkRegion::kDifference_Op);
    rend->tData->translucent.op(ct->m_damage, SkRegion::kIntersect_Op);
    rend->tData->translucent.op(rend->tData->opaqueOverlay, SkRegion::kDifference_Op);

    if (node->tData->translucent.isEmpty())
        goto renderOpaque;

    for (auto bdt = rend->m_overlayBdts.rbegin(); bdt != rend->m_overlayBdts.rend(); bdt++)
    {
        if (!aux.op((*bdt)->captureRectTranslated(), node->tData->translucent, SkRegion::kIntersect_Op))
            continue;

        (*bdt)->capturedDamage.op(aux, SkRegion::Op::kUnion_Op);
        anyway.op((*bdt)->m_paintAnywayTranslated, node->tData->translucent, SkRegion::kIntersect_Op);
        auto pass { (*bdt)->m_surfaces[ct.get()]->beginPass() };
        nodeTranslucentPass(rend, pass, aux);
        node->tData->translucent.op(aux, SkRegion::kDifference_Op);
        node->tData->translucent.op(anyway, SkRegion::kUnion_Op);
    }

    nodeTranslucentPass(rend, pass, node->tData->translucent);
renderOpaque:

    if (node->tData->opaque.isEmpty())
        goto renderChildren;

    rend->tData->opaque.op(ct->m_damage, SkRegion::kIntersect_Op);
    rend->tData->opaque.op(rend->tData->opaqueOverlay, SkRegion::kDifference_Op);

    if (rend->tData->opaque.isEmpty())
        goto renderChildren;

    for (auto bdt = rend->m_overlayBdts.rbegin(); bdt != rend->m_overlayBdts.rend(); bdt++)
    {
        if (!aux.op((*bdt)->captureRectTranslated(), node->tData->opaque, SkRegion::kIntersect_Op))
            continue;

        (*bdt)->capturedDamage.op(aux, SkRegion::Op::kUnion_Op);
        anyway.op((*bdt)->m_paintAnywayTranslated, node->tData->opaque, SkRegion::kIntersect_Op);
        auto pass { (*bdt)->m_surfaces[ct.get()]->beginPass() };
        nodeOpaquePass(rend, pass, aux);
        node->tData->opaque.op(aux, SkRegion::kDifference_Op);
        node->tData->opaque.op(anyway, SkRegion::kUnion_Op);
    }

    nodeOpaquePass(rend, pass, node->tData->opaque);

renderChildren:

    if (!node->asSubScene())
        for (size_t i = 0; i < node->children(true).size();)
        {
            if (node->children(true)[i]->m_flags.has(AKNode::Skip))
            {
                i++;
                continue;
            }

            renderNodes(node->children(true)[i]);
            node->children(true)[i]->tData->changes.reset();
            YGNodeSetHasNewLayout(node->m_layout.m_node, false);

            if (node->children(true)[i]->asBackgroundEffect())
                node->children(true)[i]->setParent(nullptr);
            else
                i++;
        }
}

void AKScene::nodeTranslucentPass(AKRenderable *node, std::shared_ptr<RPass> pass, SkRegion &region) noexcept
{
    pass->save();
    SetPassParamsFromRenderable(pass, node, false);
    CZCore::Get()->sendEvent(AKRenderEvent(*ct.get(), region, node->m_sceneRect, pass, false), *node);
    pass->restore();
}

void AKScene::nodeOpaquePass(AKRenderable *node, std::shared_ptr<RPass> pass, SkRegion &region) noexcept
{
    pass->save();
    SetPassParamsFromRenderable(pass, node, true);
    CZCore::Get()->sendEvent(AKRenderEvent(*ct.get(), region, node->m_sceneRect, pass, true), *node);
    pass->restore();
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

        if (!it.node()->worldRect().contains(ipos.x(), ipos.y()))
        {
            it.next();
            continue;
        }

        if (it.node()->inputRegion() && !it.node()->inputRegion()->contains(
                ipos.x() - it.node()->worldRect().x(),
                ipos.y() - it.node()->worldRect().y()))
        {
            it.next();
            continue;
        }

        clipper = it.node()->closestClipperParent();

        if (clipper)
        {
            if (!clipper->worldRect().contains(ipos.x(), ipos.y()))
            {
                it.next();
                continue;
            }

            if (clipper->inputRegion() && !clipper->inputRegion()->contains(
                    ipos.x() - clipper->worldRect().x(),
                    ipos.y() - clipper->worldRect().y()))
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
    for (AKNode *child : node->children(true))
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

CZBitset<CZWindowState> AKScene::windowState() const noexcept
{
    if (isSubScene())
        return {};

    return m_win->windowState;
}

MSurface *AKScene::window() const noexcept
{
    if (isSubScene())
        return {};

    return m_win->window;
}

void AKScene::addNodeDamage(AKNode &, const SkRegion &damage) noexcept
{
    ct->m_damage.op(damage, SkRegion::Op::kUnion_Op);

    for (auto &bdt : ct->m_bdts)
        if (SkIRect::Intersects(bdt->captureRectTranslated(), damage.getBounds()))
            bdt->capturedDamage.op(damage, SkRegion::Op::kUnion_Op);
}

bool AKScene::event(const CZEvent &event) noexcept
{
    if (!m_root || m_isSubScene)
        return AKObject::event(event);

    switch (event.type()) {
    case CZEvent::Type::PointerMove:
        pointerMoveEvent((const CZPointerMoveEvent&)event);
        break;
    case CZEvent::Type::PointerButton:
        pointerButtonEvent((const CZPointerButtonEvent&)event);
        break;
    case CZEvent::Type::PointerScroll:
        pointerScrollEvent((const CZPointerScrollEvent&)event);
        break;
    case CZEvent::Type::KeyboardKey:
        keyboardKeyEvent((const CZKeyboardKeyEvent&)event);
        break;
    case CZEvent::Type::PointerEnter:
        pointerEnterEvent((const CZPointerEnterEvent&)event);
        break;
    case CZEvent::Type::PointerLeave:
        pointerLeaveEvent((const CZPointerLeaveEvent&)event);
        break;
    case CZEvent::Type::KeyboardEnter:
        keyboardEnterEvent((const CZKeyboardEnterEvent&)event);
        break;
    case CZEvent::Type::KeyboardLeave:
        keyboardLeaveEvent((const CZKeyboardLeaveEvent&)event);
        break;
    case CZEvent::Type::WindowState:
        windowStateEvent((const CZWindowStateEvent&)event);
        break;
    default:
        return AKObject::event(event);
        break;
    }

    return true;
}

void AKScene::setPointerGrab(AKNode *node) noexcept
{
    if (isSubScene() || (node && node->scene() != this))
        return;

    m_win->pointerGrab.reset(node);
}

AKNode *AKScene::pointerGrab() const noexcept
{
    if (isSubScene())
        return {};

    return m_win->pointerGrab;
}

AKNode *AKScene::pointerFocus() const noexcept
{
    if (isSubScene())
        return {};

    return m_win->pointerFocus;
}

AKNode *AKScene::keyboardFocus() const noexcept
{
    if (isSubScene())
        return {};

    return m_win->keyboardFocus;
}

void AKScene::pointerEnterEvent(const CZPointerEnterEvent &e) noexcept
{
    auto app { AKApp::Get() };

    if (app->pointer().focus())
        return;

    auto self { m_self.lock() };
    app->pointer().m_history.enter = e;
    app->pointer().m_history.move.pos = e.pos;
    app->pointer().m_focus.reset(this);
    app->pointer().onFocusChanged.notify();

    CZPointerMoveEvent move {};
    move.pos = e.pos;
    move.device = e.device;
    move.serial = e.serial;
    move.userData = e.userData;
    move.ms = e.ms;
    move.us = e.us;
    pointerMoveEvent(move);
}

void AKScene::pointerMoveEvent(const CZPointerMoveEvent &e) noexcept
{
    auto app { AKApp::Get() };

    if (app->pointer().focus() != this)
        return;

    CZPointerLeaveEvent leave {};
    CZPointerEnterEvent enter {};
    enter.pos = e.pos;
    enter.device = leave.device = e.device;
    enter.serial = leave.serial = e.serial;
    enter.userData = leave.userData = e.userData;
    enter.ms = leave.ms = e.ms;
    enter.us = leave.us = e.us;

    if (pointerGrab())
    {
        EnqueueAndPropagateToParent(e, pointerGrab(), nullptr, false);
    }
    else
    {
        auto *node { nodeAt(app->pointer().pos()) };

        if (node)
        {
            if (pointerFocus())
            {
                if (pointerFocus() == node)
                {
                    EnqueueAndPropagateToParent(e, pointerFocus(), nullptr, false);
                }
                else if (pointerFocus()->isSubchildOf(node))
                {
                    EnqueueAndPropagateToParent(leave, pointerFocus(), node, false);
                }
                else if (node->isSubchildOf(pointerFocus()))
                {
                    EnqueueAndPropagateToParent(e, node, pointerFocus(), false);
                }
                else
                {
                    EnqueueAndPropagateToParent(leave, pointerFocus(), nullptr, false);
                    EnqueueAndPropagateToParent(enter, node, nullptr, false);
                }
            }
            else
            {
                EnqueueAndPropagateToParent(enter, pointerFocus(), nullptr, false);
            }
        }
        else
        {
            if (pointerFocus())
            {
                EnqueueAndPropagateToParent(leave, pointerFocus(), nullptr, false);
                m_win->pointerFocus.reset();
            }
        }

        m_win->pointerFocus.reset(node);
    }
}

void AKScene::pointerButtonEvent(const CZPointerButtonEvent &e) noexcept
{
    auto app { AKApp::Get() };

    if (app->pointer().focus() != this)
        return;

    auto *target { pointerGrab() };

    if (!target)
        target = pointerFocus();

    if (!target)
        return;

    EnqueueAndPropagateToParent(e, target, nullptr, false);
}

void AKScene::pointerScrollEvent(const CZPointerScrollEvent &e) noexcept
{
    auto app { AKApp::Get() };

    if (app->pointer().focus() != this)
        return;

    auto *target { pointerGrab() };

    if (!target)
        target = pointerFocus();

    if (!target)
        return;

    EnqueueAndPropagateToParent(e, target, nullptr, false);
}

void AKScene::pointerLeaveEvent(const CZPointerLeaveEvent &e) noexcept
{
    auto app { AKApp::Get() };

    if (app->pointer().focus() != this)
        return;

    auto self { m_self.lock() };
    app->pointer().m_focus.reset();
    app->pointer().onFocusChanged.notify();

    if (!pointerFocus())
        return;

    EnqueueAndPropagateToParent(e, pointerFocus(), nullptr, false);
    m_win->pointerFocus.reset();
}

void AKScene::keyboardEnterEvent(const CZKeyboardEnterEvent &e) noexcept
{
    auto app { AKApp::Get() };

    if (app->keyboard().focus())
        return;

    auto self { m_self.lock() };
    app->keyboard().m_history.enter = e;
    app->keyboard().m_focus.reset(this);
    app->keyboard().onFocusChanged.notify();
}

void AKScene::keyboardKeyEvent(const CZKeyboardKeyEvent &e) noexcept
{
    auto app { AKApp::Get() };

    if (app->keyboard().focus() != this)
        return;

    auto core { app->core() };

    if (core->keymap()->repeatRateMs() == 0 || core->keymap()->pressedKeys().empty())
    {
        m_win->keyDelayTimer.stop(false);
        m_win->keyRepeatTimer.stop(false);
        m_win->repeatedKey = -1;
    }
    else if (e.isPressed && !e.isRepeat && e.code != m_win->repeatedKey)
    {
        m_win->keyRepeatTimer.stop(false);
        m_win->repeatedKey = e.code;
        m_win->keyDelayTimer.start(core->keymap()->repeatDelayMs());
    }

    if (keyboardFocus())
        EnqueueAndPropagateToParent(e, keyboardFocus(), nullptr, false);
}

void AKScene::keyboardLeaveEvent(const CZKeyboardLeaveEvent &e) noexcept
{
    auto app { AKApp::Get() };

    if (app->keyboard().focus() != this)
        return;

    auto self { m_self.lock() };
    app->keyboard().m_focus.reset();
    app->keyboard().onFocusChanged.notify();

    if (!keyboardFocus())
        return;

    EnqueueAndPropagateToParent(e, keyboardFocus(), nullptr, false);
    m_win->keyboardFocus.reset();
}

void AKScene::windowStateEvent(const CZWindowStateEvent &e) noexcept
{
    m_win->windowState = e.newState;
    if (!m_root) return;

    auto core { CZCore::Get() };
    EnqueueAndPropagateToChildren(core, e.copy(), m_root);
}

void AKScene::SetPassParamsFromRenderable(std::shared_ptr<RPass> pass, AKRenderable *rend, bool opaque) noexcept
{
    // TODO: Do this for SkCanvas too
    auto *p { pass->getPainter(false) };

    // AKRenderable::color is unpremult
    p->setOption(RPainter::ColorIsPremult, false);
    p->setOption(RPainter::ReplaceImageColor, rend->replaceImageColorEnabled());
    p->setColor(rend->color());
    p->setOpacity(rend->opacity());

    auto factor { rend->colorFactor() };

    if (!rend->activated() && rend->diminishOpacityOnInactive())
        factor.fA *= AKTheme::RenderableInactiveOpacityFactor;

    p->setFactor(factor);

    if (opaque)
        p->setBlendMode(RBlendMode::Src);
    else
    {
        if (rend->customBlendModeEnabled())
            p->setBlendMode(rend->customBlendMode());
        else
            p->setBlendMode(RBlendMode::SrcOver);
    }    
}
