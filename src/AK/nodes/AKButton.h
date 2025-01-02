#ifndef AKBUTTON_H
#define AKBUTTON_H

#include <AK/nodes/AKSubScene.h>
#include <AK/nodes/AKHThreePatch.h>
#include <AK/nodes/AKSimpleText.h>
#include <AK/nodes/AKContainer.h>

class AK::AKButton : public AKSubScene
{
public:
    enum Changes
    {
        Chg_Enabled = AKSubScene::Chg_Last,
        Chg_BackgroundColor,
        Chg_Last
    };

    AKButton(const std::string &text, AKNode *parent = nullptr) noexcept;
    void setText(const std::string &text) noexcept;

    void setBackgroundColor(SkColor color) noexcept;
    SkColor backgroundColor() const noexcept { return m_backgroundColor; };

    void setEnabled(bool enabled) noexcept;
    bool enabled() const noexcept { return m_enabled; }

protected:
    void onSceneBegin() override;
    void onLayoutUpdate() override;
    void applyLayoutConstraints() noexcept;
    AKHThreePatch m_hThreePatch { this };
    AKContainer m_content { YGFlexDirectionRow, true, &m_hThreePatch };
    AKSimpleText m_text;
    SkColor m_backgroundColor { SK_ColorWHITE };
    bool m_enabled { true };
};

#endif // AKBUTTON_H
