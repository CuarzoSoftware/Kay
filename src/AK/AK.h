#ifndef AK_H
#define AK_H

#include <GLES2/gl2.h>
#include <cstdint>

#define AK_MAX_BUFFER_AGE 5

namespace AK
{
    class AKObject;
    class AKWeakUtils;
    template <class T> class AKWeak;
    template <class T> class AKBitset;
    class AKSignalBase;
    template<typename...Args> class AKSignal;
    class AKListener;
    template<typename...Args> class AKListenerTemplate;

    class AKPen;
    class AKBrush;
    class AKPainter;

    class AKScene;  /* Renders a root AKNode into an AKTarget */
    class AKTarget; /* An AKScene render destination */
    class AKLayout; /* Yoga layout of an AKNode */
    class AKSurface;

    /*********** CORE NODE TYPES ***********/

    class AKNode;       /* Base class for nodes */
    class AKContainer;  /* Container for other nodes. Doesn't produce any output on its own */
    class AKRenderable; /* A node that draws directly into an AKTarget */
    class AKBakeable;   /* A node that draws into its own buffer, which is then blitted into an AKTarget */
    class AKSubScene;   /* A node whose children are drawn into its own buffer and then rendered into an AKTarget */

    /*********** RENDERABLES ***********/

    class AKSolidColor;
    class AKImage;
    class AKPath;

    /*********** BAKEABLES ***********/

    class AKSimpleText;

    /*********** SUBSCENES ***********/

    class AKRoundContainer;

    /************ EFFECTS ************/

    class AKRoundCornersEffect;
    class AKBackgroundEffect;
    class AKBackgroundShadowEffect;
    class AKBackgroundBlurEffect;

    struct AKBlendFunc
    {
        /// Source RGB factor for blending
        GLenum sRGBFactor;

        /// Destination RGB factor for blending
        GLenum dRGBFactor;

        /// Source alpha factor for blending
        GLenum sAlphaFactor;

        /// Destination alpha factor for blendin
        GLenum dAlphaFactor;

        constexpr bool operator==(const AKBlendFunc &other) const
        {
            return sRGBFactor == other.sRGBFactor &&
                   dRGBFactor == other.dRGBFactor &&
                   sAlphaFactor == other.sAlphaFactor &&
                   dAlphaFactor == other.dAlphaFactor;
        }
    };

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
};

#define AK_IRECT_INF SkIRect::MakeLTRB(-2147483, -2147483, 2147483, 2147483)

#endif // AK_H
