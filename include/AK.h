#ifndef AK_H
#define AK_H

#include <wayland-client.h>
#include <cstdint>

namespace AK
{
    class AKApplication;
    class AKObject;
        class AKPainter;
        class AKSurface;
            class AKToplevel;
            class AKPopup;
        class AKWidget;
            class AKFrame;
                class AKButton;
                class AKLabel;
    class AKRegion;

    AKApplication *app();

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

    /// @brief 24 bits Wayland float
    typedef wl_fixed_t      Float24;

    /// @brief Unsigned integer capable of holding a pointer
    typedef uintptr_t       UIntPtr;

    template <class TA, class TB> class AKPointTemplate;
    template <class TA, class TB> class AKRectTemplate;
    template <class T> class AKBitset;

    /// 2D vector of 32 bits integers
    using AKPoint = AKPointTemplate<Int32,Float32>;

    /// 2D vector of 32 bits integers
    using AKSize = AKPoint;

    /// 2D vector of 32 bits floats
    using AKPointF = AKPointTemplate<Float32,Int32>;

    /// 2D vector of 32 bits floats
    using AKSizeF = AKPointF;

    /// 4D vector of 32 bits integers
    using AKRect = AKRectTemplate<Int32,Float32>;

    /// 4D vector of 32 bits floats
    using AKRectF = AKRectTemplate<Float32,Int32>;

    /**
     * @brief Structure representing a 2D box.
     *
     * The AKBox struct defines a 2D box using four integer coordinates (x1, y1, x2, y2).
     * It is typically used to represent bounding boxes or rectangular regions in 2D space.
     */
    struct AKBox
    {
        /// The x-coordinate of the top-left corner of the box.
        Int32 x1;

        /// The y-coordinate of the top-left corner of the box.
        Int32 y1;

        /// The x-coordinate of the bottom-right corner of the box.
        Int32 x2;

        /// The y-coordinate of the bottom-right corner of the box.
        Int32 y2;
    };

    /**
     * @brief Enumeration for Framebuffer Transformations
     *
     * This enumeration defines different transformations that can be applied to a framebuffer.
     * These transformations include rotations and flips for adjusting the orientation of the framebuffer.
     */
    enum class AKTransform : Int32
    {
        /// No transformation
        Normal = 0,

        /// Rotate 90 degrees counter-clockwise
        Rotated90 = 1,

        /// Rotate 180 degrees counter-clockwise
        Rotated180 = 2,

        /// Rotate 270 degrees counter-clockwise
        Rotated270 = 3,

        /// Flipped (swap left and right sides)
        Flipped = 4,

        /// Flip and rotate 90 degrees counter-clockwise
        Flipped90 = 5,

        /// Flip and rotate 180 degrees counter-clockwise
        Flipped180 = 6,

        /// Flip and rotate 270 degrees counter-clockwise
        Flipped270 = 7
    };


    struct AKColorF
    {
        float r, g, b, a;
    };
}

#endif //AK_H
