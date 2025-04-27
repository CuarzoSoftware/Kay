#ifndef AK_H
#define AK_H

#include <cstdint>
#include <include/core/SkFontMgr.h>

#define AK_ASSETS_DIR "/usr/local/share/Kay/assets"
#define AK_GET_CLASS(identifier) typename std::remove_const<std::remove_pointer<decltype(identifier)>::type>::type
#define AK_MAX_BUFFER_AGE 3
#define AK_UNUSED(object){(void)object;}
#define AKCLASS_NO_COPY(class_name) \
    class_name(const class_name&) = delete; \
    class_name(class_name&&) = delete; \
    class_name &operator=(const class_name&) = delete;

/**
 * @defgroup AKCore Core
 * @brief Core classes
 */
namespace AK
{
    class AKApplication;
    class AKObject;
    class AKWeakUtils;
    template <class T> class AKWeak;
    template <class T> class AKBitset;
    class AKSignalBase;
    template<typename...Args> class AKSignal;
    class AKListener;
    template<typename...Args> class AKListenerTemplate;
    class AKGLContext;
    class AKTheme;
    class AKTime;
    class AKChanges;
    class AKBackgroundDamageTracker;

    /* Event sources */

    class AKEventSource;
    class AKBooleanEventSource;
    class AKTimer;

    class AKAnimation;
    class AKImageLoader;
    class AKLog;

    class AKPen;
    class AKBrush;
    class AKPainter;
    class AKRRect;

    class AKScene;  /* Renders a root AKNode into an AKSceneTarget */
    class AKTarget;
    class AKSceneTarget; /* An AKScene render destination */
    class AKLayout; /* Yoga layout of an AKNode */
    class AKSurface;

    /*********** CORE NODE TYPES ***********/

    class AKNode;       /* Base class for nodes */
    class AKContainer;  /* Container for other nodes. Doesn't produce any output on its own */
    class AKRenderable; /* A node that draws directly into an AKSceneTarget */
    class AKBakeable;   /* A node that draws into its own buffer, which is then blitted into an AKSceneTarget */
    class AKSubScene;   /* A node whose children are drawn into its own buffer and then rendered into an AKSceneTarget */

    class AKSolidColor;
    class AKPath;
    class AKRenderableImage;
    class AKImageFrame;
    class AKThreeImagePatch;
    class AKTextCaret;
    class AKText;
    class AKCoreTextEditor;
    class AKRoundContainer;
    class AKButton;
    class AKTextField;
    class AKWindowButton;
    class AKWindowButtonGroup;
    class AKScroll;

    /************ EFFECTS ************/

    class AKRoundCornersEffect;
    class AKBackgroundEffect;
    class AKBackgroundBoxShadowEffect;
    class AKBackgroundImageShadowEffect;
    class AKBackgroundBlurEffect;
    class AKEdgeShadow;

    /************ INPUT *************/

    class AKKeyboard;
    class AKPointer;

    /************ EVENTS *************/

    class AKEvent;
    class AKDestroyEvent;
    class AKInputEvent;
    class AKInputDevice;

    class AKWindowEvent;
    class AKWindowStateEvent;
    class AKWindowCloseEvent;

    class AKPointerEvent;
    class AKPointerEnterEvent;
    class AKPointerLeaveEvent;
    class AKPointerMoveEvent;
    class AKPointerButtonEvent;
    class AKPointerScrollEvent;

    class AKPointerSwipeBeginEvent;
    class AKPointerSwipeUpdateEvent;
    class AKPointerSwipeEndEvent;

    class AKPointerPinchBeginEvent;
    class AKPointerPinchUpdateEvent;
    class AKPointerPinchEndEvent;

    class AKPointerHoldBeginEvent;
    class AKPointerHoldEndEvent;

    class AKKeyboardEvent;
    class AKKeyboardEnterEvent;
    class AKKeyboardLeaveEvent;
    class AKKeyboardKeyEvent;
    class AKKeyboardModifiersEvent;

    class AKTouchEvent;
    class AKTouchDownEvent;
    class AKTouchMoveEvent;
    class AKTouchUpEvent;
    class AKTouchFrameEvent;
    class AKTouchCancelEvent;
    class AKTouchPoint;

    class AKRenderEvent;
    class AKBakeEvent;

    class AKSceneChangedEvent;
    class AKLayoutEvent;

    class AKVibrancyEvent;

    class AKSafeEventQueue;

    /// @brief 64 bits unsigned integer
    typedef uint64_t        UInt64;

    /// @brief 64 bits signed integer
    typedef int64_t         Int64;

    /// @brief 32 bits unsigned integer
    typedef uint32_t        UInt32;

    /// @brief 32 bits signed integer
    typedef int32_t         Int32;

    /// @brief 16 bits unsigned integer
    typedef uint16_t        UInt16;

    /// @brief 16 bits signed integer
    typedef int16_t         Int16;

    /// @brief 8 bits unsigned integer
    typedef uint8_t         UInt8;

    /// @brief 8 bits signed integer
    typedef int8_t          Int8;

    /// @brief 8 bits unsigned integer
    typedef unsigned char   UChar8;

    /// @brief 8 bits signed integer
    typedef char            Char8;

    /// @brief 64 bits float
    typedef double          Float64;

    /// @brief 32 bits float
    typedef float           Float32;

    /// @brief Unsigned integer capable of holding a pointer
    typedef uintptr_t       UIntPtr;

    struct AKBlendFunc
    {
        /// Source RGB factor for blending
        UInt32 sRGBFactor;

        /// Destination RGB factor for blending
        UInt32 dRGBFactor;

        /// Source alpha factor for blending
        UInt32 sAlphaFactor;

        /// Destination alpha factor for blendin
        UInt32 dAlphaFactor;

        constexpr bool operator==(const AKBlendFunc &other) const
        {
            return sRGBFactor == other.sRGBFactor &&
                   dRGBFactor == other.dRGBFactor &&
                   sAlphaFactor == other.sAlphaFactor &&
                   dAlphaFactor == other.dAlphaFactor;
        }
    };

    AKApplication *akApp() noexcept;
    AKTheme *theme() noexcept;
    void setTheme(AKTheme *theme) noexcept;
    sk_sp<SkFontMgr> akFontManager() noexcept;
    AKPointer &akPointer() noexcept;
    AKKeyboard &akKeyboard() noexcept;
};

#define AK_IRECT_INF SkIRect::MakeLTRB(-2147483, -2147483, 2147483, 2147483)

#endif // AK_H
