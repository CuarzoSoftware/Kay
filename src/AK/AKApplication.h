#ifndef AKAPPLICATION_H
#define AKAPPLICATION_H

#include <AK/AKObject.h>
#include <EGL/egl.h>
#include <unordered_map>
#include <include/gpu/GrDirectContext.h>

class AK::AKApplication : public AKObject
{
public:
    AKApplication() noexcept;
    static AKGLContext *glContext() noexcept;
    static void freeGLContext() noexcept;
    const GrContextOptions &skContextOptions() const noexcept { return m_skContextOptions; };
    std::vector<AKNode*> animated;
protected:
    GrContextOptions m_skContextOptions;
    std::unordered_map<EGLContext, AKGLContext*> m_glContexts;
};

#endif // AKAPPLICATION_H
