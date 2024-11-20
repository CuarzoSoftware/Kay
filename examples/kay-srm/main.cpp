#include <yoga/Yoga.h>
#include <iostream>

int main()
{
    YGConfigRef config = YGConfigNew();
    YGConfigSetPointScaleFactor(config, 2.0f);
    YGNodeRef root = YGNodeNewWithConfig(config);

    YGNodeRef child = YGNodeNew();
    YGNodeInsertChild(root, child, 0);
    YGNodeStyleSetWidthPercent(child, 50.f);

    YGNodeCalculateLayout(root, 1000.f, 1000.f, YGDirectionLTR);
    std::cout << YGNodeLayoutGetWidth(child);
    return 0;
}
