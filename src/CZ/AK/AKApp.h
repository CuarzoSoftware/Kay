#ifndef CZ_AKAPPLICATION_H
#define CZ_AKAPPLICATION_H

#include <CZ/AK/Input/AKPointer.h>
#include <CZ/AK/Input/AKKeyboard.h>
#include <CZ/Core/Cuarzo.h>
#include <CZ/Ream/Ream.h>
#include <CZ/skia/modules/skparagraph/include/FontCollection.h>

/**
 * @brief Core application class.
 *
 * A per-process singleton providing shared services to all scenes: input state and
 * focus (pointer(), keyboard()), font services and access to the CZCore (event loop)
 * and RCore (Ream graphics) instances. Input events are typically sent to the AKApp,
 * which dispatches them to the focused AKScene (see @ref events_page "Event System").
 *
 * @ingroup AKCore
 */
class CZ::AKApp : public AKObject
{
public:
    /**
     * @brief Returns the current AKApp instance, or creates one if none exists.
     *
     * Kay requires both a CZCore and an RCore instance to function. The CZCore instance
     * handles Kay's event loop source, which is dispatched via CZCore::dispatch(). If you
     * are not using Louvre or Marco, you must create both CZCore and RCore instances manually
     * before calling this, otherwise, nullptr will be returned.
     *
     * @return A shared pointer to the AKApp, or nullptr on failure.
     */
    static std::shared_ptr<AKApp> GetOrMake() noexcept;

    /**
     * @brief Returns the current AKApp instance.
     *
     * @return A shared pointer to the AKApp, or nullptr if no instance exists.
     */
    static std::shared_ptr<AKApp> Get() noexcept;

    /**
     * @brief The CZCore instance handling the event loop.
     */
    std::shared_ptr<CZCore> core() const noexcept { return m_cuarzo; }

    /**
     * @brief The RCore (Ream) graphics context.
     */
    std::shared_ptr<RCore> ream() const noexcept { return m_ream; }

    /**
     * @brief The Skia font manager used by text nodes.
     */
    sk_sp<SkFontMgr> fontManager() const noexcept;

    /**
     * @brief The Skia font collection used by text nodes (e.g. AKText).
     */
    sk_sp<skia::textlayout::FontCollection> fontCollection() const noexcept;

    /**
     * @brief Global pointer state, focus and event history.
     */
    AKPointer &pointer() noexcept { return m_pointer; };

    /**
     * @brief Global keyboard state, focus and event history.
     */
    AKKeyboard &keyboard() noexcept;
protected:
    bool event(const CZEvent &event) noexcept override;
private:
    friend class AKScene;
    friend class AKAnimation;
    AKApp(std::shared_ptr<CZCore> cuarzo, std::shared_ptr<RCore> ream) noexcept;
    void setKeyboard(AKKeyboard *keyboard) noexcept;
    std::shared_ptr<CZCore> m_cuarzo;
    std::shared_ptr<RCore> m_ream;
    AKPointer m_pointer;
    std::unique_ptr<AKKeyboard> m_keyboard;
    sk_sp<SkFontMgr> m_fontManager;
    sk_sp<skia::textlayout::FontCollection> m_fontCollection;
};

#endif // CZ_AKAPPLICATION_H
