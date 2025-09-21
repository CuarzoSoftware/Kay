#include <CZ/AK/Nodes/AKImageFrame.h>
#include <CZ/Events/CZLayoutEvent.h>
#include <CZ/Ream/RImage.h>
#include <CZ/AK/AKLog.h>

using namespace CZ;

AKImageFrame::AKImageFrame(AKNode *parent) noexcept :
    AKContainer(YGFlexDirectionRow, true, parent),
    m_renderableImage(this)
{
    init();
}

AKImageFrame::AKImageFrame(std::shared_ptr<RImage> image, AKNode *parent) noexcept :
    AKContainer(YGFlexDirectionRow, true, parent),
    m_renderableImage(image, this)
{
    init();
}

void AKImageFrame::layoutEvent(const CZLayoutEvent &event)
{
    AKContainer::layoutEvent(event);
    if (sizeMode() != SizeMode::Fill && event.changes.has(CZLayoutChangeSize))
        updateDimensions();
    event.accept();
}

void AKImageFrame::init() noexcept
{
    layout().setOverflow(YGOverflowHidden);
    m_renderableImage.layout().setPositionType(YGPositionTypeAbsolute);
    m_renderableImage.layout().setFlex(1.f);
    updateDimensions();
}

void AKImageFrame::updateDimensions() noexcept
{
    if (!image() ||
        image()->size().isEmpty() ||
        (sizeMode() != SizeMode::Fill && (layout().calculatedWidth() <= 0.f || layout().calculatedHeight() <= 0.f)))
    {
        m_renderableImage.setVisible(false);
        return;
    }

    m_renderableImage.setVisible(true);

    SkRect srcRect { 0.f, 0.f, 0.f, 0.f };

    if (srcRectMode() == AKImage::SrcRectMode::Custom)
    {
        if (customSrcRect().isEmpty())
            return;

        srcRect = customSrcRect();
    }
    else
    {
        if (CZ::Is90Transform(srcTransform()))
            srcRect.setWH(image()->size().height(), image()->size().width());
        else
            srcRect.setWH(image()->size().width(), image()->size().height());
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
        const CZBitset<AKAlignment> alignment { m_alignment };

        m_renderableImage.layout().setPosition(YGEdgeLeft, YGUndefined);
        m_renderableImage.layout().setPosition(YGEdgeTop, YGUndefined);

        // X-axis
        if (alignment.hasAll(AKAlignLeft | AKAlignRight) || !alignment.has(AKAlignLeft | AKAlignRight))
            layout().setJustifyContent(YGJustifyCenter);
        else if (alignment.has(AKAlignLeft))
            layout().setJustifyContent(YGJustifyFlexStart);
        else if (alignment.has(AKAlignRight))
            layout().setJustifyContent(YGJustifyFlexEnd);

        // Y-axis
        if (alignment.hasAll(AKAlignTop | AKAlignBottom) || !alignment.has(AKAlignTop | AKAlignBottom))
            layout().setAlignItems(YGAlignCenter);
        else if (alignment.has(AKAlignTop))
            layout().setAlignItems(YGAlignFlexStart);
        else if (alignment.has(AKAlignBottom))
            layout().setAlignItems(YGAlignFlexEnd);

        layout().calculate();
    }        
}
