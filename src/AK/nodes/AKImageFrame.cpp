#include <AK/nodes/AKImageFrame.h>

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
    m_renderableImage.layout().setPositionType(YGPositionTypeRelative);

    signalLayoutChanged.subscribe(this, [this](auto changes){
        if (changes.check(AKNode::LayoutChanges::Size))
            updateDimensions();
    });

    updateDimensions();
}

void AKImageFrame::updateDimensions() noexcept
{
    if (!image() || image()->width() <= 0 || image()->height() <= 0 || layout().calculatedWidth() <= 0.f || layout().calculatedHeight() <= 0.f)
    {
        m_renderableImage.setVisible(false);
        return;
    }

    m_renderableImage.setVisible(true);

    SkISize dstSize { 0, 0 };
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
        m_renderableImage.layout().setPosition(YGEdgeLeft, 0.f);
        m_renderableImage.layout().setPosition(YGEdgeTop, 0.f);
        m_renderableImage.layout().setWidthPercent(100.f);
        m_renderableImage.layout().setHeightPercent(100.f);
    }
    else if (sizeMode() == SizeMode::Contain)
    {
        dstSize.fWidth = (srcRect.width() * layout().calculatedHeight()) / srcRect.height();

        if (dstSize.fWidth > layout().calculatedWidth())
        {
            dstSize.fWidth = layout().calculatedWidth();
            dstSize.fHeight = (srcRect.height() * layout().calculatedWidth()) /srcRect.width();
        }
        else
            dstSize.fHeight = layout().calculatedHeight();
    }
    else // Cover
    {
        dstSize.fWidth = (srcRect.width() * layout().calculatedHeight()) / srcRect.height();

        if (dstSize.fWidth < layout().calculatedWidth())
        {
            dstSize.fWidth = layout().calculatedWidth();
            dstSize.fHeight = (srcRect.height() * layout().calculatedWidth()) / srcRect.width();
        }
        else
            dstSize.fHeight = layout().calculatedHeight();
    }

    // Alignment
    if (sizeMode() != SizeMode::Fill)
    {
        const AKBitset<AKAlignment> alignment { m_alignment };

        m_renderableImage.layout().setWidth(dstSize.fWidth);
        m_renderableImage.layout().setHeight(dstSize.fHeight);

        // X-axis
        if (alignment.checkAll(AKAlignLeft | AKAlignRight) || !alignment.check(AKAlignLeft | AKAlignRight))
            m_renderableImage.layout().setPosition(YGEdgeLeft, (layout().calculatedWidth() - dstSize.width()) / 2);
        else if (alignment.check(AKAlignLeft))
            m_renderableImage.layout().setPosition(YGEdgeLeft, 0.f);
        else if (alignment.check(AKAlignRight))
            m_renderableImage.layout().setPosition(YGEdgeLeft, layout().calculatedWidth() - dstSize.width());

        // Y-axis
        if (alignment.checkAll(AKAlignTop | AKAlignBottom) || !alignment.check(AKAlignTop | AKAlignBottom))
            m_renderableImage.layout().setPosition(YGEdgeTop, (layout().calculatedHeight() - dstSize.height()) / 2);
        else if (alignment.check(AKAlignTop))
            m_renderableImage.layout().setPosition(YGEdgeTop, 0.f);
        else if (alignment.check(AKAlignBottom))
            m_renderableImage.layout().setPosition(YGEdgeTop, layout().calculatedHeight() - dstSize.height());
    }

    m_renderableImage.layout().apply();
}
