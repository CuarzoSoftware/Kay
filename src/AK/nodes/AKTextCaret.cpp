#include <AK/nodes/AKTextCaret.h>
#include <AK/AKTheme.h>
#include <AK/AKTarget.h>
#include <AK/AKTime.h>

using namespace AK;

AKTextCaret::AKTextCaret(AKNode *parent) noexcept : AKThreeImagePatch(Vertical, parent)
{
    setAnimated(true);
    SkRegion empty;
    setInputRegion(&empty);
    setSideSrcRect(AKTheme::TextCaretVThreePatchSideSrcRect);
    setCenterSrcRect(AKTheme::TextCaretVThreePatchCenterSrcRect);
    enableCustomTextureColor(true);
    setColorWithAlpha(AKTheme::SystemBlue);

    layout().setWidth(AKTheme::TextCaretVThreePatchSideSrcRect.width());
    layout().setHeight(16);
}

void AKTextCaret::onSceneBegin()
{
    const SkScalar anim { 0.5f + 0.5f * SkScalarCos(0.005f * SkScalar(AKTime::ms())) };
    setOpacity(1.f - SkScalarPow(anim, 5.f));
    AKThreeImagePatch::onSceneBegin();
    setImage(theme()->textCaretVThreePatchImage(currentTarget()));
    setScale(currentTarget()->bakedComponentsScale());
}
