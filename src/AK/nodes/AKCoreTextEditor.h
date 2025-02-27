#ifndef AKCORETEXTEDITOR_H
#define AKCORETEXTEDITOR_H

#include <AK/nodes/AKText.h>
#include <AK/nodes/AKContainer.h>

class AK::AKCoreTextEditor : public AKContainer
{
public:
    AKCoreTextEditor(AKNode *parent = nullptr) noexcept;
};

#endif // AKCORETEXTEDITOR_H
