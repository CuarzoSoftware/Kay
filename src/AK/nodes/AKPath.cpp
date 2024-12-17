#include "include/gpu/GrDirectContext.h"
#include <AK/AKSurface.h>
#include <AK/nodes/AKPath.h>
#include <include/core/SkCanvas.h>

using namespace AK;

void AKPath::onSceneBegin()
{
    return;
    m_bounds = path().getBounds();

    if (changes().test(Chg_Path))
    {
        m_bounds = path().getBounds();

        layout().setWidth(m_bounds.width());
        layout().setMaxWidth(m_bounds.width());
        layout().setMinWidth(m_bounds.width());

        layout().setHeight(m_bounds.height());
        layout().setMaxHeight(m_bounds.height());
        layout().setMinHeight(m_bounds.height());
    }
}

void AKPath::onLayoutUpdate()
{
    m_bounds = m_path.getBounds();
    m_bounds.outset(1.f, 1.f);
    m_matrix.setIdentity();
    m_matrix.preScale(SkScalar(rect().width())/m_bounds.width(), SkScalar(rect().height())/m_bounds.height());
    m_matrix.preTranslate(-m_bounds.x() + 1.f, -m_bounds.y() + 1.f);
}

void AKPath::onBake(OnBakeParams *params)
{
    if (params->damage->isEmpty() && !changes().test(Chg_Path))
        return;

    params->surface->shrink();

    SkCanvas &c { *params->surface->surface()->getCanvas() };
    c.save();
    c.clipIRect(SkIRect::MakeSize(rect().size()));
    c.clear(SK_ColorTRANSPARENT);
    c.concat(m_matrix);
    c.drawPath(path(), m_brush);
    c.restore();
}
