#include <CZ/Marco/MApplication.h>
#include <CZ/Marco/MScreen.h>
#include <CZ/Marco/roles/MToplevel.h>
#include <CZ/Marco/nodes/MVibrancyView.h>
#include <CZ/AK/Utils/AKImageLoader.h>
#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKText.h>
#include <CZ/AK/Nodes/AKButton.h>
#include <CZ/AK/Nodes/AKSolidColor.h>
#include <CZ/AK/Nodes/AKImageFrame.h>
#include <CZ/AK/Nodes/AKTextField.h>
#include <CZ/AK/Nodes/AKWindowButtonGroup.h>
#include <CZ/AK/Nodes/AKScroll.h>
#include <CZ/AK/Effects/AKEdgeShadow.h>
#include <CZ/AK/Effects/AKBackgroundBlurEffect.h>
#include <CZ/AK/Events/AKVibrancyEvent.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKLog.h>
#include <XDGKit/XDGKit.h>
#include <iostream>

using namespace CZ;
using namespace XDG;

static std::shared_ptr<XDGKit> xdg;

class SideMenu : public MVibrancyView
{
public:
    SideMenu(AKNode *parent = nullptr) :
        MVibrancyView(parent)
    {
        userCaps.add(UCWindowMove);
        layout().setMinWidth(200);
        layout().setMaxWidth(600);
        layout().setHeightPercent(100);
        layout().setWidth(250);

        windowButtons.layout().setPositionType(YGPositionTypeAbsolute);
        windowButtons.layout().setPosition(YGEdgeTop, 20.f);
        windowButtons.layout().setPosition(YGEdgeLeft, 20.f);
    }

    void startResize() noexcept
    {
        prevParentMaxWidth = parent()->layout().maxWidth();
        parent()->layout().setMaxWidth(parent()->layout().calculatedWidth());
        prevMaxWidth = layout().maxWidth();
        float prevFlex = layout().flex();
        layout().setFlex(100000.f);
        layout().calculate();
        layout().setMaxWidth(layout().calculatedWidth());
        layout().setFlex(prevFlex);

        resizeStartWidth = globalRect().width();
        resizeStartPointerX = app()->pointer().pos().x();
        resizing = true;
        app()->pointer().setCursor(CZCursorShape::ColResize);
    }

    void updateResize() noexcept
    {
        if (!resizing) return;
        layout().setWidth(
            std::max(0.f,std::min(resizeStartWidth + app()->pointer().pos().x() - resizeStartPointerX, layout().maxWidth().value)));
    }

    void stopResize() noexcept
    {
        if (!resizing) return;
        resizing = false;
        parent()->layout().setMaxWidthYGValue(prevParentMaxWidth);
        layout().setMaxWidthYGValue(prevMaxWidth);
        app()->pointer().setCursor(CZCursorShape::Default);

        /*MToplevel *win { dynamic_cast<MToplevel*>(window()) };
        if (win)
            win->setMinSize(win->minContentSize());*/
    }

    AKWindowButtonGroup windowButtons { this };

    YGValue prevMaxWidth, prevParentMaxWidth;
    Int32 resizeStartWidth;
    Int32 resizeStartPointerX;
    bool resizing { false };
};

class RightContainer : public AKSolidColor
{
public:

    RightContainer(AKNode *parent = nullptr) :
        AKSolidColor(0xFFFFFFFF, parent)
    {
        layout().setMinWidth(250);
        layout().setMinHeight(250);
        layout().setFlex(1.f);
        enableChildrenClipping(false);

        inAppBlur.shader = 3;
        leftShadow.setCursor(CZCursorShape::ColResize);
        leftShadow.enableDiminishOpacityOnInactive(true);
        shadow.enableDiminishOpacityOnInactive(true);
        topbar.enableChildrenClipping(false);
        topbar.layout().setPositionType(YGPositionTypeAbsolute);
        topbar.layout().setPosition(YGEdgeTop, 0);
        topbar.layout().setAlignItems(YGAlignCenter);
        topbar.layout().setJustifyContent(YGJustifyCenter);
        topbar.layout().setHeight(52);
        topbar.layout().setWidthPercent(100);
        topbar.userCaps.add(UCWindowMove);
        auto textStyle = helloWorld.textStyle();
        textStyle.setFontStyle(
            SkFontStyle(SkFontStyle::kExtraBold_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
        textStyle.setFontSize(14);
        textStyle.setColor(0xb3000000);
        helloWorld.setTextStyle(textStyle);
        helloWorld.enableDiminishOpacityOnInactive(true);
        body.slot()->layout().setGap(YGGutterAll, 16.f);
        body.slot()->layout().setPadding(YGEdgeAll, 32.f);
        body.slot()->layout().setPadding(YGEdgeTop, 52.f + 32.f);
        body.verticalBar().layout().setPadding(YGEdgeTop, 54.f);
        /*
        body.slot()->layout().setFlex(1.f);
        body.slot()->layout().setAlignItems(YGAlignCenter);
        body.slot()->layout().setJustifyContent(YGJustifyCenter);
        */
        cat.layout().setMargin(YGEdgeAll, 12.f);
        cat.layout().setMinWidth(120);
        cat.layout().setMinHeight(120);
        cat.layout().setFlex(1.f);
        cat.setSizeMode(AKImageFrame::SizeMode::Contain);
        newWindowButton.setBackgroundColor(AKTheme::SystemBlue);
        exitButton.setBackgroundColor(AKTheme::SystemRed);
        disabledButton.setEnabled(false);
        hiddenButton.layout().setPositionType(YGPositionTypeAbsolute);
        hiddenButton.layout().setPosition(YGEdgeLeft, 1500.f);
        cat.layout().setWidthPercent(100);

        const std::vector<std::string> iconNames {
            "firefox",
            "vscode",
            "spotify",
            "chrome",
            "foot",
            "qtcreator",
            "discord",
            "gimp",
            "gedit"
        };

        for (auto &iconName : iconNames)
        {
            const auto *xdgIcon { xdg->iconThemeManager().findIcon(iconName, 256, 1, XDGIcon::SVG, { "WhiteSur", "" }) };

            if (!xdgIcon)
                continue;

            auto image { AKImageLoader::loadFile(xdgIcon->getPath(XDGIcon::SVG), {256, 256}) };

            for (size_t i = 0; i < 10; i++)
            {
                auto *icon = new AKImageFrame(image, &body);
                icon->layout().setWidthPercent(100);
                icon->layout().setHeight(256);
            }
        }
    }

    AKScroll body { this };

    AKEdgeShadow leftShadow { CZEdgeLeft, this };
    AKImageFrame cat { AKImageLoader::loadFile(akAssetsDir() / "logo.png"), &body };
    AKButton cursorButton { "🖱️ Cursor: Default", &body };
    AKButton builtinDecorationsButton { "Toggle built-in decorations", &body };
    AKButton decorationsButton { "Toggle decoration mode", &body };
    AKButton newWindowButton { "➕  New Child Window", &body };
    AKButton mapButton { "Unmap for 1 sec", &body };
    AKButton maximizeButton { "🖥️ Toggle Maximized", &body };
    AKButton fullscreenButton { "🖥️ Toggle Fullscreen", &body };
    AKButton minimizeButton { "🖥️ Minimize", &body };
    AKButton disabledButton { "🚫 Disabled Button", &body };
    AKButton exitButton { "╰┈➤🚪 Exit", &body };

    AKButton hiddenButton { "Hidden", &body };
    AKTextField textField { &body };
    AKTextField textField2 { &body };
    AKTextField textField3 { &body };

    AKSolidColor topbar { 0xAAFFFFFF, this };
    AKBackgroundBlurEffect inAppBlur { /*&topbar*/ };
    AKText helloWorld { "🚀 Hello World!", &topbar };
    AKEdgeShadow shadow { CZEdgeBottom, &topbar };
};

class Window : public MToplevel
{
public:

    Window() noexcept : MToplevel()
    {
        /*
        auto *t = new AKSolidColor(SK_ColorBLUE, this);
        t->layout().setWidth(200);
        t->layout().setHeight(200);
        t->layout().setPositionType(YGPositionTypeStatic);
        t->layout().setPosition(YGEdgeLeft, -500);*/

        rightContainer.leftShadow.installEventFilter(this);
        layout().setFlexDirection(YGFlexDirectionRow);
        layout().setWidth(800);
        layout().setHeight(600);
        setColorWithAlpha(app()->wayland().backgroundBlurManager ? 0x00FFFFFF : 0xffF0F0F0);
        setTitle("Hello world!");
        //setMinSize(minContentSize());

        rightContainer.cursorButton.onClick.subscribe(this, [this](const auto &){
            if (cursor == 34)
                cursor = 0;
            else
                cursor++;

            rightContainer.cursorButton.setText(std::string("🖱️ Cursor: ") + cursorToString((CZCursorShape)cursor));
            pointer().setCursor((CZCursorShape)cursor);
        });

        rightContainer.newWindowButton.onClick.subscribe(this, [this](const auto &){
            Window *newWin = new Window();
            newWin->setMapped(true);
            CZWeak<MToplevel> ref(this);
            newWin->onMappedChanged.subscribe(newWin, [ref, newWin](){
                if (ref && newWin->mapped())
                    newWin->setParentToplevel(ref);
            });
        });

        rightContainer.builtinDecorationsButton.onClick.subscribe(this, [this](const auto &){
            enableBuiltinDecorations(!builtinDecorationsEnabled());
        });

        rightContainer.decorationsButton.onClick.subscribe(this, [this](const auto &) {
            setDecorationMode(decorationMode() == ClientSide ? ServerSide : ClientSide);
        });

        rightContainer.maximizeButton.onClick.subscribe(this, [this](const auto &){
            setMaximized(!maximized());
        });

        rightContainer.fullscreenButton.onClick.subscribe(this, [this](const auto &){
            setFullscreen(!fullscreen());
        });

        rightContainer.minimizeButton.onClick.subscribe(this, [this](const auto &){
            setMinimized();
        });

        rightContainer.exitButton.onClick.subscribe(&rightContainer.exitButton, [](const auto &){
            exit(0);
        });

        rightContainer.mapButton.onClick.subscribe(this, [this](const auto &){
            setMapped(false);
            CZTimer::OneShot(1000, [this](CZTimer*){
                setMapped(true);
            });
        });

        onDecorationModeChanged.subscribe(this, [this](){
            rightContainer.topbar.setVisible(decorationMode() == ClientSide);
        });
    }

    void windowStateEvent(const CZWindowStateEvent &e) override
    {
        MToplevel::windowStateEvent(e);

        if (activated())
        {
            rightContainer.topbar.addBackgroundEffect(&rightContainer.inAppBlur);
            rightContainer.topbar.setColorWithAlpha(0x00000000);
        }
        else
        {
            rightContainer.topbar.removeBackgroundEffect(&rightContainer.inAppBlur);
            rightContainer.topbar.setColorWithAlpha(0xFFe6e7e7);
        }
    }

    bool eventFilter(const CZEvent &event, AKObject &target) override
    {
        if (&target == &rightContainer.leftShadow)
        {
            if (event.type() == CZEvent::PointerButton)
            {
                const auto &e { static_cast<const CZPointerButtonEvent&>(event) };
                if (e.state() == CZPointerButtonEvent::Pressed)
                {
                    rightContainer.leftShadow.enablePointerGrab(true);
                    leftMenu.startResize();
                }
                else
                {
                    rightContainer.leftShadow.enablePointerGrab(false);
                    leftMenu.stopResize();
                }
            }
            else if (event.type() == CZEvent::PointerMove)
            {
                leftMenu.updateResize();
            }
            else if (event.type() == CZEvent::PointerLeave)
            {
                rightContainer.leftShadow.enablePointerGrab(false);
                leftMenu.stopResize();
            }
        }

        return MToplevel::eventFilter(event, target);
    }

    SideMenu leftMenu { this };
    RightContainer rightContainer { this };
    UInt32 cursor { 1 };
};

int main()
{
    setenv("KAY_DEBUG", "4", 1);
    xdg = XDGKit::Make();
    MApplication app;
    app.setAppId("org.Cuarzo.marco-basic");

    app.onScreenPlugged.subscribe(&app, [](MScreen &screen){
        std::cout << "New Screen! " << screen.props().name << std::endl;
    });

    app.onScreenUnplugged.subscribe(&app, [](MScreen &screen){
        std::cout << "Bye bye Screen! " << screen.props().name << std::endl;
    });

    Window window;
    window.setMapped(true);

    //window.::MSurface::onCallbackDone.subscribe(&window, [&window](UInt32 ms){
        //window.cat.renderableImage().setOpacity(1.f + 0.5f*SkScalarCos(ms * 0.005f));
        //std::cout << "Presented" << ms << std::endl;
    //});
    return app.exec();
}
