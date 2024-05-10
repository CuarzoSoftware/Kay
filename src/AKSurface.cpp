#include <private/AKSurface.h>
#include <private/AKApplication.h>
#include <AKFrame.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <iostream>
#include "include/core/SkColorSpace.h"

using namespace AK;

AKSurface::AKSurface(const AKSize &size) noexcept : m_imp(std::make_unique<Private>())
{
    imp()->frame.reset(new AKFrame(this, size));
    app()->imp()->surfaces.emplace_back(this);
    imp()->appLink = std::prev(app()->imp()->surfaces.end());
}

AKSurface::~AKSurface() noexcept
{
    if (app())
        app()->imp()->surfaces.erase(imp()->appLink);
}

AKFrame *AKSurface::widget() const noexcept
{
    return imp()->frame.get();
}
