#ifndef AKCONTAINER_H
#define AKCONTAINER_H

#include <AKNode.h>

class AK::AKContainer : public AKNode
{
public:
    AKContainer(AKNode *parent = nullptr) noexcept : AKNode(parent){}
};

#endif // AKCONTAINER_H
