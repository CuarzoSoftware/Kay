#ifndef AKFRAMEP_H
#define AKFRAMEP_H

#include <AKPainter.h>
#include <AKFrame.h>
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/core/SkSurface.h"

class AK::AKFrame::Private
{
public:
    std::unique_ptr<AKPainter> painter;

    // For drawing into GL framebuffer
    GrGLFramebufferInfo fbInfo;
    GrBackendRenderTarget target;
    sk_sp<SkSurface> skiaSurface;

    // For using GL framebuffer as texture
    GrGLTextureInfo textureInfo;
    GrBackendTexture skiaTexture;
    sk_sp<SkImage> frame;
};

#endif // AKFRAMEP_H
