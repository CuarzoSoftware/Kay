#include <CZ/Events/CZEvent.h>
#include <CZ/AK/AKApp.h>
#include <CZ/AK/AKScene.h>
#include <CZ/AK/AKLog.h>

#include <CZ/Ream/RCore.h>
#include <CZ/Core/CZCore.h>
#include <CZ/Core/CZKeymap.h>

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

std::shared_ptr<AKApp> AKApp::GetOrMake() noexcept
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
    setTheme(nullptr);
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

AKKeyboard &AKApp::keyboard() noexcept
{
    if (!m_keyboard)
        m_keyboard.reset(new AKKeyboard());

    return *m_keyboard.get();
}

bool AKApp::event(const CZEvent &event) noexcept
{
    if (event.isPointerEvent())
    {
        switch (event.type())
        {
        case CZEvent::Type::PointerButton:
            pointer().m_history.button = (CZPointerButtonEvent&)event;
            break;
        case CZEvent::Type::PointerMove:
            pointer().m_history.move = (CZPointerMoveEvent&)event;
            break;
        case CZEvent::Type::PointerLeave:
            pointer().m_history.leave = (CZPointerLeaveEvent&)event;
            break;
        case CZEvent::Type::PointerScroll:
            pointer().m_history.scroll = (CZPointerScrollEvent&)event;
            break;
        default:
            break;
        }

        if (pointer().focus() && event.type() != CZEvent::Type::PointerEnter)
            core()->sendEvent(event, *pointer().focus());
    }
    else if (event.isKeyboardEvent())
    {
        switch (event.type())
        {
        case CZEvent::Type::KeyboardKey:
            keyboard().m_history.key = (CZKeyboardKeyEvent&)event;
            break;
        case CZEvent::Type::KeyboardModifiers:
            keyboard().m_history.modifiers = (CZKeyboardModifiersEvent&)event;
            CZCore::Get()->keymap()->updateModifiers(
                keyboard().m_history.modifiers.modifiers.depressed,
                keyboard().m_history.modifiers.modifiers.latched,
                keyboard().m_history.modifiers.modifiers.locked,
                keyboard().m_history.modifiers.modifiers.group);
            break;
        case CZEvent::Type::KeyboardLeave:
            keyboard().m_history.leave = (CZKeyboardLeaveEvent&)event;
            break;
        default:
            break;
        }

        if (keyboard().focus() && event.type() != CZEvent::Type::KeyboardEnter)
            core()->sendEvent(event, *keyboard().focus());
    }

    return AKObject::event(event);
}

void AKApp::setKeyboard(AKKeyboard *keyboard) noexcept
{
    if (m_keyboard.get() == keyboard)
        return;

    m_keyboard.reset(keyboard);
}

