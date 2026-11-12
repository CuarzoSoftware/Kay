#include <CZ/AK/Nodes/AKImageFrame.h>
#include <CZ/AK/Events/AKRenderEvent.h>
#include <CZ/AK/AKLog.h>
#include <CZ/Core/Events/CZLayoutEvent.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Ream/RPass.h>
#include <CZ/Core/CZTimer.h>
#include <CZ/Core/CZWeak.h>
#include <cmath>

using namespace CZ;

AKImageFrame::AKImageFrame(AKNode *parent) noexcept :
    AKRenderable(RenderableHint::Image, parent) {}

AKImageFrame::AKImageFrame(std::shared_ptr<RImage> image, AKNode *parent) noexcept :
    AKRenderable(RenderableHint::Image, parent),
    m_image(image){}

void AKImageFrame::setImageProvider(ImageProvider provider) noexcept
{
    m_provider = std::move(provider);
    m_providerSize = m_providerScale = 0;

    // The image is chosen once the size it's drawn at is known (onSceneBegin()).
    if (m_provider)
        repaint();
}

void AKImageFrame::updateProvidedImage() noexcept
{
    // The size the image is drawn at (like the dst rect calculated below), in logical units: for
    // Contain/Cover from the current image's proportions (square until there's one).
    const SkSize frame { SkSize::Make(worldRect().size()) };
    SkSize image { 1.f, 1.f };

    if (m_image && !m_image->size().isEmpty())
        image = SkSize::Make(m_image->size());

    SkSize drawn { frame };

    if (m_sizeMode != SizeMode::Fill && !frame.isEmpty())
    {
        const SkScalar sx { frame.width() / image.width() }, sy { frame.height() / image.height() };
        const SkScalar s { m_sizeMode == SizeMode::Contain ? std::min(sx, sy) : std::max(sx, sy) };
        drawn = SkSize::Make(std::round(image.width() * s), std::round(image.height() * s));
    }

    const Int32 scale { std::max(1, AKNode::scale()) };
    const Int32 pixels { std::max(1, Int32(std::max(drawn.width(), drawn.height())) * scale) };
    const bool changing { pixels != m_lastPixels };
    m_lastPixels = pixels;

    if (pixels == m_providerSize && scale == m_providerScale)
        return;

    // Still resizing (e.g. growing in): keep the current image for now, the exact one once settled.
    if (changing && m_image && scale == m_providerScale)
    {
        if (!m_recheckPending)
        {
            m_recheckPending = true;
            CZTimer::OneShot(150, [weak = CZWeak<AKImageFrame>(this)](CZTimer*){
                if (!weak) return;
                weak->m_recheckPending = false;
                weak->repaint();
            });
        }

        return;
    }

    m_providerSize = pixels;
    m_providerScale = scale;
    auto provided { m_provider(pixels, scale) };

    if (provided != m_image)
    {
        m_image = std::move(provided);
        addChange(CHImage);
    }
}

bool AKImageFrame::setImage(std::shared_ptr<RImage> image) noexcept
{
    m_provider = nullptr;
    m_providerSize = m_providerScale = 0;

    if (image == m_image)
        return false;

    m_image = image;
    addChange(CHImage);
    return true;
}

bool AKImageFrame::setTransform(CZTransform transform) noexcept
{
    if (m_transform == transform)
        return false;

    m_transform = transform;
    addChange(CHTransform);
    return false;
}

bool AKImageFrame::setAlignment(CZAlignment alignment) noexcept
{
    if (m_alignment == alignment)
        return false;

    m_alignment = alignment;
    addChange(CHAlignment);
    return true;
}

bool AKImageFrame::setSizeMode(SizeMode mode) noexcept
{
    if (m_sizeMode == mode)
        return false;

    m_sizeMode = mode;
    addChange(CHSizeMode);
    return true;
}

bool AKImageFrame::setSrcRect(std::optional<SkRect> rect) noexcept
{
    if (m_srcRect == rect)
        return false;

    m_srcRect = rect;
    addChange(CHSrcRect);
    return false;
}

bool AKImageFrame::setScale(SkScalar scale) noexcept
{
    if (scale < 0.1f) scale = 0.1f;

    if (scale == m_scale)
        return false;

    m_scale = scale;
    addChange(CHScale);
    return true;
}

void AKImageFrame::onSceneBegin()
{
    AKRenderable::onSceneBegin();

    // Scale specific image: for the size it's drawn at on this screen (see setImageProvider()).
    if (m_provider)
        updateProvidedImage();

    if (!m_image)
    {
        invisibleRegion.setRect(AK_IRECT_INF);
        return;
    }

    const auto &ch { changes() };

    if (!ch.testAnyOf(CHLayoutSize, CHSizeMode, CHScale, CHImage, CHSrcRect, CHTransform, CHAlignment))
        return;

    addDamage(AK_IRECT_INF);

    /* CALC SRC RECT */

    if (m_srcRect.has_value())
        m_finalSrcRect = m_srcRect.value();
    else
    {
        if (CZ::Is90Transform(m_transform))
            m_finalSrcRect.setWH(m_image->size().height(), m_image->size().width());
        else
            m_finalSrcRect.setWH(m_image->size().width(), m_image->size().height());
    }

    /* CALC DST RECT */

    if (sizeMode() == SizeMode::Fill)
    {
        m_dst.setSize(worldRect().size());
        invisibleRegion.setEmpty();
    }
    else if (sizeMode() == SizeMode::Contain)
    {
        const auto frameSize { SkSize::Make(worldRect().size()) };
        const auto imageSize { SkSize::Make(m_finalSrcRect.width(), m_finalSrcRect.height()) };

        // Scale imageSize to fit frameSize while preserving aspect ratio
        const auto scaleX { frameSize.width() / imageSize.width() };
        const auto scaleY { frameSize.height() / imageSize.height() };
        const auto scale { std::min(scaleX, scaleY) }; // Preserve aspect ratio

        const auto scaledWidth { imageSize.width() * scale };
        const auto scaledHeight { imageSize.height() * scale };

        // Position based on alignment
        SkScalar x = 0.f, y = 0.f;

        const CZBitset<CZEdge> alignment { m_alignment };

        // Horizontal alignment
        if (alignment & CZEdgeLeft)
            x = 0;
        else if (alignment & CZEdgeRight)
            x = frameSize.width() - scaledWidth;
        else // Center horizontally
            x = (frameSize.width() - scaledWidth) * 0.5f;

        // Vertical alignment
        if (alignment & CZEdgeTop)
            y = 0;
        else if (alignment & CZEdgeBottom)
            y = frameSize.height() - scaledHeight;
        else // Center vertically
            y = (frameSize.height() - scaledHeight) * 0.5f;

        m_dst = SkRect::MakeXYWH(x, y, scaledWidth, scaledHeight).round();

        invisibleRegion.setRect(SkIRect::MakeSize(worldRect().size()));
        invisibleRegion.op(m_dst, SkRegion::kDifference_Op);
    }
    else // Cover
    {
        const auto frameSize { SkSize::Make(worldRect().size()) };
        const auto imageSize { SkSize::Make(m_finalSrcRect.width(), m_finalSrcRect.height()) };

        // Scale to cover while preserving aspect ratio
        const auto scaleX { frameSize.width() / imageSize.width() };
        const auto scaleY { frameSize.height() / imageSize.height() };
        const auto scale { std::max(scaleX, scaleY) };

        const auto scaledWidth { imageSize.width() * scale };
        const auto scaledHeight { imageSize.height() * scale };

        SkScalar x = 0.f, y = 0.f;

        const CZBitset<CZEdge> alignment { m_alignment };

        // Horizontal positioning
        if (alignment & CZEdgeLeft)
        {
            // Right edge matches frame right
            x = frameSize.width() - scaledWidth;
        }
        else if (alignment & CZEdgeRight)
        {
            // Left edge matches frame left
            x = 0.f;
        }
        else // Center horizontally
        {
            x = (frameSize.width() - scaledWidth) * 0.5f;
        }

        // Vertical positioning
        if (alignment & CZEdgeTop)
        {
            // Bottom edge matches frame bottom
            y = frameSize.height() - scaledHeight;
        }
        else if (alignment & CZEdgeBottom)
        {
            // Top edge matches frame top
            y = 0.f;
        }
        else // Center vertically
        {
            y = (frameSize.height() - scaledHeight) * 0.5f;
        }

        m_dst = SkRect::MakeXYWH(x, y, scaledWidth, scaledHeight).round();
        invisibleRegion.setEmpty();
    }
}

void AKImageFrame::renderEvent(const AKRenderEvent &e)
{
    if (e.damage.isEmpty() || !m_image)
        return;

    auto *p { e.pass->getPainter() };
    RDrawImageInfo info {};
    info.image = m_image;
    info.dst = m_dst.makeOffset(e.rect.topLeft());
    info.src = m_finalSrcRect;
    info.srcScale = m_srcRect.has_value() ? m_scale : 1.f;
    info.srcTransform = m_transform;
    p->drawImage(info, &e.damage);
}

