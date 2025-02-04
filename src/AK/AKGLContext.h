#ifndef AKGLCONTEXT_H
#define AKGLCONTEXT_H

#include <AK/AKObject.h>
#include <EGL/egl.h>
#include <include/gpu/GrDirectContext.h>
#include <include/core/SkSurface.h>
#include <unordered_map>

class AK::AKGLContext : public AKObject
{
public:
    sk_sp<GrDirectContext> skContext() const noexcept { return m_skContext; }
private:
    friend class AKApplication;
    friend class AKSurface;
    struct FBO
    {
        GLuint id; // The GL fbo id
        UInt32 serial; // The AKSurface current id
        sk_sp<SkSurface> skSurface;
    };
    AKGLContext(sk_sp<GrDirectContext> skContext) noexcept;
    ~AKGLContext();
    const FBO &getFBO(UInt32 slot) const noexcept;
    void storeFBO(UInt32 slot, FBO fbo) noexcept;
    void destroyFBO(UInt32 slot) noexcept;
    EGLDisplay m_eglDisplay;
    EGLContext m_eglContext;
    sk_sp<GrDirectContext> m_skContext;
    std::unordered_map<UInt32, FBO> m_fbos;
};

#endif // AKGLCONTEXT_H
