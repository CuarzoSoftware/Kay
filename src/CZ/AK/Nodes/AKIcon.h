#ifndef AKICON_H
#define AKICON_H

#include <CZ/AK/Nodes/AKRenderable.h>

class CZ::AKIcon : public AKRenderable
{
public:
    enum Changes
    {
        CHIcon = AKRenderable::CHLast,
        CHLast
    };

    AKIcon(const std::string &iconName = "", UInt32 size = 32, AKNode *parent = nullptr) noexcept;

    void setSize(UInt32 size) noexcept;
    void setIcon(const std::string &iconName) noexcept;
    const std::string &iconName() const noexcept { return m_iconName; }
    std::shared_ptr<RImage> image() const noexcept { return m_image; }

protected:
    void onSceneBegin() override;
    void layoutEvent(const CZLayoutEvent &event) override;
    void renderEvent(const AKRenderEvent &event) override;

private:
    std::string m_iconName;
    std::shared_ptr<RImage> m_image;
};

#endif // AKICON_H
