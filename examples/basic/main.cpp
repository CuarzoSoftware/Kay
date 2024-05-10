#include <AKApplication.h>
#include <AKToplevel.h>
#include <AKRegion.h>
#include <AKFrame.h>
#include <AKLabel.h>

using namespace AK;

int main()
{
    AKApplication app;

    AKToplevel window(AKSize(600, 512));

    AKWidget widget(window.widget());
    widget.setPos(AKPoint(100, 100));
    widget.setSize(AKSize(200, 200));
    widget.setBackgroundColor({1.f, 0.f, 0.f, 1.f});

    AKWidget nestedWidget(&widget);
    nestedWidget.setPos(AKPoint(25, 25));
    nestedWidget.setSize(AKSize(50, 50));
    nestedWidget.setBackgroundColor({0.f, 1.f, 0.f, 1.f});

    AKFrame windowFrameChild1(window.widget());
    windowFrameChild1.setPos(AKPoint(300, 100));
    windowFrameChild1.setBackgroundColor({0.f, 0.f, 1.f, 1.f});
    windowFrameChild1.setSize(AKSize(200, 200));

    AKLabel label("Alan Kay", window.widget());
    label.setPos(AKPoint(30, 30));

    return app.run();
}
