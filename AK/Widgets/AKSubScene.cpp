#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrDirectContext.h"

#include <AK/Widgets/AKSubScene.h>
#include <include/core/SkCanvas.h>

using namespace AK;

AKSubScene::AKSubScene(AKNode *parent) noexcept : AKBakeable(parent)
{
    setClipsChildren(true);
    m_caps |= Scene;
}

void AKSubScene::bakeChildren(SkCanvas *canvas, const SkRegion &clip, bool surfaceChanged, const SkRegion *inDamageRegion) noexcept
{
    TargetData *parentTargetData { t };
    AKTarget *target;
    bool isNewTarget;

    if (m_targets.find(t->target) == m_targets.end())
    {
        target = m_scene.createTarget();
        target->root = this;
        target->m_isSubScene = true;
        m_targets[t->target] = target;
        isNewTarget = true;
    }
    else {
        target = m_targets[t->target];
        isNewTarget = false;
    }

    target->age = surfaceChanged || isNewTarget ? 0 : 1;
    target->surface = t->bake.surface;
    target->surface->recordingContext()->asDirectContext()->resetContext();
    target->viewport = SkRect::MakeWH(layout().calculatedWidth(), layout().calculatedHeight());
    target->scale = t->target->scale;
    target->dstRect = t->bake.srcRect.roundOut();
    target->inClipRegion = &clip;
    target->inDamageRegion = inDamageRegion;
    target->outDamageRegion = &t->clientDamage;
    target->outOpaqueRegion = &m_opaqueRegion;
    canvas->save();
    canvas->setMatrix(SkMatrix());
    m_scene.render(target);
    canvas->restore();
    t = parentTargetData;
}

void AKSubScene::onBake(SkCanvas *canvas, const SkRegion &clip, bool surfaceChanged)
{
    //canvas->clear(SkScalarRoundToInt(t->target->m_xyScale.x()) == 2 ? SK_ColorGREEN : SK_ColorYELLOW);
    bakeChildren(canvas, clip, surfaceChanged);
}
