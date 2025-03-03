#ifndef AKGLCONTEXT_H
#define AKGLCONTEXT_H

#include <AK/AKPainter.h>
#include <AK/AKObject.h>

#include <include/core/SkSurface.h>
#include <include/gpu/ganesh/GrDirectContext.h>

#include <unordered_map>
#include <EGL/egl.h>

class AK::AKGLContext : public AKObject
{
public:
    sk_sp<GrDirectContext> skContext() const noexcept { return m_skContext; }
    std::shared_ptr<AKPainter> painter() const noexcept { return m_painter; };    
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
    AKCLASS_NO_COPY(AKGLContext);
    ~AKGLContext();
    const FBO &getFBO(UInt32 slot) const noexcept;
    void storeFBO(UInt32 slot, FBO fbo) noexcept;
    void destroyFBO(UInt32 slot) noexcept;
    sk_sp<GrDirectContext> m_skContext;
    std::shared_ptr<AKPainter> m_painter;
    EGLDisplay m_eglDisplay;
    EGLContext m_eglContext;
    std::unordered_map<UInt32, FBO> m_fbos;
};

#endif // AKGLCONTEXT_H
