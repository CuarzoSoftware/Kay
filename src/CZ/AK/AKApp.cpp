#include <CZ/Events/CZEvent.h>
#include <CZ/AK/AKApp.h>
#include <CZ/AK/AKLog.h>

#include <CZ/Ream/RCore.h>
#include <CZ/CZCore.h>

#include <CZ/skia/ports/SkFontMgr_fontconfig.h>

using namespace CZ;

static std::weak_ptr<AKApp> s_app;

AKApp::AKApp(std::shared_ptr<CZCore> cuarzo, std::shared_ptr<RCore> ream) noexcept : m_cuarzo(cuarzo), m_ream(ream)
{
    m_fontManager = SkFontMgr_New_FontConfig(nullptr);
    assert("Failed to create the font manager" && m_fontManager);
    m_fontCollection = sk_make_sp<skia::textlayout::FontCollection>();
    m_fontCollection->setDefaultFontManager(m_fontManager);
    m_fontCollection->enableFontFallback();
}

std::shared_ptr<AKApp> AKApp::Make() noexcept
{
    if (auto app = s_app.lock())
        return app;

    auto cuarzo { CZCore::Get() };

    if (!cuarzo)
    {
        AKLog(CZFatal, CZLN, "Failed to create AKApp (missing CZCore)");
        return {};
    }

    auto ream { RCore::Get() };

    if (!ream)
    {
        AKLog(CZFatal, CZLN, "Failed to create AKApp (missing RCore)");
        return {};
    }

    auto app { std::shared_ptr<AKApp>(new AKApp(cuarzo, ream)) };
    s_app = app;
    return app;
}

std::shared_ptr<CZ::AKApp> AKApp::Get() noexcept
{
    return s_app.lock();
}

sk_sp<SkFontMgr> AKApp::fontManager() const noexcept
{
    return m_fontManager;
}

sk_sp<skia::textlayout::FontCollection> AKApp::fontCollection() const noexcept
{
    return m_fontCollection;
}

AKPointer &AKApp::pointer() noexcept
{
    if (!m_pointer)
        m_pointer.reset(new AKPointer());

    return *m_pointer.get();
}

AKKeyboard &AKApp::keyboard() noexcept
{
    if (!m_keyboard)
        m_keyboard.reset(new AKKeyboard());

    return *m_keyboard.get();
}

void AKApp::setPointer(AKPointer *pointer) noexcept
{
    if (m_pointer.get() == pointer)
        return;

    m_pointer.reset(pointer);
}

void AKApp::setKeyboard(AKKeyboard *keyboard) noexcept
{
    if (m_keyboard.get() == keyboard)
        return;

    m_keyboard.reset(keyboard);
}

