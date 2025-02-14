#include <AK/nodes/AKImageFrame.h>
#include <AK/AKLog.h>

using namespace AK;

AKImageFrame::AKImageFrame(AKNode *parent) noexcept :
    AKContainer(YGFlexDirectionRow, true, parent),
    m_renderableImage(this)
{
    init();
}

AKImageFrame::AKImageFrame(sk_sp<SkImage> image, AKNode *parent) noexcept :
    AKContainer(YGFlexDirectionRow, true, parent),
    m_renderableImage(image, this)
{
    init();
}

void AKImageFrame::init() noexcept
{
    layout().setOverflow(YGOverflowHidden);
    m_renderableImage.layout().setPositionType(YGPositionTypeAbsolute);
    m_renderableImage.layout().setFlex(1.f);
    updateDimensions();
    signalLayoutChanged.subscribe(this, [this](auto changes){
        if (sizeMode() != SizeMode::Fill && changes.check(LayoutChanges::Size))
            updateDimensions();
    });
}

void AKImageFrame::updateDimensions() noexcept
{
    if (!image() ||
        image()->width() <= 0 ||
        image()->height() <= 0 ||
        (sizeMode() != SizeMode::Fill && (layout().calculatedWidth() <= 0.f || layout().calculatedHeight() <= 0.f)))
    {
        m_renderableImage.setVisible(false);
        return;
    }

    m_renderableImage.setVisible(true);

    SkRect srcRect { 0.f, 0.f, 0.f, 0.f };

    if (srcRectMode() == AKRenderableImage::SrcRectMode::Custom)
    {
        if (customSrcRect().isEmpty())
            return;

        srcRect = customSrcRect();
    }
    else
    {
        if (AK::is90Transform(srcTransform()))
            srcRect.setWH(image()->height(), image()->width());
        else
            srcRect.setWH(image()->width(), image()->height());
    }

    if (sizeMode() == SizeMode::Fill)
    {
        layout().setAlignItems(YGAlignFlexStart);
        layout().setJustifyContent(YGJustifyFlexStart);
        m_renderableImage.layout().setAspectRatio(YGUndefined);
        m_renderableImage.layout().setPosition(YGEdgeLeft, 0.f);
        m_renderableImage.layout().setPosition(YGEdgeTop, 0.f);
        m_renderableImage.layout().setWidthPercent(100.f);
        m_renderableImage.layout().setHeightPercent(100.f);
    }
    else if (sizeMode() == SizeMode::Contain)
    {
        const Float32 aspectRatio { layout().calculatedWidth() / layout().calculatedHeight() };
        m_renderableImage.layout().setAspectRatio(srcRect.width()/srcRect.height());

        if (aspectRatio >= m_renderableImage.layout().aspectRatio())
        {
            m_renderableImage.layout().setHeightPercent(100.f);
            m_renderableImage.layout().setWidthAuto();
        }
        else
        {
            m_renderableImage.layout().setWidthPercent(100.f);
            m_renderableImage.layout().setHeightAuto();
        }
    }
    else // Cover
    {
        const Float32 aspectRatio { layout().calculatedWidth() / layout().calculatedHeight() };
        m_renderableImage.layout().setAspectRatio(srcRect.width()/srcRect.height());

        if (aspectRatio < m_renderableImage.layout().aspectRatio())
        {
            m_renderableImage.layout().setHeightPercent(100.f);
            m_renderableImage.layout().setWidthAuto();
        }
        else
        {
            m_renderableImage.layout().setWidthPercent(100.f);
            m_renderableImage.layout().setHeightAuto();
        }
    }

    // Alignment
    if (sizeMode() != SizeMode::Fill)
    {
        const AKBitset<AKAlignment> alignment { m_alignment };

        m_renderableImage.layout().setPosition(YGEdgeLeft, YGUndefined);
        m_renderableImage.layout().setPosition(YGEdgeTop, YGUndefined);

        // X-axis
        if (alignment.checkAll(AKAlignLeft | AKAlignRight) || !alignment.check(AKAlignLeft | AKAlignRight))
            layout().setJustifyContent(YGJustifyCenter);
        else if (alignment.check(AKAlignLeft))
            layout().setJustifyContent(YGJustifyFlexStart);
        else if (alignment.check(AKAlignRight))
            layout().setJustifyContent(YGJustifyFlexEnd);

        // Y-axis
        if (alignment.checkAll(AKAlignTop | AKAlignBottom) || !alignment.check(AKAlignTop | AKAlignBottom))
            layout().setAlignItems(YGAlignCenter);
        else if (alignment.check(AKAlignTop))
            layout().setAlignItems(YGAlignFlexStart);
        else if (alignment.check(AKAlignBottom))
            layout().setAlignItems(YGAlignFlexEnd);

        layout().calculate();
    }        
}
