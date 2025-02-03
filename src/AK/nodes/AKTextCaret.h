#ifndef AKTEXTCARET_H
#define AKTEXTCARET_H

#include <AK/nodes/AKThreeImagePatch.h>

class AK::AKTextCaret : public AKThreeImagePatch
{
public:
    AKTextCaret(AKNode *parent = nullptr) noexcept;

    void setAnimated(bool enabled) noexcept;
protected:
    void onSceneBegin() override;
    UInt32 m_animStartMs { 0 };
};

#endif // AKTEXTCARET_H
