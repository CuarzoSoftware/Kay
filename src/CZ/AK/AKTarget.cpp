#include <CZ/AK/AKTarget.h>
#include <CZ/AK/Nodes/AKNode.h>
#include <CZ/AK/AKScene.h>
#include <CZ/Utils/CZVectorUtils.h>

using namespace CZ;

void AKTarget::setClearColor(SkColor color) noexcept
{
    if (m_clearColor == color)
        return;

    m_clearColor = color;
    m_needsFullRepaint = true;
    markDirty();
}

void AKTarget::setBakedNodesScale(Int32 scale) noexcept
{
    if (scale == m_bakedNodesScale)
        return;

    m_bakedNodesScale = std::clamp(scale, 1, 4);
    markDirty();
}

void AKTarget::markDirty() noexcept
{
    //if (isDirty())
    //    return;

    m_isDirty = true;
    onMarkedDirty.notify(*this);
}

AKTarget::AKTarget(std::shared_ptr<AKScene> scene) noexcept : m_scene(scene)
{
    scene->m_targets.emplace_back(this);
}

AKTarget::~AKTarget() noexcept
{
    CZVectorUtils::RemoveOneUnordered(m_scene->m_targets, this);
    notifyDestruction();
}
