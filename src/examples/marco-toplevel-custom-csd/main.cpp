#include <CZ/AK/AKLog.h>
#include <CZ/Marco/MApplication.h>
#include <CZ/Marco/roles/MToplevel.h>
#include <CZ/AK/Nodes/AKButton.h>
#include <CZ/Events/CZLayoutEvent.h>
#include <CZ/AK/Utils/AKImageLoader.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKAnimation.h>

using namespace CZ;

class Window : public MToplevel
{
public:
    Window()
    {
        for (size_t i = 0; i < 4; i++)
        {
            customDecorationNodes.emplace_back(std::make_unique<AKSolidColor>(AKTheme::SystemBlue, this));

            // Prevents being affected by flex
            customDecorationNodes.back()->layout().setPositionType(YGPositionTypeAbsolute);
        }

        exitBtn.onClick.subscribe(this, [](const auto &){
            exit(0);
        });

        toggleBuiltinDecorationsBtn.onClick.subscribe(this, [this](const auto &){
            enableBuiltinDecorations(!builtinDecorationsEnabled());
            updateDecorations();
            enableChildrenClipping(builtinDecorationsEnabled());

            if (builtinDecorationsEnabled())
                decorationsAnimation.stop();
            else
                decorationsAnimation.start();
        });

        decorationsAnimation.setDuration(5000);
        decorationsAnimation.setOnUpdateCallback([this](AKAnimation *anim){
            const SkScalar color { SkScalarSin(M_PI * anim->value()) };

            for (auto &deco : customDecorationNodes)
                deco->setColorWithAlpha({ 1.f - color, color, SkScalarPow(color, 4.f), 0.72f });
        });

        decorationsAnimation.setOnFinishCallback([this](AKAnimation *anim){
            if (!builtinDecorationsEnabled())
                anim->start();
        });

        onStatesChanged.subscribe(this, [this](const CZWindowStateEvent &){

            if (activated())
                setDecorationMargins({32, 32, 32, 32});
            else
                setDecorationMargins({16, 16, 16, 16});
        });

        enableChildrenClipping(false);
        setDecorationMargins({32, 32, 32, 32});
        layout().setPadding(YGEdgeAll, 32.f);
        layout().setJustifyContent(YGJustifySpaceEvenly);
        setMinSize(minContentSize());
        layout().setWidth(500);
        layout().setHeight(500);
        enableBuiltinDecorations(false);
        decorationsAnimation.start();
    };

    // Decorations
    std::vector<std::unique_ptr<AKSolidColor>> customDecorationNodes;
    AKAnimation decorationsAnimation;

    AKButton toggleBuiltinDecorationsBtn { "Toggle built-in decorations", this };
    AKButton exitBtn { "Exit", this };

    // Configure the decoration nodes
    void updateDecorations() noexcept
    {
        if (decorationMode() == ClientSide && !builtinDecorationsEnabled())
        {
            // L
            customDecorationNodes[0]->layout().setPosition(YGEdgeLeft, -decorationMargins().left());
            customDecorationNodes[0]->layout().setPosition(YGEdgeTop, 0.f);
            customDecorationNodes[0]->layout().setWidth(decorationMargins().left());
            customDecorationNodes[0]->layout().setHeightPercent(100.f);

            // T
            customDecorationNodes[1]->layout().setPosition(YGEdgeLeft, -decorationMargins().left());
            customDecorationNodes[1]->layout().setPosition(YGEdgeTop, -decorationMargins().top());
            customDecorationNodes[1]->layout().setWidth(decorationMargins().left() + globalRect().width() + decorationMargins().right());
            customDecorationNodes[1]->layout().setHeight(decorationMargins().top());

            // R
            customDecorationNodes[2]->layout().setPosition(YGEdgeRight, -decorationMargins().right());
            customDecorationNodes[2]->layout().setPosition(YGEdgeTop, 0.f);
            customDecorationNodes[2]->layout().setWidth(decorationMargins().right());
            customDecorationNodes[2]->layout().setHeightPercent(100.f);

            // B
            customDecorationNodes[3]->layout().setPosition(YGEdgeLeft, -decorationMargins().left());
            customDecorationNodes[3]->layout().setPosition(YGEdgeBottom, -decorationMargins().bottom());
            customDecorationNodes[3]->layout().setWidth(decorationMargins().left() + globalRect().width() + decorationMargins().right());
            customDecorationNodes[3]->layout().setHeight(decorationMargins().bottom());

            // This needs to be called whenever you change the layout
            // of nodes in response to a layoutEvent otherwise the scene
            // won't know the right dimensions during rendering
            layout().calculate();

            for (auto &node : customDecorationNodes)
                node->setVisible(true);
        }
        else // Using built-in decorations
        {
            for (auto &node : customDecorationNodes)
                node->setVisible(false);
        }
    }

    // Detect when the window's central node size changes
    void layoutEvent(const CZLayoutEvent &e) override
    {
        MToplevel::layoutEvent(e);

        if (e.changes().has(CZLayoutEvent::Size))
            updateDecorations();
    }

    // Triggered after calling setDecorationMargins()
    void decorationMarginsChanged() override
    {
        MToplevel::decorationMarginsChanged();
        updateDecorations();
    }
};

int main()
{
    setenv("KAY_DEBUG", "4", 1);

    MApplication app;
    app.setAppId("org.Cuarzo.marco-toplevel-custom-csd");

    Window window;
    window.setMapped(true);

    return app.exec();
}
