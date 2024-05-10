#ifndef AKRECT_H
#define AKRECT_H

#include <AKPoint.h>

/**
 * @brief Template for 4D vectors
*/
template <class TA, class TB>
class AK::AKRectTemplate
{
public:

    /// Initializes the vector with (0,0,0,0).
    constexpr AKRectTemplate() noexcept {}

    /// Initializes the vector with (x,y,width,height).
    constexpr AKRectTemplate(TA x, TA y, TA width, TA height) noexcept
    {
        m_topLeft.m_x       = x;
        m_topLeft.m_y       = y;
        m_bottomRight.m_x   = width;
        m_bottomRight.m_y   = height;
    }

    /// Initializes the vector with (topLeft.x(),topLeft.y(),bottomRight.x(),bottomRight.y()).
    constexpr AKRectTemplate(const AKPointTemplate<TA,TB> &topLeft, const AKPointTemplate<TA,TB> &bottomRight) noexcept : m_topLeft(topLeft), m_bottomRight(bottomRight){}

    /// Initializes the vector with (topLeft.x(),topLeft.y(),bottomRight.x(),bottomRight.y()).
    constexpr AKRectTemplate(TA topLeft, const AKPointTemplate<TA,TB> &bottomRight) noexcept : m_topLeft(topLeft), m_bottomRight(bottomRight){}

    /// Initializes the vector with (topLeft.x(),topLeft.y(),bottomRight.x(),bottomRight.y()).
    constexpr AKRectTemplate(const AKPointTemplate<TA,TB> &topLeft, TA bottomRight) noexcept : m_topLeft(topLeft), m_bottomRight(bottomRight){}

    /// Initializes the vector with (all,all,all,all).
    constexpr AKRectTemplate(TA all) noexcept : m_topLeft(all), m_bottomRight(all){}

    /// Initializes the vector with (p.x(),p.y(),p.x(),p.y()).
    constexpr AKRectTemplate(const AKPointTemplate<TA,TB> &p) noexcept : m_topLeft(p), m_bottomRight(p){}

    /// Copy constructor
    constexpr AKRectTemplate(const AKRectTemplate<TB,TA> &r) noexcept
    {
        m_topLeft = r.m_topLeft;
        m_bottomRight = r.m_bottomRight;
    }

    /// First vector component
    constexpr TA x()       const noexcept   {return m_topLeft.m_x;}

    /// Second vector component
    constexpr TA y()       const noexcept   {return m_topLeft.m_y;}

    /// Third vector component
    constexpr TA w()       const noexcept   {return m_bottomRight.m_x;}

    /// Fourth vector component
    constexpr TA h()       const noexcept   {return m_bottomRight.m_y;}

    /// Third vector component
    constexpr TA width()   const noexcept   {return m_bottomRight.m_x;}

    /// Fourth vector component
    constexpr TA height()  const noexcept   {return m_bottomRight.m_y;}

    /// The multiplication of the third and fourth component (width*height)
    constexpr TA area()    const noexcept   {return m_bottomRight.m_x*m_bottomRight.m_y;}

    /**
     * Returns true if the rectangle contains the point.
     *
     * @param point The 2D vector to check
     * @param inclusive If true, the edges of the rectangle are considered as part of the rectangle.
     */
    constexpr bool containsPoint(const AKPointTemplate<TA,TB> &point, bool inclusive = true) const noexcept
    {
        if(inclusive)
        {
            return point.m_x >= m_topLeft.m_x &&
                   point.m_x <= m_topLeft.m_x + m_bottomRight.m_x &&
                   point.m_y >= m_topLeft.m_y &&
                   point.m_y <= m_topLeft.m_y + m_bottomRight.m_y;
        }

        return point.m_x > m_topLeft.m_x &&
               point.m_x < m_topLeft.m_x + m_bottomRight.m_x &&
               point.m_y > m_topLeft.m_y &&
               point.m_y < m_topLeft.m_y + m_bottomRight.m_y;
    }

    /**
     * Returns true if the rectangles intersect.
     *
     * @param rect Rectangle to intersect
     * @param inclusive If true, the edges of the rectangle are considered
     */
    constexpr bool intersects(const AKRectTemplate &rect, bool inclusive = true) const noexcept
    {
        if(inclusive)
        {
            return x() <= rect.x()+rect.w() &&
                   x() + w() >= rect.x() &&
                   y() <= rect.y()+rect.h() &&
                   y()+h() >= rect.y();
        }

        return x() < rect.x()+rect.w() &&
               x() + w() > rect.x() &&
               y() < rect.y()+rect.h() &&
               y()+h() > rect.y();
    }

    /// 2D vector given by the (x,y) components of the rectangle
    constexpr const AKPointTemplate<TA,TB> &topLeft() const noexcept {return m_topLeft;}

    /// 2D vector given by the (w,h) components of the rectangle
    constexpr const AKSize &bottomRight() const noexcept {return m_bottomRight;}

    /// 2D vector given by the (x,y) components of the rectangle
    constexpr const AKPointTemplate<TA,TB> &TL() const noexcept {return m_topLeft;}

    /// 2D vector given by the (w,h) components of the rectangle
    constexpr const AKSize &BR() const noexcept {return m_bottomRight;}

    /// 2D vector given by the (x,y) components of the rectangle
    constexpr const AKPointTemplate<TA,TB> &pos() const noexcept {return m_topLeft;}

    /// 2D vector given by the (w,h) components of the rectangle
    constexpr const AKSize &size() const noexcept {return m_bottomRight;}

    /// Asigns the first component
    constexpr void setX(TA x) noexcept {m_topLeft.m_x = x;}

    /// Asigns the second component
    constexpr void setY(TA y) noexcept {m_topLeft.m_y = y;}

    /// Asigns the third component
    constexpr void setW(TA width) noexcept {m_bottomRight.m_x = width;}

    /// Asigns the fourth component
    constexpr void setH(TA height) noexcept {m_bottomRight.m_y = height;}

    /// Asigns the third component
    constexpr void setWidth(TA width) noexcept {m_bottomRight.m_x = width;}

    /// Asigns the fourth component
    constexpr void setHeight(TA height) noexcept {m_bottomRight.m_y = height;}

    /// Asigns the (x,y) components
    constexpr void setTL(const AKPointTemplate<TA,TB> &p) noexcept {m_topLeft = p;}

    /// Asigns the (w,h) components
    constexpr void setBR(const AKPointTemplate<TA,TB> &p) noexcept {m_bottomRight = p;}

    /// Asigns the (x,y) components
    constexpr void setTL(const AKPointTemplate<TB,TA> &p) noexcept {m_topLeft = p;}

    /// Asigns the (w,h) components
    constexpr void setBR(const AKPointTemplate<TB,TA> &p) noexcept {m_bottomRight = p;}

    /// Asigns the (x,y) components
    constexpr void setTopLeft(const AKPointTemplate<TA,TB> &p) noexcept {m_topLeft = p;}

    /// Asigns the (w,h) components
    constexpr void setBottomRight(const AKPointTemplate<TA,TB> &p) noexcept {m_bottomRight = p;}

    /// Asigns the (x,y) components
    constexpr void setTopLeft(const AKPointTemplate<TB,TA> &p) noexcept {m_topLeft = p;}

    /// Asigns the (w,h) components
    constexpr void setBottomRight(const AKPointTemplate<TB,TA> &p) noexcept {m_bottomRight = p;}

    /// Asigns the (x,y) components
    constexpr void setPos(const AKPointTemplate<TA,TB> &p) noexcept {m_topLeft = p;}

    /// Asigns the (w,h) components
    constexpr void setSize(const AKPointTemplate<TA,TB> &p) noexcept {m_bottomRight = p;}

    /// Asigns the (x,y) components
    constexpr void setPos(const AKPointTemplate<TB,TA> &p) noexcept {m_topLeft = p;}

    /// Asigns the (w,h) components
    constexpr void setSize(const AKPointTemplate<TB,TA> &p) noexcept {m_bottomRight = p;}

    /// Returns true if the resulting rectangle has an area of 0.
    constexpr bool clip(const AKRectTemplate &rect) noexcept
    {
        TA x0 = x();
        TA x1 = x0 + w();
        TA y0 = y();
        TA y1 = y0 + h();

        TA rx1 = rect.x() + rect.w();
        TA ry1 = rect.y() + rect.h();

        // X
        if(rect.x() > x0)
            x0 = rect.x();
        else if(rx1 < x0)
            x0 = rx1;

        // W
        if(rect.x() > x1)
            x1 = rect.x();
        else if(rx1 < x1)
            x1 = rx1;

        // Y
        if(rect.y() > y0)
            y0 = rect.y();
        else if(ry1 < y0)
            y0 = ry1;

        // H
        if(rect.y() > y1)
            y1 = rect.y();
        else if(ry1 < y1)
            y1 = ry1;

        setX(x0);
        setY(y0);
        setW(x1 - x0);
        setH(y1 - y0);

        return (w() == 0 || h() == 0 );
    }

    constexpr AKRectTemplate &operator+=(TA factor) noexcept
    {
        m_topLeft.m_x += factor;
        m_topLeft.m_y += factor;
        m_bottomRight.m_x += factor;
        m_bottomRight.m_y += factor;
        return *this;
    }

    constexpr AKRectTemplate &operator-=(TA factor) noexcept
    {
        m_topLeft.m_x -= factor;
        m_topLeft.m_y -= factor;
        m_bottomRight.m_x -= factor;
        m_bottomRight.m_y -= factor;
        return *this;
    }

    constexpr AKRectTemplate &operator*=(TA factor) noexcept
    {
        m_topLeft.m_x *= factor;
        m_topLeft.m_y *= factor;
        m_bottomRight.m_x *= factor;
        m_bottomRight.m_y *= factor;
        return *this;
    }

    constexpr AKRectTemplate &operator/=(TA factor) noexcept
    {
        m_topLeft.m_x /= factor;
        m_topLeft.m_y /= factor;
        m_bottomRight.m_x /= factor;
        m_bottomRight.m_y /= factor;
        return *this;
    }

    constexpr AKRectTemplate &operator+=(TB factor) noexcept
    {
        m_topLeft.m_x += factor;
        m_topLeft.m_y += factor;
        m_bottomRight.m_x += factor;
        m_bottomRight.m_y += factor;
        return *this;
    }

    constexpr AKRectTemplate &operator-=(TB factor) noexcept
    {
        m_topLeft.m_x -= factor;
        m_topLeft.m_y -= factor;
        m_bottomRight.m_x -= factor;
        m_bottomRight.m_y -= factor;
        return *this;
    }

    constexpr AKRectTemplate &operator*=(TB factor) noexcept
    {
        m_topLeft.m_x *= factor;
        m_topLeft.m_y *= factor;
        m_bottomRight.m_x *= factor;
        m_bottomRight.m_y *= factor;
        return *this;
    }

    constexpr AKRectTemplate &operator/=(TB factor) noexcept
    {
        m_topLeft.m_x /= factor;
        m_topLeft.m_y /= factor;
        m_bottomRight.m_x /= factor;
        m_bottomRight.m_y /= factor;
        return *this;
    }

    constexpr AKRectTemplate &operator+=(const AKRectTemplate &r) noexcept
    {
        m_topLeft.m_x += r.m_topLeft.m_x;
        m_topLeft.m_y += r.m_topLeft.m_y;
        m_bottomRight.m_x += r.m_bottomRight.m_x;
        m_bottomRight.m_y += r.m_bottomRight.m_y;
        return *this;
    }

    constexpr AKRectTemplate &operator-=(const AKRectTemplate &r) noexcept
    {
        m_topLeft.m_x -= r.m_topLeft.m_x;
        m_topLeft.m_y -= r.m_topLeft.m_y;
        m_bottomRight.m_x -= r.m_bottomRight.m_x;
        m_bottomRight.m_y -= r.m_bottomRight.m_y;
        return *this;
    }

    constexpr AKRectTemplate &operator*=(const AKRectTemplate &r) noexcept
    {
        m_topLeft.m_x *= r.m_topLeft.m_x;
        m_topLeft.m_y *= r.m_topLeft.m_y;
        m_bottomRight.m_x *= r.m_bottomRight.m_x;
        m_bottomRight.m_y *= r.m_bottomRight.m_y;
        return *this;
    }

    constexpr AKRectTemplate &operator/=(const AKRectTemplate &r) noexcept
    {
        m_topLeft.m_x /= r.m_topLeft.m_x;
        m_topLeft.m_y /= r.m_topLeft.m_y;
        m_bottomRight.m_x /= r.m_bottomRight.m_x;
        m_bottomRight.m_y /= r.m_bottomRight.m_y;
        return *this;
    }

    constexpr AKRectTemplate operator+(TA factor) const noexcept
    {
        return AKRectTemplate(m_topLeft.m_x+factor,m_topLeft.m_y+factor,m_bottomRight.m_x+factor,m_bottomRight.m_y+factor);
    }

    constexpr AKRectTemplate operator-(TA factor) const noexcept
    {
        return AKRectTemplate(m_topLeft.m_x-factor,m_topLeft.m_y-factor,m_bottomRight.m_x-factor,m_bottomRight.m_y-factor);
    }

    constexpr AKRectTemplate operator*(TA factor) const noexcept
    {
        return AKRectTemplate(m_topLeft.m_x*factor,m_topLeft.m_y*factor,m_bottomRight.m_x*factor,m_bottomRight.m_y*factor);
    }

    constexpr AKRectTemplate operator/(TA factor) const noexcept
    {
        return AKRectTemplate(m_topLeft.m_x/factor,m_topLeft.m_y/factor,m_bottomRight.m_x/factor,m_bottomRight.m_y/factor);
    }

    constexpr AKRectTemplate operator+(TB factor) const noexcept
    {
        return AKRectTemplate(m_topLeft.m_x+factor,m_topLeft.m_y+factor,m_bottomRight.m_x+factor,m_bottomRight.m_y+factor);
    }

    constexpr AKRectTemplate operator-(TB factor) const noexcept
    {
        return AKRectTemplate(m_topLeft.m_x-factor,m_topLeft.m_y-factor,m_bottomRight.m_x-factor,m_bottomRight.m_y-factor);
    }

    constexpr AKRectTemplate operator*(TB factor) const noexcept
    {
        return AKRectTemplate(m_topLeft.m_x*factor,m_topLeft.m_y*factor,m_bottomRight.m_x*factor,m_bottomRight.m_y*factor);
    }

    constexpr AKRectTemplate operator/(TB factor) const noexcept
    {
        return AKRectTemplate(m_topLeft.m_x/factor,m_topLeft.m_y/factor,m_bottomRight.m_x/factor,m_bottomRight.m_y/factor);
    }

    constexpr AKRectTemplate operator+(const AKRectTemplate &r) const noexcept
    {
        return AKRectTemplate(m_topLeft.m_x+r.m_topLeft.m_x,m_topLeft.m_y+r.m_topLeft.m_y,m_bottomRight.m_x+r.m_bottomRight.m_x,m_bottomRight.m_y+r.m_bottomRight.m_y);
    }

    constexpr AKRectTemplate operator-(const AKRectTemplate &r) const noexcept
    {
        return AKRectTemplate(m_topLeft.m_x-r.m_topLeft.m_x,m_topLeft.m_y-r.m_topLeft.m_y,m_bottomRight.m_x-r.m_bottomRight.m_x,m_bottomRight.m_y-r.m_bottomRight.m_y);
    }

    constexpr AKRectTemplate operator*(const AKRectTemplate &r) const noexcept
    {
        return AKRectTemplate(m_topLeft.m_x*r.m_topLeft.m_x,m_topLeft.m_y*r.m_topLeft.m_y,m_bottomRight.m_x*r.m_bottomRight.m_x,m_bottomRight.m_y*r.m_bottomRight.m_y);
    }

    constexpr AKRectTemplate operator/(const AKRectTemplate &r) const noexcept
    {
        return AKRectTemplate(m_topLeft.m_x/r.m_topLeft.m_x,m_topLeft.m_y/r.m_topLeft.m_y,m_bottomRight.m_x/r.m_bottomRight.m_x,m_bottomRight.m_y/r.m_bottomRight.m_y);
    }

    constexpr bool operator==(const AKRectTemplate &p) const noexcept
    {
        return m_topLeft.m_x == p.m_topLeft.m_x && m_topLeft.m_y == p.m_topLeft.m_y && m_bottomRight.m_x == p.m_bottomRight.m_x && m_bottomRight.m_y == p.m_bottomRight.m_y;
    }

    constexpr bool operator!=(const AKRectTemplate &p) const noexcept
    {
        return m_topLeft.m_x != p.m_topLeft.m_x || m_topLeft.m_y != p.m_topLeft.m_y || m_bottomRight.m_x != p.m_bottomRight.m_x || m_bottomRight.m_y != p.m_bottomRight.m_y;
    }

    /*******************************************************************/

    constexpr AKRectTemplate &operator+=(const AKPointTemplate<TA,TB> &p) noexcept
    {
        m_topLeft += p;
        m_bottomRight += p;
        return *this;
    }

    constexpr AKRectTemplate &operator-=(const AKPointTemplate<TA,TB> &p) noexcept
    {
        m_topLeft -= p;
        m_bottomRight -= p;
        return *this;
    }

    constexpr AKRectTemplate &operator*=(const AKPointTemplate<TA,TB> &p) noexcept
    {
        m_topLeft *= p;
        m_bottomRight *= p;
        return *this;
    }

    constexpr AKRectTemplate &operator/=(const AKPointTemplate<TA,TB> &p) noexcept
    {
        m_topLeft /= p;
        m_bottomRight /= p;
        return *this;
    }

private:
    friend class AKRectTemplate<TB,TA>;
    AKPointTemplate<TA,TB> m_topLeft,m_bottomRight;
};

#endif // AKRECT_H
