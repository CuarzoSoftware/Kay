#ifndef CZ_AKAPPLICATION_H
#define CZ_AKAPPLICATION_H

#include <CZ/AK/Input/AKPointer.h>
#include <CZ/AK/Input/AKKeyboard.h>
#include <CZ/Core/Cuarzo.h>
#include <CZ/Ream/Ream.h>
#include <CZ/skia/modules/skparagraph/include/FontCollection.h>

/**
 * @brief Core application class
 */
class CZ::AKApp : public AKObject
{
public:
    static std::shared_ptr<AKApp> Make() noexcept;
    static std::shared_ptr<AKApp> Get() noexcept;
    sk_sp<SkFontMgr> fontManager() const noexcept;
    sk_sp<skia::textlayout::FontCollection> fontCollection() const noexcept;
    AKPointer &pointer() noexcept;
    AKKeyboard &keyboard() noexcept;
private:
    friend class AKScene;
    friend class AKAnimation;
    AKApp(std::shared_ptr<CZCore> cuarzo, std::shared_ptr<RCore> ream) noexcept;
    void setPointer(AKPointer *pointer) noexcept;
    void setKeyboard(AKKeyboard *keyboard) noexcept;
    std::shared_ptr<CZCore> m_cuarzo;
    std::shared_ptr<RCore> m_ream;
    std::unique_ptr<AKPointer> m_pointer;
    std::unique_ptr<AKKeyboard> m_keyboard;
    sk_sp<SkFontMgr> m_fontManager;
    sk_sp<skia::textlayout::FontCollection> m_fontCollection;
};

#endif // CZ_AKAPPLICATION_H
