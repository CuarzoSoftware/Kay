#include <AKSubScene.h>
#include <include/core/SkCanvas.h>

using namespace AK;

AKSubScene::AKSubScene(AKNode *parent) noexcept : AKBakeable(parent)
{
    m_caps |= Scene;
    m_scene.setClearColor(SK_ColorCYAN);
}

void AKSubScene::onBake(SkCanvas *canvas, const SkRect &clip, bool surfaceChanged)
{
    auto *parentTarget { t };
    AKTarget *target;

    if (m_targets.find(t->target) == m_targets.end())
    {
        target = m_scene.createTarget();
        target->root = this;
        m_targets[t->target] = target;
    }
    else {
        target = m_targets[t->target];
    }

    target->age = surfaceChanged ? 0 : 1;
    target->surface = t->bake.surface;
    target->viewport = SkRect::MakeXYWH(
        clip.left(),
        clip.top(),
        clip.width(), clip.height());
    target->scale = t->target->scale;
    target->dstRect.fLeft = clip.fLeft * t->target->m_xyScale.x();
    target->dstRect.fTop = clip.fTop * t->target->m_xyScale.y();
    target->dstRect.fRight = clip.fRight * t->target->m_xyScale.x();
    target->dstRect.fBottom = clip.fBottom * t->target->m_xyScale.y();
    target->outDamageRegion = &t->clientDamage;
    target->outOpaqueRegion = &m_opaqueRegion;
    canvas->save();
    canvas->setMatrix(SkMatrix());
    m_scene.render(target);
    canvas->restore();
    t = parentTarget;
}
