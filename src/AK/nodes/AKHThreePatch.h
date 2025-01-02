#ifndef AKHTHREEPATCH_H
#define AKHTHREEPATCH_H

#include <AK/nodes/AKRenderable.h>

class AK::AKHThreePatch : public AKRenderable
{
public:
    AKHThreePatch(AKNode *parent = nullptr) noexcept : AKRenderable(RenderableHint::Texture, parent) {};

    enum Changes
    {
        Chg_SideSrcRect = AKRenderable::Chg_Last,
        Chg_CenterSrcRect,
        Chg_Scale,
        Chg_Image,
        Chg_Last
    };

    void setSideSrcRect(const SkRect &rect) noexcept
    {
        if (rect == m_sideSrcRect)
            return;

        m_sideSrcRect = rect;
        addChange(Chg_SideSrcRect);
    }

    const SkRect &sideSrcRect() const noexcept
    {
        return m_sideSrcRect;
    }

    void setCenterSrcRect(const SkRect &rect) noexcept
    {
        if (rect == m_centerSrcRect)
            return;

        m_centerSrcRect = rect;
        addChange(Chg_SideSrcRect);
    }

    const SkRect &centerSrcRect() const noexcept
    {
        return m_centerSrcRect;
    }

    void setScale(SkScalar scale) noexcept
    {
        if (scale == m_scale)
            return;

        m_scale = scale;
        addChange(Chg_Scale);
    }

    SkScalar scale() const noexcept
    {
        return m_scale;
    }

    void setImage(sk_sp<SkImage> image) noexcept
    {
        if (m_image == image)
            return;

        m_image = image;
        addChange(Chg_Image);
    }

    sk_sp<SkImage> image() const noexcept
    {
        return m_image;
    }

protected:
    void onRender(AKPainter *painter, const SkRegion &damage) override;
    SkRect m_sideSrcRect { 0.f, 0.f, 0.f, 0.f };
    SkRect m_centerSrcRect { 0.f, 0.f, 0.f, 0.f };
    SkScalar m_scale { 1.f };
    sk_sp<SkImage> m_image;
};

#endif // AKHTHREEPATCH_H
