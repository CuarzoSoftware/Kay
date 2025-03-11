#include <AK/nodes/AKTextCaret.h>
#include <AK/events/AKLayoutEvent.h>
#include <AK/AKTheme.h>
#include <AK/AKTarget.h>
#include <AK/AKTime.h>

using namespace AK;

AKTextCaret::AKTextCaret(AKNode *parent) noexcept : AKThreeImagePatch(Vertical, parent)
{
    SkRegion empty;
    setInputRegion(&empty);
    setSideSrcRect(AKTheme::TextCaretVThreePatchSideSrcRect);
    setCenterSrcRect(AKTheme::TextCaretVThreePatchCenterSrcRect);
    enableCustomTextureColor(true);
    setColorWithAlpha(AKTheme::SystemBlue);
    layout().setWidth(AKTheme::TextCaretVThreePatchSideSrcRect.width());
    layout().setHeight(16);
    updateDimensions();

    m_blinkAnimation.setDuration(1200);

    m_blinkAnimation.setOnUpdateCallback([this](AKAnimation *anim){
        setOpacity((1.f + SkScalarCos(anim->value() * M_PI * 2.f)) * 0.5f);
    });

    m_blinkAnimation.setOnFinishCallback([this](AKAnimation *anim){
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

void AKTextCaret::layoutEvent(const AKLayoutEvent &event)
{
    AKThreeImagePatch::layoutEvent(event);

    if (event.changes().check(AKLayoutEvent::Changes::Scale))
        updateDimensions();

    event.accept();
}

void AKTextCaret::updateDimensions() noexcept
{
    setImage(theme()->textCaretVThreePatchImage(scale()));
    setImageScale(scale());
}
