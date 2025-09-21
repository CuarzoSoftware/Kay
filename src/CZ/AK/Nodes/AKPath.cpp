#include <CZ/Ream/RPass.h>
#include <CZ/Ream/RSurface.h>
#include <CZ/AK/Nodes/AKPath.h>
#include <CZ/AK/Events/AKBakeEvent.h>
#include <CZ/skia/core/SkCanvas.h>

using namespace CZ;

void AKPath::onSceneBegin()
{
    if (changes().test(CHPath))
    {
        m_bounds = m_path.getBounds();
        m_bounds.outset(1.f, 1.f);
        m_matrix.setIdentity();
        m_matrix.preScale(SkScalar(worldRect().width())/m_bounds.width(), SkScalar(worldRect().height())/m_bounds.height());
        m_matrix.preTranslate(-m_bounds.x() + 1.f, -m_bounds.y() + 1.f);
    }
}

void AKPath::bakeEvent(const AKBakeEvent &e)
{
    if (e.damage.isEmpty() && !e.changes.test(CHPath))
        return;

    auto pass { e.surface->beginPass() };
    auto *c { pass->getCanvas() };

    c->save();
    c->clipIRect(SkIRect::MakeSize(worldRect().size()));
    c->clear(SK_ColorTRANSPARENT);
    c->concat(m_matrix);
    c->drawPath(path(), m_brush);
    c->restore();
}
