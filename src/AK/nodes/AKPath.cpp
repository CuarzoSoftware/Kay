#include <AK/AKSurface.h>
#include <AK/nodes/AKPath.h>
#include <include/core/SkCanvas.h>

using namespace AK;

void AKPath::onSceneBegin()
{
    if (changes().test(CHPath))
    {
        m_bounds = m_path.getBounds();
        m_bounds.outset(1.f, 1.f);
        m_matrix.setIdentity();
        m_matrix.preScale(SkScalar(globalRect().width())/m_bounds.width(), SkScalar(globalRect().height())/m_bounds.height());
        m_matrix.preTranslate(-m_bounds.x() + 1.f, -m_bounds.y() + 1.f);
    }
}

void AKPath::onBake(const BakeEvent &event)
{
    if (event.damage.isEmpty() && !event.changes.test(CHPath))
        return;

    event.surface.shrink();
    event.canvas().save();
    event.canvas().clipIRect(SkIRect::MakeSize(globalRect().size()));
    event.canvas().clear(SK_ColorTRANSPARENT);
    event.canvas().concat(m_matrix);
    event.canvas().drawPath(path(), m_brush);
    event.canvas().restore();
}
