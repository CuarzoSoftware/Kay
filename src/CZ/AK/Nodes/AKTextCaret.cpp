#include <CZ/AK/Nodes/AKTextCaret.h>
#include <CZ/Core/Events/CZLayoutEvent.h>
#include <CZ/AK/AKTheme.h>
#include <CZ/AK/AKTarget.h>

using namespace CZ;

AKTextCaret::AKTextCaret(AKNode *parent) noexcept : AKThreePatch(CZOrientation::V, parent)
{
    SkRegion empty;
    setInputRegion(&empty);
    setSideSrcRect(AKTheme::TextCaretVThreePatchSideSrcRect);
    setCenterSrcRect(AKTheme::TextCaretVThreePatchCenterSrcRect);
    enableReplaceImageColor(true);
    setColor(AKTheme::SystemBlue);
    layout().setWidth(AKTheme::TextCaretVThreePatchSideSrcRect.width());
    layout().setHeight(16);
    updateDimensions();

    m_blinkAnimation.setDuration(1200);

    m_blinkAnimation.setOnUpdateCallback([this](CZAnimation *anim){
        setOpacity((1.f + SkScalarCos(anim->value() * M_PI * 2.f)) * 0.5f);
    });

    m_blinkAnimation.setOnFinishCallback([this](CZAnimation *anim){
        if (animated())
            anim->start();
    });
}

void AKTextCaret::setAnimated(bool enabled) noexcept
{
    if (enabled == animated())
        return;

    m_animated = enabled;

    if (animated())
        m_blinkAnimation.start();
    else
    {
        m_blinkAnimation.stop();
        setOpacity(1.f);
    }
}

void AKTextCaret::layoutEvent(const CZLayoutEvent &event)
{
    AKThreePatch::layoutEvent(event);

    if (event.changes.has(CZLayoutChangeScale))
        updateDimensions();

    event.accept();
}

void AKTextCaret::updateDimensions() noexcept
{
    setImage(theme()->textCaretVThreePatchImage(scale()));
    setImageScale(scale());
}
