#ifndef AKAPPLICATION_H
#define AKAPPLICATION_H

#include <AK/AKObject.h>
#include <EGL/egl.h>
#include <unordered_map>
#include <include/gpu/ganesh/GrContextOptions.h>

class AK::AKApplication : public AKObject
{
public:
    AKApplication() noexcept;
    sk_sp<SkFontMgr> fontManager() const noexcept;
    static AKGLContext *glContext() noexcept;
    static void freeGLContext() noexcept;
    const GrContextOptions &skContextOptions() const noexcept { return m_skContextOptions; };
    std::vector<AKNode*> animated;
protected:
    GrContextOptions m_skContextOptions;
    sk_sp<SkFontMgr> m_fontManager;
    std::unordered_map<EGLContext, AKGLContext*> m_glContexts;
};

#endif // AKAPPLICATION_H
