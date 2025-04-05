#include <Marco/MApplication.h>
#include <Marco/MScreen.h>
#include <Marco/roles/MToplevel.h>
#include <Marco/roles/MPopup.h>
#include <AK/utils/AKImageLoader.h>
#include <AK/nodes/AKContainer.h>
#include <AK/nodes/AKText.h>
#include <AK/nodes/AKButton.h>
#include <AK/nodes/AKSolidColor.h>
#include <AK/nodes/AKImageFrame.h>
#include <AK/nodes/AKTextField.h>
#include <AK/effects/AKEdgeShadow.h>
#include <AK/AKTheme.h>
#include <AK/AKLog.h>

using namespace AK;

class Menu: public MPopup
{
public:
    Menu(MSurface *parent) noexcept : MPopup()
    {
        setColorWithoutAlpha(SkColorSetARGB(rand()%255, rand()%255, rand()%255, rand()%255));
        setParent(parent);
        setAnchor(parent->role() == Role::Toplevel ? Anchor::TopLeft : Anchor::BottomRight);
        layout().setGap(YGGutterAll, 16.f);
        layout().setPadding(YGEdgeAll, 16.f);
        const SkISize contentSize { minContentSize() };
        layout().setWidth(contentSize.width());
        layout().setHeight(contentSize.height());

        closeButton.on.clicked.subscribe(this, [this](){ setMapped(false); });
        showButton.on.clicked.subscribe(this, [this](){
            if (!subMenu)
                subMenu = std::make_unique<Menu>(this);

            subMenu->setMapped(true);
        });
    }

    AKButton showButton { "Add sub popup", this };
    AKButton closeButton { "Close", this };
    std::unique_ptr<Menu> subMenu;
};

class Window : public MToplevel
{
public:
    Window() noexcept : MToplevel()
    {
        setTitle("Popup Example");
        layout().setGap(YGGutterAll, 16.f);
        layout().setPadding(YGEdgeAll, 16.f);
        layout().setJustifyContent(YGJustifySpaceEvenly);
        setMinSize(SkISize(400, 400));

        showButton.on.clicked.subscribe(this, [this](){
            menu.setMapped(true);
        });
        exitButton.on.clicked.subscribe(this, [](){ exit(0); });
    }

    AKButton showButton { "Show popup", this};
    AKButton exitButton { "Exit", this };
    Menu menu { this };
};

int main()
{
    setenv("WAYLAND_DISPLAY", "wayland-0", 0);
    setenv("KAY_DEBUG", "4", 1);
    MApplication app;
    app.setAppId("org.Cuarzo.marco-popup");
    Window window;
    window.setMapped(true);
    return app.exec();
}
