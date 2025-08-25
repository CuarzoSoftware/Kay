#ifndef CZ_AKCORETEXTEDITOR_H
#define CZ_AKCORETEXTEDITOR_H

#include <CZ/AK/Nodes/AKText.h>
#include <CZ/AK/Nodes/AKContainer.h>

class CZ::AKCoreTextEditor : public AKContainer
{
public:
    AKCoreTextEditor(AKNode *parent = nullptr) noexcept;
    CZ_DISABLE_COPY(AKCoreTextEditor)
};

#endif // CZ_AKCORETEXTEDITOR_H
