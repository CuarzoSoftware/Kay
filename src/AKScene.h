#ifndef AKSCENE_H
#define AKSCENE_H

#include <AKObject.h>
#include <AKTarget.h>
#include <AKNode.h>
#include <memory.h>
#include <vector>

class AK::AKScene : public AKObject
{
public:
    AKScene() noexcept;
    AKTarget *createTarget() noexcept;
    bool destroyTarget(AKTarget *target);
    bool render(AKTarget *target);
    void setClearColor(SkColor color) noexcept
    {
        m_clearColor = color;
    }
private:
    friend class AKTarget;
    SkCanvas *c;
    SkMatrix m_matrix;
    AKTarget *t;
    std::vector<AKTarget*> m_targets;
    SkColor m_clearColor { SK_ColorWHITE };
    void validateTarget(AKTarget *target) noexcept;
    void updateMatrix() noexcept;
    void calculateNewDamage(AKNode *node);
    void updateDamageRing() noexcept;
    void renderOpaque(AKNode *node);
    void renderBackground() noexcept;
    void renderTranslucent(AKNode *node);
};

#endif // AKSCENE_H
