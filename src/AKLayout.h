#ifndef AKLAYOUT_H
#define AKLAYOUT_H

#include <AKNode.h>

class AK::AKLayout : public AKNode
{
public:
    AKLayout(AKNode *parent) noexcept : AKNode(parent){}
};

#endif // AKLAYOUT_H
