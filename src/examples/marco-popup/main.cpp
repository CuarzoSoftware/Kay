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
#include <AK/events/AKPointerButtonEvent.h>
#include <AK/AKTheme.h>
#include <AK/AKLog.h>

using namespace AK;

static const std::vector<std::string> anchorGravityNames {
    "None",
    "Top",
    "Bottom",
    "Left",
    "Right",
    "TopLeft",
    "BottomLeft",
    "TopRight",
    "BottomRight"
};

class Menu: public MPopup
{
public:
    Menu(MSurface *parent, bool useGrab) noexcept : MPopup(), useGrab(useGrab)
    {
        setColorWithoutAlpha(SkColorSetARGB(rand()%255, rand()%255, rand()%255, rand()%255));
        setParent(parent);
        setAnchor(parent->role() == Role::Toplevel ? Anchor::TopLeft : Anchor::BottomRight);
        layout().setGap(YGGutterAll, 16.f);
        layout().setPadding(YGEdgeAll, 16.f);
        const SkISize contentSize { minContentSize() };
        layout().setWidth(contentSize.width());
        layout().setHeight(contentSize.height());

        if (useGrab)
            setGrab(&app()->pointer().eventHistory().button);

        closeButton.on.clicked.subscribe(this, [this](){ setMapped(false); });
        showButton.on.clicked.subscribe(this, [this](){
            if (!subMenu)
                subMenu = std::make_unique<Menu>(this, this->useGrab);
            subMenu->setMapped(true);
        });

        anchorButton.on.clicked.subscribe(this, [this](){
            UInt32 a = (UInt32)anchor();
            if (a == 8) a = 0;
            else a++;
            setAnchor((Anchor)a);
            anchorButton.setText("Anchor: " + anchorGravityNames[a]);
        });

        gravityButton.on.clicked.subscribe(this, [this](){
            UInt32 g = (UInt32)gravity();
            if (g == 8) g = 0;
            else g++;
            setGravity((Gravity)g);
            gravityButton.setText("Gravity: " + anchorGravityNames[g]);
        });
    }

    ~Menu() { AKLog::debug("Popup destroyed"); }

    AKButton showButton { "Add sub popup", this };
    AKButton anchorButton { "Change anchor: TopLeft", this };
    AKButton gravityButton { "Change gravity: BottomRight", this };
    AKButton closeButton { "Close", this };
    std::unique_ptr<Menu> subMenu;
    bool useGrab;
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

        multiButton.on.clicked.subscribe(this, [this](){
            Menu *menu = new Menu(this, false);
            menu->onMappedChanged.subscribe(menu, [menu](){
                if (!menu->mapped())
                    menu->destroyLater();
            });
            menu->setMapped(true);
            menu->showButton.on.clicked.notify();
            menu->subMenu->showButton.on.clicked.notify();
            menu->subMenu->subMenu->showButton.on.clicked.notify();
        });

        exitButton.on.clicked.subscribe(this, [](){ exit(0); });
    }

    AKButton showButton { "Show popup", this};
    AKButton multiButton { "Show 4 popups at once", this};
    AKButton exitButton { "Exit", this };
    Menu menu { this , true };
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
