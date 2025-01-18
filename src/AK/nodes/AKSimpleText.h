#ifndef AKSIMPLETEXT_H
#define AKSIMPLETEXT_H

#include <AK/AKPen.h>
#include <AK/AKBrush.h>
#include <include/core/SkFont.h>
#include <include/core/SkTypeface.h>
#include <AK/nodes/AKBakeable.h>

class AK::AKSimpleText : public AKBakeable
{
public:
    enum Changes
    {
        Chg_Font = AKBakeable::Chg_Last,
        Chg_Text,
        Chg_Brush,
        Chg_Pen,
        Chg_Last
    };

    AKSimpleText(const std::string &text, AKNode *parent = nullptr) noexcept :
        AKBakeable(parent)
    {
        m_brush.setAntiAlias(false);
        m_pen.setAntiAlias(true);
        setText(text);
        enableCustomTextureColor(true);
    }

    void setFont(const SkFont &font) noexcept
    {
        if (m_font == font)
            return;

        addChange(Chg_Font);
        m_font = font;
    }

    const SkFont &font() const noexcept
    {
        return m_font;
    }

    void setText(const std::string &text) noexcept
    {
        if (m_text == text)
            return;

        addChange(Chg_Text);
        m_text = text;
    }

    const std::string &text() const noexcept
    {
        return m_text;
    }

    void setBrush(const AKBrush &brush) noexcept
    {
        if (m_brush == brush)
            return;

        m_brush = brush;
        addChange(Chg_Brush);
    }

    const AKBrush &brush() const noexcept
    {
        return m_brush;
    }

    void setPen(const AKPen &pen) noexcept
    {
        if (m_pen == pen)
            return;

        m_pen = pen;
        addChange(Chg_Pen);
    }

    const AKPen &pen() const noexcept
    {
        return m_pen;
    }

protected:
    void updateLayout() override;
    void onBake(OnBakeParams *params) override;
    void updateDimensions() noexcept;
    std::string m_text;
    SkColor m_color { SK_ColorBLACK };
    SkFont m_font;
    AKBrush m_brush;
    AKPen m_pen { AKPen::NoPen() };
    SkRect m_bounds;
};

#endif // AKSIMPLETEXT_H
