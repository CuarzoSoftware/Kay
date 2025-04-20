#ifndef AKBACKGROUNDDAMAGETRACKER_H
#define AKBACKGROUNDDAMAGETRACKER_H

#include <AK/AKObject.h>
#include <include/core/SkRegion.h>

class AK::AKBackgroundDamageTracker : public AKObject
{
public:
    bool enabled { false };
    AKNode *node;
    SkIRect reactiveRect;
    SkIRect reactiveRectTranslated;
    SkRegion repaintAnyway;
    SkRegion repaintAnywayTranslated;
    SkRegion damage;
    std::unordered_map<AKSceneTarget*,std::shared_ptr<AKSurface>> surfaces;
    std::shared_ptr<AKSurface> currentSurface;
    SkScalar q { 0.125f * 1.f };
    Int32 r { 0 };
    Int32 divisibleBy { 1 };
};

#endif // AKBACKGROUNDDAMAGETRACKER_H
