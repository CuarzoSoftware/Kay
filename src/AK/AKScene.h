#ifndef AKSCENE_H
#define AKSCENE_H

#include <AK/AKObject.h>
#include <AK/AKTarget.h>
#include <AK/nodes/AKNode.h>
#include <memory.h>
#include <vector>

class AK::AKScene : public AKObject
{
public:
    AKScene() noexcept = default;
    AKTarget *createTarget(std::shared_ptr<AKPainter> painter = nullptr) noexcept;
    bool destroyTarget(AKTarget *target);
    bool render(AKTarget *target);
    void setClearColor(SkColor color) noexcept
    {
        m_clearColor = SkColor4f::FromColor(color);
    }
private:
    friend class AKTarget;
    SkCanvas *c;
    AKTarget *t;
    SkMatrix m_matrix;
    std::vector<AKTarget*> m_targets;
    SkColor4f m_clearColor { 0.f, 0.f, 0.f, 0.f };
    void validateTarget(AKTarget *target) noexcept;
    void updateMatrix() noexcept;
    void notifyBegin(AKNode *node);
    void calculateNewDamage(AKNode *node);
    void updateDamageRing() noexcept;
    void renderBackground() noexcept;
    void renderNodes(AKNode *node);
};

#endif // AKSCENE_H
