#ifndef APPLICATION_H
#define APPLICATION_H

#include <CZ/AK/AKApp.h>
#include <CZ/AK/AKScene.h>
#include <CZ/AK/Utils/AKImageLoader.h>
#include <CZ/AK/Nodes/AKContainer.h>

#include <SRM/SRMCore.h>

namespace CZ
{
class Application : public AKApp
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

inline static Application *app() noexcept { return (Application*)AKApp::Get(); }
}

#endif // APPLICATION_H
