#ifndef AKPAINTER_H
#define AKPAINTER_H

#include <AK/AKTransform.h>
#include <include/core/SkColor.h>
#include <include/core/SkRegion.h>
#include <include/core/SkImage.h>
#include <AK/AKObject.h>
#include <GLES2/gl2.h>

class AK::AKPainter : public AKObject
{
public:
    /**
     * @brief Parameters required for bindTextureMode().
     *
     * This struct provides all the necessary parameters to map a texture into the current destination framebuffer.
     */
    struct TextureParams
    {
        /**
         * @brief Texture to be drawn.
         */
        sk_sp<SkImage> texture;

        /**
         * @brief Position of the texture (destination rect) in compositor-global coordinates.
         */
        SkIPoint pos;

        /**
         * @brief Subrect of the texture to be mapped in surface units.
         *
         * Coordinates should be specified in the space generated after applying the scale factor and transformation to the texture buffer.
         */
        SkRect srcRect;

        /**
         * @brief Destination size of the source rect in surface units.
         */
        SkISize dstSize;

        /**
         * @brief Transform already applied to the texture.
         *
         * For example, if the texture is rotated 90 degrees counterclockwise and you want to display it in a normal orientation,
         * use LTransform::Rotated90 and LPainter will apply the inverse transform (LTransform::Rotated270).\n
         * If you don't want to apply any transform use LTransform::Normal.
         */
        AKTransform srcTransform { AKTransform::Normal };

        /**
         * @brief Scale factor of the texture.
         */
        SkScalar srcScale { 1.f };
    };

    static std::shared_ptr<AKPainter> Make() noexcept
    {
        return std::shared_ptr<AKPainter>(new AKPainter());
    }

    void setParamsFromRenderable(AKRenderable *renderable) noexcept;

    /**
     * @brief Switches to texture mode.
     *
     * This method maps a texture to the compositor-global space, enabling subsequent drawing of specific sub-rectangles using drawBox(), drawRect() or drawRegion().
     *
     * @param params Parameters required to map the texture.
     */
    void bindTextureMode(const TextureParams &params) noexcept;

    /**
     * @brief Switches to color mode.
     *
     * In color mode, drawBox(), drawRect() and drawRegion() can be used to draw rectagles of the specified color.
     *
     * The color is set using setColor() and setAlpha().
     */
    void bindColorMode() noexcept;

    /**
     * @brief Draws a texture or color rect on the screen based on the current rendering mode.
     *
     * @param rect The rect to be drawn in compositor-global coordinates.
     */
    void drawRect(const SkIRect &rect) noexcept;

    /**
     * @brief Draws a texture or color region on the screen based on the current rendering mode.
     *
     * @param region The region to be drawn in compositor-global coordinates.
     */
    void drawRegion(const SkRegion &region) noexcept;

    /**
     * @brief Enables or disables a custom texture color.
     *
     * When enabled, the bound texture RGB values are replaced by the color set with setColor().
     */
    void enableCustomTextureColor(bool enabled) noexcept;

    /**
     * @brief Checks if custom texture color is enabled.
     *
     * @see enableCustomTextureColor().
     *
     * @return `true` if custom texture color is enabled, otherwise `false`.
     */
    bool customTextureColorEnabled() const noexcept;

    /**
     * @brief Checks if automatic blending function selection is enabled.
     *
     * @see enable enableAutoBlendFunc().
     */
    bool autoBlendFuncEnabled() const noexcept;

    /**
     * @brief Toggles automatic blending function selection.
     *
     * When enabled, LPainter will automatically select the appropriate OpenGL blend function mode
     * based on whether the bound texture uses premultiplied alpha or not (see LTexture::premultipliedAlpha()).
     *
     * When disabled, LPainter will use the blend function set with setBlendFunc().
     *
     * Enabled by default.
     */
    void enableAutoBlendFunc(bool enabled) noexcept;

    /**
     * @brief Sets a custom blend function.
     *
     * Setting a custom blend function can be useful for masking or other sophisticated blending effects.
     *
     * To make LPainter use this blend function, the auto blend function must be disabled, see enableAutoBlendFunc().
     */
    void setBlendFunc(const AKBlendFunc &blendFunc) noexcept;

    /**
     * @brief Sets the alpha value.
     *
     * - In texture mode, the texture alpha value is multiplied by this value.
     * - In color mode, this value becomes the alpha value of the color.
     *
     * @param alpha The alpha value to be set.
     */
    void setAlpha(SkScalar alpha) noexcept;

    /**
     * @brief Sets the color.
     *
     * - In texture mode and if customTextureColorEnabled() is enabled, this value replaces the texture RGB values while keeping the alpha intact.
     * - If color mode, this is the color to be drawn. See setAlpha().
     *
     * @param color The color to be set.
     */
    void setColor(const SkColor4f &color) noexcept;

    /**
     * @brief Sets the color factor.
     *
     * This method multiplies each component of the source color by the specified factor.
     *
     * @note Setting all components to 1.0 disables the effect.
     *
     * @param r The value of the red component (range: [0.0, 1.0]).
     * @param g The value of the green component (range: [0.0, 1.0]).
     * @param b The value of the blue component (range: [0.0, 1.0]).
     * @param a The value of the alpha component (range: [0.0, 1.0]).
     */
    void setColorFactor(Float32 r, Float32 g, Float32 b, Float32 a) noexcept;

    /**
     * @brief Sets the color factor.
     *
     * This method multiplies each component of the source color by the specified factor.
     *
     * @note Setting all components to 1.0 disables the effect.
     *
     * @param factor The color factor to be set.
     */
    void setColorFactor(const SkColor4f &factor) noexcept;

    /**
     * @brief Sets the clear color.
     *
     * This method sets the clear color used when calling clearScreen().
     *
     * @param r Value of the red component (range [0.0, 1.0]).
     * @param g Value of the green component (range [0.0, 1.0]).
     * @param b Value of the blue component (range [0.0, 1.0]).
     * @param a Value of the alpha component (range [0.0, 1.0]).
     */
    void setClearColor(Float32 r, Float32 g, Float32 b, Float32 a) noexcept;

    /**
     * @brief Clear the framebuffer.
     *
     * This method clears the bound framebuffer using the color set with setClearColor().
     */
    void clearScreen() noexcept;

    /**
     * @brief Binds the specified framebuffer for rendering.
     *
     * This method binds the provided framebuffer for rendering, allowing subsequent rendering operations to be performed on it.
     *
     * @note Output framebuffers are automatically bound prior a LOutput::paintGL() event.
     *
     * @param framebuffer The framebuffer to be bound.
     */
    void bindTarget(AKTarget *target) noexcept;

    /**
     * @brief Retrieves the currently bound framebuffer.
     *
     * This method returns a pointer to the currently bound framebuffer for rendering.
     *
     * @return A pointer to the currently bound framebuffer.
     */
    AKTarget *boundTarget() const noexcept;

    /**
     * @brief Bind the internal LPainter program.
     *
     * @note This method should be used if you are working with your own OpenGL programs and want to use the LPainter methods again.
     */
    void bindProgram() noexcept;

    ~AKPainter();

private:
    friend class AKGLContext;
    AKPainter() noexcept;

    enum ShaderMode : GLint
    {
        LegacyMode = 0,
        TextureMode = 1,
        ColorMode = 2
    };

    struct Uniforms
    {
        GLuint
            texSize,
            srcRect,
            activeTexture,
            mode,
            color,
            colorFactorEnabled,
            texColorEnabled,
            alpha,
            premultipliedAlpha,
            has90deg;
    } uniforms, uniformsExternal;

    Uniforms *currentUniforms;

    struct UserState
    {
        TextureParams textureParams;
        ShaderMode mode { TextureMode };
        sk_sp<SkImage> texture;
        AKBlendFunc customBlendFunc { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA };
        Float32 alpha { 1.f };
        SkColor4f  color { 1.f, 1.f, 1.f, 1.f };
        SkColor4f colorFactor { 1.f, 1.f, 1.f, 1.f };
        bool autoBlendFunc { true };
        bool customTextureColor { false };
    } userState;

    SkRect srcRect { 0, 0, 0, 0 };
    bool needsBlendFuncUpdate { true };

    static inline GLfloat square[]
    {
        //  VERTEX     FRAGMENT
       -1.0f,  1.0f,   0.f, 1.f, // TL
       -1.0f, -1.0f,   0.f, 0.f, // BL
        1.0f, -1.0f,   1.f, 0.f, // BR
        1.0f,  1.0f,   1.f, 1.f  // TR
    };

    GLuint vertexShader, fragmentShader, fragmentShaderExternal;

    struct ShaderState
    {
        SkSize texSize { 0, 0 };
        SkRect srcRect { 0, 0, 0, 0 };
        GLuint activeTexture { 0 };
        ShaderMode mode { ShaderMode::TextureMode };
        SkColor4f color { SkColors::kBlack };
        GLfloat alpha { 1.f };
        GLfloat scale { 1.f };
        bool colorFactorEnabled { false };
        bool texColorEnabled { false };
        bool premultipliedAlpha { false };
        bool has90deg { false };
    };

    ShaderState state, stateExternal;
    ShaderState *currentState;
    GLuint programObject, programObjectExternal, currentProgram;
    AKTarget *t = nullptr;
    GLuint fbId = 0;
    GLenum textureTarget;

    static GLuint compileShader(GLenum type, const char *shaderString);
    void setupProgram() noexcept;

    void shaderSetPremultipliedAlpha(bool premultipliedAlpha) noexcept
    {
        if (currentState->premultipliedAlpha != premultipliedAlpha)
        {
            currentState->premultipliedAlpha = premultipliedAlpha;
            glUniform1i(currentUniforms->premultipliedAlpha, premultipliedAlpha);
        }
    }

    void shaderSetTexSize(const SkSize &size) noexcept
    {
        if (currentState->texSize != size)
        {
            currentState->texSize = size;
            glUniform2f(currentUniforms->texSize, size.width(), size.height());
        }
    }

    void shaderSetSrcRect(const SkRect &rect) noexcept
    {
        if (currentState->srcRect != rect)
        {
            currentState->srcRect = rect;
            glUniform4f(currentUniforms->srcRect, rect.x(), rect.y(), rect.width(), rect.height());
        }
    }

    void shaderSetActiveTexture(GLuint unit) noexcept
    {
        if (currentState->activeTexture != unit)
        {
            currentState->activeTexture = unit;
            glUniform1i(currentUniforms->activeTexture, unit);
        }
    }

    void shaderSetMode(ShaderMode mode) noexcept
    {
        if (currentState->mode != mode)
        {
            currentState->mode = mode;
            glUniform1i(currentUniforms->mode, mode);
        }
    }

    void shaderSetColor(const SkColor4f &color) noexcept
    {
        if (currentState->color != color)
        {
            currentState->color = color;
            glUniform3f(currentUniforms->color, color.fR, color.fG, color.fB);
        }
    }

    void shaderSetColorFactorEnabled(bool enabled) noexcept
    {
        if (currentState->colorFactorEnabled != enabled)
        {
            currentState->colorFactorEnabled = enabled;
            glUniform1i(currentUniforms->colorFactorEnabled, enabled);
        }
    }

    void shaderSetTexColorEnabled(bool enabled) noexcept
    {
        if (currentState->texColorEnabled != enabled)
        {
            currentState->texColorEnabled = enabled;
            glUniform1i(currentUniforms->texColorEnabled, enabled);
        }
    }

    void shaderSetHas90Deg(bool enabled) noexcept
    {
        if (currentState->has90deg != enabled)
        {
            currentState->has90deg = enabled;
            glUniform1i(currentUniforms->has90deg, enabled);
        }
    }

    void shaderSetAlpha(Float32 a) noexcept
    {
        if (currentState->alpha != a)
        {
            currentState->alpha = a;
            glUniform1f(currentUniforms->alpha, a);
        }
    }

    // GL params

    void switchTarget(GLenum target) noexcept;

    void setViewport(Int32 x, Int32 y, Int32 w, Int32 h) noexcept;

    void updateBlendingParams() noexcept;
};

#endif // AKPAINTER_H
