#ifndef AKAPPLICATION_H
#define AKAPPLICATION_H

#include "modules/skparagraph/include/FontCollection.h"
#include <AK/input/AKPointer.h>
#include <AK/input/AKKeyboard.h>
#include <EGL/egl.h>
#include <unordered_map>
#include <include/gpu/ganesh/GrContextOptions.h>

class AK::AKApplication : public AKObject
{
public:
    AKApplication() noexcept;
    sk_sp<SkFontMgr> fontManager() const noexcept;
    sk_sp<skia::textlayout::FontCollection> fontCollection() const noexcept;
    static AKGLContext *glContext() noexcept;
    static void freeGLContext() noexcept;
    const GrContextOptions &skContextOptions() const noexcept { return m_skContextOptions; };
    std::vector<AKNode*> animated;


    bool postEvent(const AKEvent &event, AKObject &object);
    AKPointer &pointer() noexcept;
    AKKeyboard &keyboard() noexcept;
protected:
    void setPointer(AKPointer *pointer) noexcept;
    void setKeyboard(AKKeyboard *keyboard) noexcept;
    std::unique_ptr<AKPointer> m_pointer;
    std::unique_ptr<AKKeyboard> m_keyboard;
    GrContextOptions m_skContextOptions;
    sk_sp<SkFontMgr> m_fontManager;
    sk_sp<skia::textlayout::FontCollection> m_fontCollection;
    std::unordered_map<EGLContext, AKGLContext*> m_glContexts;
};

#endif // AKAPPLICATION_H
