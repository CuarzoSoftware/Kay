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

void AKPath::onBake(OnBakeParams *params)
{
    if (params->damage->isEmpty() && !changes().test(CHPath))
        return;

    params->surface->shrink();

    SkCanvas &c { *params->surface->surface()->getCanvas() };
    c.save();
    c.clipIRect(SkIRect::MakeSize(globalRect().size()));
    c.clear(SK_ColorTRANSPARENT);
    c.concat(m_matrix);
    c.drawPath(path(), m_brush);
    c.restore();
}
