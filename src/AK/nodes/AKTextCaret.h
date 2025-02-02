#ifndef AKTEXTCARET_H
#define AKTEXTCARET_H

#include <AK/nodes/AKThreeImagePatch.h>

class AK::AKTextCaret : public AKThreeImagePatch
{
public:
    AKTextCaret(AKNode *parent = nullptr) noexcept;

protected:
    void onSceneBegin() override;
};

#endif // AKTEXTCARET_H
