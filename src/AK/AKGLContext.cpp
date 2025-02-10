#include <AK/AKGLContext.h>
#include <AK/AKLog.h>

using namespace AK;

AKGLContext::AKGLContext(sk_sp<GrDirectContext> skContext) noexcept : m_skContext(skContext)
{
    m_eglDisplay = eglGetCurrentDisplay();
    m_eglContext = eglGetCurrentContext();
    m_painter.reset(new AKPainter());
    AKLog::debug("[AKGLContext] Created for EGLDisplay %lu EGLContext %lu.", (UInt64)m_eglDisplay, (UInt64)m_eglContext);
}

AKGLContext::~AKGLContext()
{
    while (!m_fbos.empty())
        destroyFBO(m_fbos.begin()->first);

    AKLog::debug("[AKGLContext] Destroyed from EGLDisplay %lu EGLContext %lu.", (UInt64)m_eglDisplay, (UInt64)m_eglContext);
}

const AKGLContext::FBO &AKGLContext::getFBO(UInt32 slot) const noexcept
{
    static FBO invalidFBO { 0, 0, nullptr };
    auto it = m_fbos.find(slot);
    if (it == m_fbos.end()) return invalidFBO;
    return it->second;
}

void AKGLContext::storeFBO(UInt32 slot, FBO fbo) noexcept
{
    assert("Slot already in use." && m_fbos.find(slot) == m_fbos.end());
    m_fbos[slot] = fbo;
}

void AKGLContext::destroyFBO(UInt32 slot) noexcept
{
    auto it = m_fbos.find(slot);
    if (it == m_fbos.end()) return;

    if (eglGetCurrentDisplay() != m_eglDisplay || eglGetCurrentContext() != m_eglContext)
        AKLog::warning("[AKGLContext] Attempt to destroy FBO %d without its EGLDisplay and EGLContext bound. This can lead to GL memory leaks.", it->second.id);
    else
    {
        GLint currentFB;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFB);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &it->second.id);
        glBindFramebuffer(GL_FRAMEBUFFER, currentFB);
        //AKLog::debug("[AKGLContext] Slot %d FBO %d destroyed.", it->first, it->second.id);
    }

    m_fbos.erase(it);
}
