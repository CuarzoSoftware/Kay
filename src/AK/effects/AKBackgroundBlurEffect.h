#ifndef AKBACKGROUNDBLUREFFECT_H
#define AKBACKGROUNDBLUREFFECT_H

#include <AK/effects/AKBackgroundEffect.h>
#include <AK/AKSignal.h>
#include <AK/AKBrush.h>
#include <include/core/SkPath.h>

/**
 * @brief Background blur effect
 *
 * This effect allows blurring the background of a node with optional clipping.
 *
 * It only works if an SkImage is provided to the current AKTarget and
 * shares the same backing storage as the target surface.
 */
class AK::AKBackgroundBlurEffect : public AKBackgroundEffect
{
public:

    /**
     * @brief Changes.
     */
    enum Changes
    {
        /// The sigma() value has changed.
        CHSigma = AKBackgroundEffect::CHLast,

        /// The clipMode() value has changed.
        CHClipMode,
        CHLast
    };

    /**
     * @brief Specifies how the effect is positioned and clipped relative to the target node.
     */
    enum ClipMode
    {
        /**
         * The effectRect is automatically matched with the target node and clipped by it.
         * In this mode, the custom clip path is ignored.
         */
        Automatic,

        /**
         * Both effectRect and the clip path are manually specified by subscribing to onSceneCalculatedRectSignal.
         */
        Manual
    };

    /**
     * @brief Creates a background blur effect with the specified Gaussian sigma, attached to the target node.
     *
     * @param clipMode Specifies the clipping mode for the effect. Default is ClipMode::Automatic.
     * @param sigma The x and y Gaussian sigma values for the blur effect. Default is {16.f, 16.f}.
     * @param target The target node to which the effect is attached. Default is nullptr.
     */
    AKBackgroundBlurEffect(ClipMode clipMode = Automatic, const SkVector &sigma = { 16.f, 16.f }, AKNode *target = nullptr) noexcept :
        AKBackgroundEffect(Behind),
        m_sigma(sigma),
        m_clipMode(clipMode)
    {
        if (target)
            target->addBackgroundEffect(this);
    }

    AKCLASS_NO_COPY(AKBackgroundBlurEffect)

    /**
     * @brief Sets the clipping mode for the blur effect.
     */
    void setClipMode(ClipMode mode) noexcept
    {
        if (m_clipMode == mode)
            return;

        m_clipMode = mode;
        addChange(CHClipMode);
    }

    /**
     * @brief Gets the current clipping mode.
     */
    ClipMode clipMode() const noexcept
    {
        return m_clipMode;
    }

    /**
     * @brief Sets the Gaussian sigma values for the blur effect.
     *
     * @param sigma The new x and y Gaussian sigma values.
     */
    void setSigma(const SkVector &sigma) noexcept
    {
        if (m_sigma == sigma)
            return;

        m_sigma = sigma;
        addChange(CHSigma);
    }

    /**
     * @brief Gets the current Gaussian sigma values.
     */
    const SkVector &sigma() const noexcept
    {
        return m_sigma;
    }

    struct
    {
        /**
         * @brief Signal triggered after the target node layout has been calculated.
         *
         * This signal is triggered only when using ClipMode::Manual.
         * During the callback, both effectRect and clipPath should be specified.
         */
        AKSignal<> targetLayoutUpdated;
    } on;

    /**
     * @brief Clip path for the blur area.
     *
     * This path is only used by the ClipMode::Manual mode and must be specified
     * relative to the effectRect. Parts that fall outside effectRect are ignored.
     *
     * Should be set during the callback of onSceneCalculatedRectSignal which is triggered right after the target
     * node layout has been calculated.
     *
     * @note Performance tip: Avoid updating when not required as Skia regenerates the clipping mask each time.
     */
    SkPath clip;
    using AKBackgroundEffect::effectRect;
protected:
    void onSceneCalculatedRect() override;
    void renderEvent(const AKRenderEvent &event) override;
    void onTargetNodeChanged() override { /* Nothing to free here */ }
private:
    AKBrush m_brush;
    SkVector m_sigma;
    ClipMode m_clipMode;
};

#endif // AKBACKGROUNDBLUREFFECT_H
