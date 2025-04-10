#include <Marco/MApplication.h>
#include <Marco/MScreen.h>
#include <Marco/roles/MToplevel.h>
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
#include <iostream>

using namespace AK;

class Window : public MToplevel
{
public:
    Window() noexcept : MToplevel() {
        setColorWithAlpha(/*0xffF0F0F0*/0x00FFFFFF);
        setTitle("Hello world!");
        shadow.enableDiminishOpacityOnInactive(true);
        topbar.enableDiminishOpacityOnInactive(true);
        topbar.enableChildrenClipping(false);
        topbar.layout().setPositionType(YGPositionTypeAbsolute);
        topbar.layout().setPosition(YGEdgeTop, 0);
        topbar.layout().setAlignItems(YGAlignCenter);
        topbar.layout().setJustifyContent(YGJustifyCenter);
        topbar.layout().setHeight(32);
        topbar.layout().setWidthPercent(100);
        topbarSpace.layout().setHeight(32);
        auto textStyle = helloWorld.textStyle();
        textStyle.setFontStyle(
            SkFontStyle(SkFontStyle::kExtraBold_Weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant));
        textStyle.setFontSize(14);
        textStyle.setColor(0xb3000000);
        helloWorld.setTextStyle(textStyle);
        helloWorld.enableDiminishOpacityOnInactive(true);
        body.layout().setFlex(1.f);
        body.layout().setAlignItems(YGAlignCenter);
        body.layout().setJustifyContent(YGJustifyCenter);
        body.layout().setPadding(YGEdgeAll, 48.f);
        body.layout().setPadding(YGEdgeTop, 24.f);
        body.layout().setGap(YGGutterAll, 8.f);
        cat.layout().setMargin(YGEdgeAll, 12.f);
        cat.layout().setMinWidth(140);
        cat.layout().setMinHeight(140);
        cat.layout().setWidthPercent(100);
        cat.layout().setFlex(1.f);
        cat.setSizeMode(AKImageFrame::SizeMode::Contain);
        newWindowButton.setBackgroundColor(AKTheme::SystemBlue);
        exitButton.setBackgroundColor(AKTheme::SystemRed);

        disabledButton.setEnabled(false);

        cursorButton.on.clicked.subscribe(this, [this](){
            if (cursor == 34)
                cursor = 0;
            else
                cursor++;

            cursorButton.setText(std::string("🖱️ Cursor: ") + cursorToString((AKCursor)cursor));
            pointer().setCursor((AKCursor)cursor);
        });

        newWindowButton.on.clicked.subscribe(this, [this](){
            Window *newWin = new Window();
            newWin->setMapped(true);
            AKWeak<MToplevel> ref(this);
            newWin->onMappedChanged.subscribe(newWin, [ref, newWin](){
                if (ref && newWin->mapped())
                    newWin->setParentToplevel(ref);
            });
        });

        builtinDecorationsButton.on.clicked.subscribe(this, [this](){
            enableBuiltinDecorations(!builtinDecorationsEnabled());
        });

        decorationsButton.on.clicked.subscribe(this, [this]() {
            setDecorationMode(decorationMode() == ClientSide ? ServerSide : ClientSide);
        });

        maximizeButton.on.clicked.subscribe(this, [this](){
           setMaximized(!maximized());
        });

        fullscreenButton.on.clicked.subscribe(this, [this](){
            setFullscreen(!fullscreen());
        });

        minimizeButton.on.clicked.subscribe(this, [this](){
            setMinimized();
        });

        exitButton.on.clicked.subscribe(&exitButton, [](){
            exit(0);
        });

        mapButton.on.clicked.subscribe(this, [this]{
            setMapped(false);
            AKTimer::OneShoot(1000, [this](AKTimer*){
                setMapped(true);
            });
        });

        onDecorationModeChanged.subscribe(this, [this](){
            topbar.setVisible(decorationMode() == ClientSide);
        });

        setMinSize(minContentSize());
    }

    AKContainer topbarSpace { YGFlexDirectionColumn, false, this };
    AKContainer body { YGFlexDirectionColumn, true, this };
    AKImageFrame cat { AKImageLoader::loadFile("/usr/local/share/Kay/assets/logo.png"), &body };
    UInt32 cursor { 1 };
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
    AKTextField textField { &body };
    AKTextField textField2 { &body };
    AKTextField textField3 { &body };

    AKSolidColor topbar { 0xFFFAFAFA, this };
    AKText helloWorld { "🚀 Hello World!", &topbar };
    AKEdgeShadow shadow { &topbar };
};

int main()
{
    setenv("KAY_DEBUG", "4", 1);
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

    window.::MSurface::onCallbackDone.subscribe(&window, [&window](UInt32 ms){
        //window.cat.renderableImage().setOpacity(1.f + 0.5f*SkScalarCos(ms * 0.005f));
        //std::cout << "Presented" << ms << std::endl;
    });
    return app.exec();
}
