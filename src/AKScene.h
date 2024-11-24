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
    std::shared_ptr<AKTarget> createTarget() noexcept;
    bool render(std::shared_ptr<AKTarget> target);

    AKNode *root() noexcept
    {
        return &m_root;
    }

private:
    AKNode m_root;
    SkCanvas *c;
    SkMatrix m_matrix;
    AKTarget *t;
    std::vector<std::shared_ptr<AKTarget>> m_targets;
    void updateMatrix() noexcept;
    void calculateNewDamage(AKNode *node);
    void renderOpaque(AKNode *node);
    void renderBackground() noexcept;
    void renderTranslucent(AKNode *node);
};

#endif // AKSCENE_H
