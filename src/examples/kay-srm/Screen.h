#ifndef SCREENDATA_H
#define SCREENDATA_H

#include <Application.h>
#include <ProgressBar.h>
#include <CZ/AK/AKScene.h>
#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKImageFrame.h>
#include <CZ/AK/Nodes/AKText.h>

#include <SRM/SRMConnector.h>

using namespace CZ;

struct Screen : public AKContainer
{
    Screen(SRMConnector *connector) noexcept;
    ~Screen() noexcept;

    void updateDimensions() noexcept;
    void updateTarget() noexcept;

    AKTarget *target { nullptr };
    SRMConnector *connector { nullptr };
    AKImageFrame logo { this };
    ProgressBar progressBar { this };
    AKText emoji { "👻 Boo!!\nPress CTRL+C" , this };
};

#endif // SCREENDATA_H
