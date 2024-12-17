#include "AK/AKTarget.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include <AK/AKPainter.h>
#include <AK/nodes/AKRenderable.h>
#include <EGL/egl.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLES2/gl2.h>
#include <iostream>

#define LPAINTER_TRACK_UNIFORMS 1

using namespace AK;

static void makeExternalShader(std::string &shader) noexcept
{
    size_t pos = 0;
    std::string findStr = "sampler2D";
    std::string replaceStr = "samplerExternalOES";

    shader = std::string("#extension GL_OES_EGL_image_external : require\n") + shader;

    while ((pos = shader.find(findStr, pos)) != std::string::npos)
    {
        shader.replace(pos, findStr.length(), replaceStr);
        pos += replaceStr.length();
    }
}

void AKPainter::setParamsFromRenderable(AKRenderable *renderable) noexcept
{
    if (!renderable)
        return;

    bindProgram();
    bindTarget(renderable->currentTarget());
    setColor(renderable->color());
    setColorFactor(renderable->colorFactor());
    setAlpha(renderable->opacity());
    setBlendFunc(renderable->customBlendFunc());
    enableAutoBlendFunc(!renderable->customBlendFuncEnabled());
    enableCustomTextureColor(renderable->customTextureColorEnabled());
}

void AKPainter::bindTextureMode(const TextureParams &p) noexcept
{
    GrBackendTexture texInfo;
    GrGLTextureInfo glInfo;
    SkImages::GetBackendTextureFromImage(p.texture, &texInfo, false);
    texInfo.getGLTextureInfo(&glInfo);

    GLenum glTarget = glInfo.fTarget;
    switchTarget(glTarget);

    if (userState.mode != TextureMode)
    {
        userState.mode = TextureMode;
        needsBlendFuncUpdate = true;
    }

    if (!userState.texture || userState.texture->imageInfo().alphaType() != p.texture->imageInfo().alphaType())
        needsBlendFuncUpdate = true;

    userState.texture = p.texture;

    Float32 fbScale;
    fbScale = t->xyScale().x();

    SkIPoint pos = p.pos - SkIPoint::Make(t->viewport.x(), t->viewport.y());
    Float32 srcDstX, srcDstY;
    Float32 srcW, srcH;
    Float32 srcDstW, srcDstH;
    Float32 srcFbX1, srcFbY1, srcFbX2, srcFbY2;
    Float32 srcFbW, srcFbH;
    Float32 srcRectW = p.srcRect.width() <= 0.f ? 0.001f : p.srcRect.width();
    Float32 srcRectH = p.srcRect.height() <= 0.f ? 0.001f : p.srcRect.height();

    bool xFlip = false;
    bool yFlip = false;

    AKTransform invTrans = AK::requiredTransform(p.srcTransform, t->transform);
    bool rotate = AK::is90Transform(invTrans);

    if (AK::is90Transform(p.srcTransform))
    {
        srcH = Float32(p.texture->width()) / p.srcScale;
        srcW = Float32(p.texture->height()) / p.srcScale;
        yFlip = !yFlip;
        xFlip = !xFlip;
    }
    else
    {
        srcW = (Float32(p.texture->width()) / p.srcScale);
        srcH = (Float32(p.texture->height()) / p.srcScale);
    }

    srcDstW = (Float32(p.dstSize.width()) * srcW) / srcRectW;
    srcDstX = (Float32(p.dstSize.width()) * p.srcRect.x()) / srcRectW;
    srcDstH = (Float32(p.dstSize.height()) * srcH) / srcRectH;
    srcDstY = (Float32(p.dstSize.height()) * p.srcRect.y()) / srcRectH;

    switch (invTrans)
    {
    case AKTransform::Normal:
        break;
    case AKTransform::Rotated90:
        xFlip = !xFlip;
        break;
    case AKTransform::Rotated180:
        xFlip = !xFlip;
        yFlip = !yFlip;
        break;
    case AKTransform::Rotated270:
        yFlip = !yFlip;
        break;
    case AKTransform::Flipped:
        xFlip = !xFlip;
        break;
    case AKTransform::Flipped90:
        xFlip = !xFlip;
        yFlip = !yFlip;
        break;
    case AKTransform::Flipped180:
        yFlip = !yFlip;
        break;
    case AKTransform::Flipped270:
        break;
    default:
        return;
    }

    Float32 screenW = Float32(t->viewport.width());
    Float32 screenH = Float32(t->viewport.height());

    switch (t->transform)
    {
    case AKTransform::Normal:
        srcFbX1 = pos.x() - srcDstX;
        srcFbX2 = srcFbX1 + srcDstW;

        if (fbId == 0)
        {
            srcFbY1 = screenH - pos.y() + srcDstY;
            srcFbY2 = srcFbY1 - srcDstH;
        }
        else
        {
            srcFbY1 = pos.y() - srcDstY;
            srcFbY2 = srcFbY1 + srcDstH;
        }
        break;
    case AKTransform::Rotated90:
        srcFbX2 = pos.y() - srcDstY;
        srcFbX1 = srcFbX2 + srcDstH;

        if (fbId == 0)
        {
            srcFbY1 = pos.x() - srcDstX;
            srcFbY2 = srcFbY1 + srcDstW;
        }
        else
        {
            srcFbY2 = screenW - pos.x() + srcDstX;
            srcFbY1 = srcFbY2 - srcDstW;
            yFlip = !yFlip;
        }
        break;
    case AKTransform::Rotated180:
        srcFbX2 = screenW - pos.x() + srcDstX;
        srcFbX1 = srcFbX2 - srcDstW;

        if (fbId == 0)
        {
            srcFbY2 = pos.y() - srcDstY;
            srcFbY1 = srcFbY2 + srcDstH;
        }
        else
        {
            srcFbY2 = screenH - pos.y() + srcDstY;
            srcFbY1 = srcFbY2 - srcDstH;
        }
        break;
    case AKTransform::Rotated270:
        srcFbX1 = screenH - pos.y() + srcDstY;
        srcFbX2 = srcFbX1 - srcDstH;

        if (fbId == 0)
        {
            srcFbY2 = screenW - pos.x() + srcDstX;
            srcFbY1 = srcFbY2 - srcDstW;
        }
        else
        {
            srcFbY1 = pos.x() - srcDstX;
            srcFbY2 = srcFbY1 + srcDstW;
            yFlip = !yFlip;
        }
        break;
    case AKTransform::Flipped:
        srcFbX2 = screenW - pos.x() + srcDstX;
        srcFbX1 = srcFbX2 - srcDstW;

        if (fbId == 0)
        {
            srcFbY1 = screenH - pos.y() + srcDstY;
            srcFbY2 = srcFbY1 - srcDstH;
        }
        else
        {
            srcFbY1 = pos.y() - srcDstY;
            srcFbY2 = srcFbY1 + srcDstH;
        }
        break;
    case AKTransform::Flipped90:
        srcFbX2 = pos.y() - srcDstY;
        srcFbX1 = srcFbX2 + srcDstH;

        if (fbId == 0)
        {
            srcFbY2 = screenW - pos.x() + srcDstX;
            srcFbY1 = srcFbY2 - srcDstW;
        }
        else
        {
            srcFbY1 = pos.x() - srcDstX;
            srcFbY2 = srcFbY1 + srcDstW;
            yFlip = !yFlip;
        }
        break;
    case AKTransform::Flipped180:
        if (fbId == 0)
        {
            srcFbX1 = pos.x() - srcDstX;
            srcFbY1 = pos.y() - srcDstY + srcDstH;
            srcFbX2 = pos.x() - srcDstX + srcDstW;
            srcFbY2 = pos.y() - srcDstY;
        }
        else
        {
            srcFbY2 = screenH - pos.y() + srcDstY;
            srcFbY1 = srcFbY2 - srcDstH;
            srcFbX1 = pos.x() - srcDstX;
            srcFbX2 = pos.x() - srcDstX + srcDstW;
        }
        break;
    case AKTransform::Flipped270:
        srcFbX1 = screenH - pos.y() + srcDstY;
        srcFbX2 = srcFbX1 - srcDstH;

        if (fbId == 0)
        {
            srcFbY1 = pos.x() - srcDstX;
            srcFbY2 = srcFbY1 + srcDstW;
        }
        else
        {
            srcFbY2 = screenW - pos.x() + srcDstX;
            srcFbY1 = srcFbY2 - srcDstW;
            yFlip = !yFlip;
        }
        break;
    default:
        return;
    }

    shaderSetHas90Deg(rotate);

    if (xFlip)
    {
        srcW = srcFbX2;
        srcFbX2 = srcFbX1;
        srcFbX1 = srcW;
    }

    if (yFlip)
    {
        srcH = srcFbY2;
        srcFbY2 = srcFbY1;
        srcFbY1 = srcH;
    }

    srcFbX1 *= fbScale;
    srcFbY1 *= fbScale;

    srcFbW = srcFbX2 * fbScale - srcFbX1;
    srcFbH = srcFbY2 * fbScale - srcFbY1;

    srcRect = SkRect::MakeXYWH(srcFbX1, srcFbY1, srcFbW, srcFbH);

    glActiveTexture(GL_TEXTURE0);
    shaderSetMode(TextureMode);
    shaderSetActiveTexture(0);
    glBindTexture(glTarget, glInfo.fID);
    glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(glTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(glTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void AKPainter::bindColorMode() noexcept
{
    if (userState.mode == ColorMode)
        return;

    userState.mode = ColorMode;
    needsBlendFuncUpdate = true;
}

void AKPainter::drawRect(const SkIRect &rect) noexcept
{
    if (needsBlendFuncUpdate)
        updateBlendingParams();

    setViewport(rect.x(), rect.y(), rect.width(), rect.height());
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void AKPainter::drawRegion(const SkRegion &region) noexcept
{
    if (needsBlendFuncUpdate)
        updateBlendingParams();

    SkRegion::Iterator it(region);
    while (!it.done())
    {
        setViewport(it.rect().x(),
                    it.rect().y(),
                    it.rect().width(),
                    it.rect().height());
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        it.next();
    }
}

void AKPainter::enableCustomTextureColor(bool enabled) noexcept
{
    if (userState.customTextureColor == enabled)
        return;

    userState.customTextureColor = enabled;
    needsBlendFuncUpdate = true;
}

bool AKPainter::customTextureColorEnabled() const noexcept
{
    return userState.customTextureColor;
}

bool AKPainter::autoBlendFuncEnabled() const noexcept
{
    return userState.autoBlendFunc;
}

void AKPainter::enableAutoBlendFunc(bool enabled) noexcept
{
    if (userState.autoBlendFunc == enabled)
        return;

    userState.autoBlendFunc = enabled;
    needsBlendFuncUpdate = true;
}

void AKPainter::setBlendFunc(const AKBlendFunc &blendFunc) noexcept
{
    userState.customBlendFunc = blendFunc;

    if (!userState.autoBlendFunc)
        glBlendFuncSeparate(blendFunc.sRGBFactor, blendFunc.dRGBFactor, blendFunc.sAlphaFactor, blendFunc.dAlphaFactor);
}

void AKPainter::setAlpha(SkScalar alpha) noexcept
{
    if (userState.alpha == alpha)
        return;

    userState.alpha = alpha;
    needsBlendFuncUpdate = true;
}

void AKPainter::setColor(const SkColor4f &color) noexcept
{
    if (userState.color == color)
        return;

    userState.color = color;
    needsBlendFuncUpdate = true;
}

void AKPainter::setColorFactor(Float32 r, Float32 g, Float32 b, Float32 a) noexcept
{
    if (userState.colorFactor.fR == r &&
        userState.colorFactor.fG == g &&
        userState.colorFactor.fB == b &&
        userState.colorFactor.fA == a)
        return;

    userState.colorFactor = {r, g, b, a};
    shaderSetColorFactorEnabled(r != 1.f || g != 1.f || b != 1.f || a != 1.f);
    needsBlendFuncUpdate = true;
}

void AKPainter::setColorFactor(const SkColor4f &factor) noexcept
{
    if (userState.colorFactor == factor)
        return;

    userState.colorFactor = factor;
    shaderSetColorFactorEnabled(factor.fR != 1.f || factor.fG != 1.f || factor.fB != 1.f || factor.fA != 1.f);
    needsBlendFuncUpdate = true;
}

void AKPainter::setClearColor(Float32 r, Float32 g, Float32 b, Float32 a) noexcept
{
    glClearColor(r,g,b,a);
}

void AKPainter::clearScreen() noexcept
{
    if (!t)
        return;

    glDisable(GL_BLEND);
    setViewport(t->viewport.x(), t->viewport.y(), t->viewport.width(), t->viewport.height());
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
}

void AKPainter::bindTarget(AKTarget *target) noexcept
{
    if (!target)
    {
        fbId = 0;
        t = nullptr;
        return;
    }

    t = target;
    fbId = t->fbId();
    glBindFramebuffer(GL_FRAMEBUFFER, fbId);
}

AKTarget *AKPainter::boundTarget() const noexcept
{
    return t;
}

void AKPainter::bindProgram() noexcept
{
    eglBindAPI(EGL_OPENGL_ES_API);
    glUseProgram(currentProgram);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindAttribLocation(currentProgram, 0, "vertexPosition");
    glUseProgram(currentProgram);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, square);
    glEnableVertexAttribArray(0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_CLAMP);
    glDisable(GL_FRAMEBUFFER_SRGB);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_DITHER);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glDisable(GL_POLYGON_OFFSET_POINT);
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    glDisable(GL_RASTERIZER_DISCARD);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glDisable(GL_SAMPLE_ALPHA_TO_ONE);
    glDisable(GL_SAMPLE_COVERAGE);
    glDisable(GL_SAMPLE_SHADING);
    glDisable(GL_SAMPLE_MASK);
    glDisable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_POLYGON_STIPPLE);
    glDisable(GL_COLOR_LOGIC_OP);
    glDisable(GL_INDEX_LOGIC_OP);
    glDisable(GL_COLOR_TABLE);
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glDisable(GL_MULTISAMPLE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glCullFace(GL_BACK);
    glLineWidth(1);
    glHint(GL_GENERATE_MIPMAP_HINT, GL_FASTEST);
    glPolygonOffset(0, 0);
    glDepthFunc(GL_LESS);
    glDepthRangef(0, 1);
    glStencilMask(1);
    glDepthMask(GL_FALSE);
    glFrontFace(GL_CCW);
    glBlendColor(0, 0, 0, 0);
    glBlendEquation(GL_FUNC_ADD);

    glUniform2f(currentUniforms->texSize,
                currentState->texSize.width(),
                currentState->texSize.height());

    glUniform4f(currentUniforms->srcRect,
                currentState->srcRect.x(),
                currentState->srcRect.y(),
                currentState->srcRect.width(),
                currentState->srcRect.height());

    glUniform1i(currentUniforms->activeTexture,
                currentState->activeTexture);

    glUniform1i(currentUniforms->colorFactorEnabled,
                currentState->colorFactorEnabled);

    glUniform1f(currentUniforms->alpha,
                currentState->alpha);

    glUniform1i(currentUniforms->mode,
                currentState->mode);

    glUniform3f(currentUniforms->color,
                currentState->color.fR,
                currentState->color.fG,
                currentState->color.fB);

    glUniform1i(currentUniforms->texColorEnabled,
                currentState->texColorEnabled);

    glUniform1i(currentUniforms->premultipliedAlpha,
                currentState->premultipliedAlpha);

    glUniform1i(currentUniforms->has90deg,
                currentState->has90deg);

    updateBlendingParams();
}

AKPainter::AKPainter() noexcept
{
    GLchar vShaderStr[] = R"(
        precision mediump float;
        precision mediump int;
        uniform mediump vec2 texSize;
        uniform mediump vec4 srcRect;
        attribute mediump vec4 vertexPosition;
        varying mediump vec2 v_texcoord;
        uniform lowp int mode;
        uniform bool has90deg;

        void main()
        {
            gl_Position = vec4(vertexPosition.xy, 0.0, 1.0);

            if (mode == 1)
            {
                if (vertexPosition.x == -1.0)
                    v_texcoord.x = srcRect.x;
                else
                    v_texcoord.x = srcRect.z;

                if (vertexPosition.y == 1.0)
                    v_texcoord.y = srcRect.y;
                else
                    v_texcoord.y = srcRect.w;

                if (has90deg)
                    v_texcoord.yx = v_texcoord;
            }
        }
        )";

    GLchar fShaderStr[] =R"(
        uniform mediump sampler2D tex;
        uniform bool colorFactorEnabled;
        uniform lowp int mode;
        uniform mediump float alpha;
        uniform mediump vec3 color;
        varying mediump vec2 v_texcoord;
        uniform bool texColorEnabled;
        uniform bool premultipliedAlpha;

        void main()
        {
            // Texture
            if (mode != 2)
            {
                if (texColorEnabled)
                {
                    gl_FragColor.xyz = color;
                    gl_FragColor.w = texture2D(tex, v_texcoord).w;
                    if (alpha != 1.0)
                        gl_FragColor.w *= alpha;
                }
                else
                {
                    if (premultipliedAlpha)
                    {
                        gl_FragColor = texture2D(tex, v_texcoord);

                        if (alpha != 1.0)
                            gl_FragColor *= alpha;

                        if (colorFactorEnabled)
                            gl_FragColor.xyz *= color;
                    }
                    else
                    {
                        gl_FragColor = texture2D(tex, v_texcoord);

                        if (alpha != 1.0)
                            gl_FragColor.w *= alpha;

                        if (colorFactorEnabled)
                            gl_FragColor.xyz *= color;
                    }
                }
            }

            // Solid color
            else
            {
                gl_FragColor.xyz = color;
                gl_FragColor.w = alpha;
            }
        }
        )";

    std::string fShaderStrExternal = fShaderStr;
    makeExternalShader(fShaderStrExternal);
    vertexShader = compileShader(GL_VERTEX_SHADER, vShaderStr);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fShaderStr);
    fragmentShaderExternal = compileShader(GL_FRAGMENT_SHADER, fShaderStrExternal.c_str());

    GLint linked;

    /************** RENDER PROGRAM EXTERNAL **************/

    programObjectExternal = glCreateProgram();
    glAttachShader(programObjectExternal, vertexShader);
    glAttachShader(programObjectExternal, fragmentShaderExternal);
    glLinkProgram(programObjectExternal);
    glGetProgramiv(programObjectExternal, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(programObjectExternal, GL_INFO_LOG_LENGTH, &infoLen);
        glDeleteProgram(programObjectExternal);
        std::cerr << "[LPainter::LPainter] Failed to compile external OES shader.\n";
    }
    else
    {
        currentProgram = programObjectExternal;
#if LPAINTER_TRACK_UNIFORMS == 1
        currentState = &stateExternal;
#endif
        currentUniforms = &uniformsExternal;
        setupProgram();
    }

    /************** RENDER PROGRAM **************/

    programObject = glCreateProgram();
    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    // Link the program
    glLinkProgram(programObject);

    // Check the link status
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);
        glDeleteProgram(programObject);
        exit(-1);
    }

    currentProgram = programObject;
#if LPAINTER_TRACK_UNIFORMS == 1
    currentState = &state;
#endif
    currentUniforms = &uniforms;
    setupProgram();

    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
    glEnable(GL_BLEND);
    glEnable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_DITHER);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glDisable(GL_SAMPLE_COVERAGE);
    glDisable(GL_SAMPLE_ALPHA_TO_ONE);

    shaderSetPremultipliedAlpha(true);
    shaderSetColorFactorEnabled(false);
    shaderSetAlpha(1.f);
}

AKPainter::~AKPainter()
{
    notifyDestruction();
    glDeleteProgram(programObject);
    glDeleteProgram(programObjectExternal);
    glDeleteShader(fragmentShaderExternal);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);
}

GLuint AKPainter::compileShader(GLenum type, const char *shaderString)
{
    GLuint shader;
    GLint compiled;

    shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderString, nullptr);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        GLchar *errorLog = new GLchar[infoLen];
        glGetShaderInfoLog(shader, infoLen, &infoLen, errorLog);
        std::cerr << "[LOpenGL::compileShader] " << errorLog;
        glDeleteShader(shader);
        delete[] errorLog;
        return 0;
    }

    return shader;
}

void AKPainter::setupProgram() noexcept
{
    glBindAttribLocation(currentProgram, 0, "vertexPosition");

    // Use the program object
    glUseProgram(currentProgram);

    // Load the vertex data
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, square);

    // Enables the vertex array
    glEnableVertexAttribArray(0);

    // Get Uniform Variables
    currentUniforms->texSize = glGetUniformLocation(currentProgram, "texSize");
    currentUniforms->srcRect = glGetUniformLocation(currentProgram, "srcRect");
    currentUniforms->activeTexture = glGetUniformLocation(currentProgram, "tex");
    currentUniforms->mode = glGetUniformLocation(currentProgram, "mode");
    currentUniforms->color= glGetUniformLocation(currentProgram, "color");
    currentUniforms->texColorEnabled = glGetUniformLocation(currentProgram, "texColorEnabled");
    currentUniforms->colorFactorEnabled = glGetUniformLocation(currentProgram, "colorFactorEnabled");
    currentUniforms->alpha = glGetUniformLocation(currentProgram, "alpha");
    currentUniforms->premultipliedAlpha = glGetUniformLocation(currentProgram, "premultipliedAlpha");
    currentUniforms->has90deg = glGetUniformLocation(currentProgram, "has90deg");
}

void AKPainter::switchTarget(GLenum target) noexcept
{
    if (textureTarget != target)
    {
        if (target == GL_TEXTURE_2D)
        {
            currentProgram = programObject;
            currentUniforms = &uniforms;
            glUseProgram(currentProgram);
            currentState = &state;
            shaderSetColorFactorEnabled(stateExternal.colorFactorEnabled);
            shaderSetAlpha(stateExternal.alpha);
            shaderSetMode(stateExternal.mode);
            shaderSetColor(stateExternal.color);
            shaderSetTexColorEnabled(stateExternal.texColorEnabled);
            shaderSetPremultipliedAlpha(stateExternal.premultipliedAlpha);
            shaderSetHas90Deg(stateExternal.has90deg);
        }
        else
        {
            currentProgram = programObjectExternal;
            currentUniforms = &uniformsExternal;
            glUseProgram(currentProgram);
            currentState = &stateExternal;
            shaderSetColorFactorEnabled(state.colorFactorEnabled);
            shaderSetAlpha(state.alpha);
            shaderSetMode(state.mode);
            shaderSetColor(state.color);
            shaderSetTexColorEnabled(state.texColorEnabled);
            shaderSetPremultipliedAlpha(state.premultipliedAlpha);
            shaderSetHas90Deg(state.has90deg);
        }

        textureTarget = target;
    }
}

void AKPainter::setViewport(Int32 x, Int32 y, Int32 w, Int32 h) noexcept
{
    x -= t->viewport.x();
    y -= t->viewport.y();

    if (t->transform == AKTransform::Normal) {}
    else if (t->transform == AKTransform::Rotated270)
    {
        Float32 tmp = x;
        x = t->viewport.height() - y - h;
        y = tmp;
        tmp = w;
        w = h;
        h = tmp;
    }
    else if (t->transform == AKTransform::Rotated180)
    {
        x = t->viewport.width() - x - w;
        y = t->viewport.height() - y - h;
    }
    else if (t->transform == AKTransform::Rotated90)
    {
        Float32 tmp = x;
        x = y;
        y = t->viewport.width() - tmp - w;
        tmp = w;
        w = h;
        h = tmp;
    }
    else if (t->transform == AKTransform::Flipped)
    {
        x = t->viewport.width() - x - w;
    }
    else if (t->transform == AKTransform::Flipped270)
    {
        Float32 tmp = x;
        x = t->viewport.height() - y - h;
        y = t->viewport.width() - tmp - w;
        tmp = w;
        w = h;
        h = tmp;
    }
    else if (t->transform == AKTransform::Flipped180)
    {
        y = t->viewport.height() - y - h;
    }
    else if (t->transform == AKTransform::Flipped90)
    {
        Float32 tmp = x;
        x = y;
        y = tmp;
        tmp = w;
        w = h;
        h = tmp;
    }

    // TODO: use skia info
    if (fbId == 0)
    {
        if (AK::is90Transform(t->transform))
            y = t->viewport.width() - y - h;
        else
            y = t->viewport.height() - y - h;
    }

    Float32 fbScale;

    // TODO: Separate x and y
    fbScale = t->xyScale().x();

    const Int32 x2 = floorf(Float32(x + w) * fbScale);
    const Int32 y2 = floorf(Float32(y + h) * fbScale);

    x = floorf(Float32(x) * fbScale);
    y = floorf(Float32(y) * fbScale);
    w = x2 - x;
    h = y2 - y;

    glScissor(x, y, w, h);
    glViewport(x, y, w, h);

    if (currentState->mode == TextureMode)
    {
        shaderSetSrcRect(SkRect::MakeXYWH(
            (Float32(x) - srcRect.x()) / srcRect.width(),
            (Float32(y2) - srcRect.y()) / srcRect.height(),
            (Float32(x2) - srcRect.x()) / srcRect.width(),
            (Float32(y) - srcRect.y()) / srcRect.height()));
    }
}

void AKPainter::updateBlendingParams() noexcept
{
    needsBlendFuncUpdate = false;
    shaderSetMode(userState.mode);

    if (userState.mode == TextureMode)
    {
        /* Texture with replaced color */
        if (userState.customTextureColor)
        {
            shaderSetTexColorEnabled(true);
            SkColor4f color { userState.color };
            const Float32 alpha { userState.alpha * userState.colorFactor.fA};
            color.fR *= userState.colorFactor.fR;
            color.fG *= userState.colorFactor.fG;
            color.fB *= userState.colorFactor.fB;

            if (userState.autoBlendFunc)
            {
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            }
            else
            {
                glBlendFuncSeparate(userState.customBlendFunc.sRGBFactor,
                                    userState.customBlendFunc.dRGBFactor,
                                    userState.customBlendFunc.sAlphaFactor,
                                    userState.customBlendFunc.dAlphaFactor);
            }

            shaderSetColor(color);
            shaderSetAlpha(alpha);
        }

        /* Normal texture */
        else
        {
            shaderSetTexColorEnabled(false);

            /* Texture has premultiplied alpha */
            if (userState.texture && userState.texture->imageInfo().alphaType() == SkAlphaType::kPremul_SkAlphaType)
            {
                if (userState.autoBlendFunc)
                {
                    SkColor4f colorFactor;
                    const Float32 alpha { userState.alpha * userState.colorFactor.fA };
                    colorFactor.fR = userState.colorFactor.fR * alpha;
                    colorFactor.fG = userState.colorFactor.fG * alpha;
                    colorFactor.fB = userState.colorFactor.fB * alpha;
                    shaderSetPremultipliedAlpha(true);
                    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                    shaderSetColor(colorFactor);
                    shaderSetAlpha(alpha);
                }
                else
                {
                    const SkColor4f colorFactor {
                        userState.colorFactor.fR,
                        userState.colorFactor.fG,
                        userState.colorFactor.fB,
                        1.f
                    };
                    const Float32 alpha { userState.alpha * userState.colorFactor.fA };
                    shaderSetPremultipliedAlpha(false);
                    glBlendFuncSeparate(userState.customBlendFunc.sRGBFactor,
                                        userState.customBlendFunc.dRGBFactor,
                                        userState.customBlendFunc.sAlphaFactor,
                                        userState.customBlendFunc.dAlphaFactor);
                    shaderSetColor(colorFactor);
                    shaderSetAlpha(alpha);
                }
            }
            else
            {
                const SkColor4f colorFactor {
                    userState.colorFactor.fR,
                    userState.colorFactor.fG,
                    userState.colorFactor.fB,
                    1.f
                };
                const Float32 alpha { userState.alpha * userState.colorFactor.fA };
                shaderSetPremultipliedAlpha(false);

                if (userState.autoBlendFunc)
                {
                    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                }
                else
                {
                    glBlendFuncSeparate(userState.customBlendFunc.sRGBFactor,
                                        userState.customBlendFunc.dRGBFactor,
                                        userState.customBlendFunc.sAlphaFactor,
                                        userState.customBlendFunc.dAlphaFactor);
                }

                shaderSetColor(colorFactor);
                shaderSetAlpha(alpha);
            }
        }
    }

    /* Solid color mode */
    else
    {
        SkColor4f color { userState.color };
        Float32 alpha { userState.alpha };

        alpha *= userState.colorFactor.fA;
        color.fR *= userState.colorFactor.fR;
        color.fG *= userState.colorFactor.fG;
        color.fB *= userState.colorFactor.fB;

        if (userState.autoBlendFunc)
        {
            color.fR *= alpha;
            color.fG *= alpha;
            color.fB *= alpha;
            glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }
        else
        {
            glBlendFuncSeparate(userState.customBlendFunc.sRGBFactor,
                                userState.customBlendFunc.dRGBFactor,
                                userState.customBlendFunc.sAlphaFactor,
                                userState.customBlendFunc.dAlphaFactor);
        }

        shaderSetColor(color);
        shaderSetAlpha(alpha);
    }
}
