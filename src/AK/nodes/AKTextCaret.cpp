#include <AK/nodes/AKTextCaret.h>
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

    signalLayoutChanged.subscribe(this, [this](auto changes){
        if (changes.check(LayoutChanges::Scale))
            updateDimensions();
    });
}

void AKTextCaret::setAnimated(bool enabled) noexcept
{
    if (enabled == animated())
        return;

    AKNode::setAnimated(enabled);

    if (animated())
    {
        setOpacity(0.f);
        m_animStartMs = AKTime::ms();
    }
    else
        setOpacity(1.f);
}

void AKTextCaret::updateDimensions() noexcept
{
    setImage(theme()->textCaretVThreePatchImage(scale()));
    setScale(scale());
}

void AKTextCaret::onSceneBegin()
{
    if (animated())
    {
        const SkScalar anim { 0.5f + 0.5f * SkScalarCos(0.005f * SkScalar(AKTime::ms() - m_animStartMs)) };
        setOpacity(1.f - SkScalarPow(anim, 5.f));
    }
    AKThreeImagePatch::onSceneBegin();
}
