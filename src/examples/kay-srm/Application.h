#ifndef APPLICATION_H
#define APPLICATION_H

#include <AK/AKApplication.h>
#include <AK/AKScene.h>
#include <AK/utils/AKImageLoader.h>
#include <AK/nodes/AKContainer.h>

#include <SRM/SRMCore.h>

namespace AK
{
class Application : public AKApplication
{
public:
    Application() noexcept;
    ~Application() noexcept;

    void arrangeScreens() noexcept;
    void finish() noexcept;

    SRMCore *core { nullptr };
    AKScene scene;
    AKContainer root;
};

inline static Application *app() noexcept { return (Application*)akApp(); }
}

#endif // APPLICATION_H
