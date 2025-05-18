#include <AK/AKLog.h>
#include <Marco/MApplication.h>
#include <Marco/roles/MToplevel.h>
#include <Marco/roles/MSubsurface.h>
#include <AK/nodes/AKButton.h>
#include <AK/nodes/AKText.h>
#include <AK/AKAnimation.h>

using namespace AK;

class SubWindow : public MSubsurface
{
public:
    SubWindow(MSurface *parent, Int32 size, size_t n) :
        MSubsurface(parent),
        n(n),
        label {std::to_string(n), this }
    {
        auto style = label.textStyle();
        SkPaint p;
        p.setColor(SK_ColorWHITE);
        style.setForegroundColor(p);
        label.setTextStyle(style);
        setColorWithoutAlpha(SkColorSetRGB(rand() % 255, rand() % 255, rand() % 255));
        layout().setWidth(size);
        layout().setHeight(size);
        layout().setJustifyContent(YGJustifyCenter);
        layout().setAlignItems(YGAlignCenter);
        setPos({-size, -size});
        setMapped(true);

        toggleMap.onClick.subscribe(this, [this](const auto &){
            setMapped(false);
            AKTimer::OneShot(1000, [this](AKTimer*){
                setMapped(true);
            });
        });

        addButton.onClick.subscribe(this, [this](const auto &){
            SubWindow *w = new SubWindow(this, surfaceSize().width(), subSurfaces().size() + 1);
            w->addButton.setVisible(false);
            w->setPos(-(surfaceSize().width()/2.5) * w->n, -(surfaceSize().width()/2.5) * w->n + 20);
        });

        upButton.onClick.subscribe(this, [this](const auto &){

            if (this->parent()->subSurfaces().back() == this)
                return;
            placeAbove(*std::next(parentLink()));
        });

        downButton.onClick.subscribe(this, [this](const auto &){

            if (this->parent()->subSurfaces().front() == this)
                return;
            placeBelow(*std::prev(parentLink()));
        });
    }

    size_t n;
    AKText label;
    AKButton toggleMap { "Hide 1 sec", this };
    AKButton addButton { "Add sub", this };
    AKButton upButton { "Up", this };
    AKButton downButton { "Down", this };
};

class Window : public MToplevel
{
public:
    Window() noexcept : MToplevel()
    {
        layout().setPadding(YGEdgeAll, 32.f);
        layout().setGap(YGGutterAll, 32.f);

        addButton.onClick.subscribe(this, [this](const auto &){
            new SubWindow(this, 120, subSurfaces().size());
        });

        animateButton.onClick.subscribe(this, [this](const auto &){
            animated = !animated;

            if (animated)
                spinAnimation.start();
            else
                spinAnimation.stop();
        });

        destroyButton.onClick.subscribe(this, [this](const auto &){
            while (!subSurfaces().empty())
                delete subSurfaces().back();
        });

        exitButton.onClick.subscribe(this, [](const auto &){exit(0);});

        spinAnimation.setDuration(10000);
        spinAnimation.setOnUpdateCallback([this](AKAnimation *a){

            const SkScalar phase =  2.f * M_PI * a->value();
            const SkScalar subN = subSurfaces().size();
            const SkScalar phaseSlice = (2.f * M_PI)/subN;
            const SkSize tlSize (surfaceSize().width(), surfaceSize().height());
            const SkSize tlCenter = { tlSize.width() / 2.f, tlSize.height() / 2.f };
            const SkSize radius (tlCenter.width() + 100.f, tlCenter.height() + 100.f );

            SkScalar x, y;
            for (MSubsurface *subS : subSurfaces())
            {
                SubWindow *s = (SubWindow*)subS;
                x = SkScalarCos(phaseSlice * s->n + phase);
                y = SkScalarSin(phaseSlice * s->n + phase);

                s->setPos(tlCenter.width() + x * radius.width() - s->globalRect().width() / 2,
                          tlCenter.height() + y * radius.height() - s->globalRect().height() / 2);
            }

        });

        spinAnimation.setOnFinishCallback([this](AKAnimation *a) {
            if (animated)
                a->start();
        });

        setMinSize(minContentSize());
    }

    bool animated { false };
    AKAnimation spinAnimation;
    AKButton addButton { "Create subsurface", this };
    AKButton animateButton { "Animate subsurfaces", this };
    AKButton destroyButton { "Destoy subsurfaces", this };
    AKButton exitButton { "Exit", this };
};

int main()
{
    setenv("KAY_DEBUG", "4", 1);
    MApplication app;
    app.setAppId("org.Cuarzo.marco-subsurfaces");

    Window window;
    window.setMapped(true);
    return app.exec();
}
