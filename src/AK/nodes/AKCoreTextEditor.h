#ifndef AKCORETEXTEDITOR_H
#define AKCORETEXTEDITOR_H

#include <AK/nodes/AKText.h>
#include <AK/nodes/AKContainer.h>

class AK::AKCoreTextEditor : public AKContainer
{
public:
    AKCoreTextEditor(AKNode *parent = nullptr) noexcept;
    AKCLASS_NO_COPY(AKCoreTextEditor)
};

#endif // AKCORETEXTEDITOR_H
