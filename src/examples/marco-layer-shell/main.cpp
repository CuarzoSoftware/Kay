#include <Marco/roles/MSubsurface.h>
#include <Marco/MApplication.h>
#include <Marco/MScreen.h>
#include <Marco/roles/MLayerSurface.h>
#include <Marco/roles/MSubsurface.h>
#include <Marco/roles/MPopup.h>
#include <AK/nodes/AKText.h>
#include <AK/nodes/AKButton.h>
#include <AK/AKLog.h>
#include <AK/nodes/AKImageFrame.h>
#include <AK/utils/AKImageLoader.h>
#include <AK/AKAnimation.h>
#include <AK/AKGLContext.h>
#include <AK/events/AKWindowCloseEvent.h>
#include <AK/events/AKPointerButtonEvent.h>

using namespace AK;

static const std::vector<Int32> exclusiveZones { 0, -1, 200, 300, 400, 500, 600, 700, 800 };
static size_t exclusiveZoneI { 0 };
static const std::vector<std::string> layerNames { "Background", "Bottom", "Top", "Overlay" };

class Window : public MLayerSurface
{
public:
    Window() noexcept
        : MLayerSurface()
    {

        onBeforeClose.subscribe(this, [this](const AKWindowCloseEvent &e){
            e.ignore(); // Prevent Marco from unmapping it
            AKLog::warning("The server sent a close event.");
            pickAnotherScreen();
        });

        anchorLButton.onClick.subscribe(this, [this](const auto &){
            auto tmpAnchor { anchor() };
            tmpAnchor.setFlag(AKEdgeLeft, !anchor().check(AKEdgeLeft));
            setAnchor(tmpAnchor);
            anchorLButton.setText(std::string("Anchor L: ") + (anchor().check(AKEdgeLeft) ? "ON" : "OFF"));

            if (anchor().checkAll(AKEdgeLeft | AKEdgeRight))
                requestAvailableWidth();
            else
                layout().setWidthAuto();
        });

        anchorTButton.onClick.subscribe(this, [this](const auto &){
            auto tmpAnchor { anchor() };
            tmpAnchor.setFlag(AKEdgeTop, !anchor().check(AKEdgeTop));
            setAnchor(tmpAnchor);
            anchorTButton.setText(std::string("Anchor T: ") + (anchor().check(AKEdgeTop) ? "ON" : "OFF"));

            if (anchor().checkAll(AKEdgeTop | AKEdgeBottom))
                requestAvailableHeight();
            else
                layout().setHeightAuto();
        });

        anchorBButton.onClick.subscribe(this, [this](const auto &){
            auto tmpAnchor { anchor() };
            tmpAnchor.setFlag(AKEdgeBottom, !anchor().check(AKEdgeBottom));
            setAnchor(tmpAnchor);
            anchorBButton.setText(std::string("Anchor B: ") + (anchor().check(AKEdgeBottom) ? "ON" : "OFF"));

            if (anchor().checkAll(AKEdgeTop | AKEdgeBottom))
                requestAvailableHeight();
            else
                layout().setHeightAuto();
        });

        anchorRButton.onClick.subscribe(this, [this](const auto &){
            auto tmpAnchor { anchor() };
            tmpAnchor.setFlag(AKEdgeRight, !anchor().check(AKEdgeRight));
            setAnchor(tmpAnchor);
            anchorRButton.setText(std::string("Anchor R: ") + (anchor().check(AKEdgeRight) ? "ON" : "OFF"));

            if (anchor().checkAll(AKEdgeLeft | AKEdgeRight))
                requestAvailableWidth();
            else
                layout().setWidthAuto();
        });

        marginAnim.setOnUpdateCallback([this](AKAnimation *a){
            const Float64 phase { a->value() * M_PI * 2.f };
            setMargin(SkIRect::MakeLTRB(
                SkScalarCos(phase) * 100 - 100,
                SkScalarSin(phase) * 100,
                0, 0));
        });

        marginAnim.setOnFinishCallback([this](auto *){
            setMargin({0, 0, 0, 0});
        });

        marginAnim.setDuration(2000);

        animateMargin.onClick.subscribe(this, [this](const auto &){
            if (!marginAnim.running())
                marginAnim.start();
        });

        toggleScope.onClick.subscribe(this, [this](const auto &){
            setScope(scope() == "A" ? "B" : "A");
            toggleScope.setText("Scope: " + scope());
            requestAvailableWidth();
            requestAvailableHeight();
        });

        exclusiveZoneBtn.onClick.subscribe(this, [this](const auto &){
            if (exclusiveZoneI == exclusiveZones.size() - 1)
                exclusiveZoneI = 0;
            else
                exclusiveZoneI++;

            setExclusiveZone(exclusiveZones[exclusiveZoneI]);
            requestAvailableWidth();
            requestAvailableHeight();
            exclusiveZoneBtn.setText(std::string("Vary Exclusive Zone: ") + std::to_string(exclusiveZones[exclusiveZoneI]));
        });

        changeScreen.onClick.subscribe(this, [this](const auto &){
            pickAnotherScreen();
        });

        changeLayerBtn.onClick.subscribe(this, [this](const auto &){
            if (layer() == Layer::Overlay)
                setLayer(Layer::Background);
            else
                setLayer((Layer)(layer() + 1));

            changeLayerBtn.setText(std::string("Change Layer: ") + layerNames[layer()]);
        });

        showPopupBtn.onClick.subscribe(this, [this](const auto &e){
            popup.setGrab(&e);
            popup.setMapped(true);
        });

        unmapBtn.onClick.subscribe(this, [this](const auto &){
            setMapped(false);

            AKTimer::OneShot(1000, [this](AKTimer*){
                setMapped(true);
            });
        });

        exitButton.onClick.subscribe(this, [](const auto &){
            exit(0);
        });

        onLeftScreen.subscribe(this, [this](MScreen &s){
            if (&s == screen())
                pickAnotherScreen();
        });

        setLayer(MLayerSurface::Overlay);
        setAnchor(AKEdgeLeft | AKEdgeTop | AKEdgeRight | AKEdgeBottom);
        setExclusiveZone(exclusiveZones[exclusiveZoneI]);
        setScreen(nullptr);
        setScope("A");

        anchorContainer.layout().setGap(YGGutterAll, 16.f);
        anchorContainer.layout().setJustifyContent(YGJustifySpaceBetween);

        layout().setPadding(YGEdgeAll, 16.f);
        layout().setGap(YGGutterAll, 16.f);
        requestAvailableWidth();
        requestAvailableHeight();
        setMapped(true);

        subsurface.layout().setPadding(YGEdgeAll, 4.f);
        subsurface.setMapped(true);
        subsurface.setPos(0, -30);

        popup.layout().setPadding(YGEdgeAll, 4.f);
        popup.setParent(this);
        popup.setOffset(0, -30);
        popup.setAnchorRect({-1, -1, -1, -1});
        popup.setAnchor(MPopup::Anchor::TopRight);
        popup.setGravity(MPopup::Gravity::BottomLeft);
        popup.setConstraintAdjustment(MPopup::SlideY | MPopup::SlideX);
    }

    void pickAnotherScreen() noexcept
    {
        for (MScreen *s : app()->screens())
        {
            if (s != screen())
            {
                setScreen(s);
                changeScreen.setText("Screen: " + s->props().name);
                requestAvailableWidth();
                requestAvailableHeight();
                break;
            }
        }
    }

    void suggestedSizeChanged() override
    {
        if (suggestedSize().width() != 0)
            layout().setWidth(suggestedSize().width());

        if (suggestedSize().height() != 0)
            layout().setHeight(suggestedSize().height());
    }

    MPopup popup;
    AKText popupText { "I'm a popup!", &popup };

    MSubsurface subsurface { this };
    AKText subsurfaceText { "I'm a subsurface!", &subsurface };

    AKContainer anchorContainer { YGFlexDirectionRow, false, this };
    AKButton anchorLButton { "Anchor L: ON", &anchorContainer };
    AKButton anchorTButton { "Anchor T: ON", &anchorContainer };
    AKButton anchorRButton { "Anchor R: ON", &anchorContainer };
    AKButton anchorBButton { "Anchor B: ON", &anchorContainer };

    AKButton changeLayerBtn { "Change Layer: Overlay", this };

    AKAnimation marginAnim;
    AKButton animateMargin { "Animate Top-Left Margin", this };
    AKButton exclusiveZoneBtn { "Vary Exclusive Zone: 0", this };
    AKButton toggleScope { "Scope: A", this };
    AKButton changeScreen { "Screen: Auto", this };
    AKButton showPopupBtn { "Show Popup", this };
    AKButton unmapBtn { "Unmap for 1 sec", this };
    AKButton exitButton { "Exit", this };
};

int main()
{
    setenv("KAY_DEBUG", "4", 0);
    MApplication app;
    app.setAppId("org.Cuarzo.marco-layer-shell");

    if (app.screens().empty())
    {
        AKLog::fatal("No screens available!");
        exit(1);
    }


    Window window;
    return app.exec();
}
