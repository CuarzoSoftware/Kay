#ifndef SCREENDATA_H
#define SCREENDATA_H

#include <Application.h>
#include <ProgressBar.h>
#include <AK/AKScene.h>
#include <AK/nodes/AKContainer.h>
#include <AK/nodes/AKImageFrame.h>
#include <AK/nodes/AKText.h>

#include <SRM/SRMConnector.h>

using namespace AK;

struct Screen : public AKContainer
{
    Screen(SRMConnector *connector) noexcept;
    ~Screen() noexcept;

    void updateDimensions() noexcept;
    void updateTarget() noexcept;

    AKSceneTarget *target { nullptr };
    SRMConnector *connector { nullptr };
    AKImageFrame logo { this };
    ProgressBar progressBar { this };
    AKText emoji { "👻 Boo!!\nPress CTRL+C" , this };
};

#endif // SCREENDATA_H
