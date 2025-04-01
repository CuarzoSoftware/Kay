#ifndef DOCKCONTAINER_H
#define DOCKCONTAINER_H

#include <AK/nodes/AKThreeImagePatch.h>

using namespace AK;

class DockContainer : public AKThreeImagePatch
{
public:
    DockContainer(AKNode *parent) noexcept;
protected:
    void layoutEvent(const AKLayoutEvent &event) override;
    void updateImage() noexcept;
};

#endif // DOCKCONTAINER_H
