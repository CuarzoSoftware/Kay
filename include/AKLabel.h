#ifndef AKLABEL_H
#define AKLABEL_H

#include "include/core/SkFont.h"
#include "include/core/SkCanvas.h"
#include <AKFrame.h>

class AK::AKLabel : public AKFrame
{
public:
    AKLabel(const std::string &text = "", AKWidget *parent = nullptr) noexcept;

    void paintEvent(AKPainter &painter) noexcept override;

private:
    std::string m_text;
    SkFont m_font;

};

#endif // AKLABEL_H
