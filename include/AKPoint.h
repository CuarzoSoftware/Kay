#ifndef AKPOINT_H
#define AKPOINT_H

#include <AK.h>

/**
 * @brief Template for 2D vectors
 */
template <class TA, class TB>
class AK::AKPointTemplate
{
public:

    /// Initializes the vector with (0,0).
    constexpr AKPointTemplate() noexcept : m_x(0), m_y(0) {}

    /// Initializes the vector with (x,y).
    constexpr AKPointTemplate(TA x, TA y) noexcept : m_x(x), m_y(y) {}

    /// Initializes the vector with (x,y).
    constexpr AKPointTemplate(TB x, TB y) noexcept : m_x(x), m_y(y) {}

    /// Initializes the vector with (n,n).
    constexpr AKPointTemplate(TA n) noexcept : m_x(n), m_y(n) {}

    /// Initializes the vector with (n,n).
    constexpr AKPointTemplate(TB n) noexcept : m_x(n), m_y(n) {}

    /// Copy constructor
    constexpr AKPointTemplate(const AKPointTemplate<TB,TA> &point) noexcept
    {
        m_x = point.m_x;
        m_y = point.m_y;
    }

    /// First component of the vector
    constexpr TA x() const noexcept { return m_x; }

    /// Second component of the vector
    constexpr TA y() const noexcept { return m_y; }

    /// First component of the vector (equivalent to x())
    constexpr TA w() const noexcept { return m_x; }

    /// Second component of the vector (equivalent to y())
    constexpr TA h() const noexcept { return m_y; }

    /// First component of the vector (equivalent to x())
    constexpr TA width() const noexcept { return m_x; }

    /// Second component of the vector (equivalent to y())
    constexpr TA height() const noexcept { return m_y; }

    /// Product of the components (x*y)
    constexpr TA area() const noexcept { return m_x * m_y; }

    /// Assigns the value to the first component of the vector
    constexpr void setX(TA x) noexcept { m_x = x; }

    /// Assigns the value to the second component of the vector
    constexpr void setY(TA y) noexcept { m_y = y; }

    /// Assigns the value to the first component of the vector (equivalent to setX())
    constexpr void setW(TA x) noexcept { m_x = x; }

    /// Assigns the value to the second component of the vector (equivalent to setY())
    constexpr void setH(TA y) noexcept { m_y = y; }

    /// Assigns the value to the first component of the vector (equivalent to setX())
    constexpr void setWidth(TA x) noexcept { m_x = x; }

    /// Assigns the value to the second component of the vector (equivalent to setY())
    constexpr void setHeight(TA y) noexcept { m_y = y; }

    /// Assigns the second component while maintaining proportion with the first
    constexpr AKPointTemplate<TA, TB> constrainedToHeight(TA size) const noexcept
    {
        AKPointTemplate point;

        if (size == 0 || h() == 0)
            return point;

        point.setW((w() * size) / h());
        point.setH(size);

        return point;
    }

    /// Distance from another point
    constexpr TA distanceFrom(const AKPointTemplate<TA, TB> &point) const noexcept
    {
        return sqrt(pow(x() - point.x(), 2.f) + pow(y() - point.y(), 2.f));
    }

    constexpr AKPointTemplate<TA,TB> &operator+=(TA factor) noexcept
    {
        m_x += factor;
        m_y += factor;
        return *this;
    }

    constexpr AKPointTemplate<TA,TB> &operator-=(TA factor) noexcept
    {
        m_x -= factor;
        m_y -= factor;
        return *this;
    }

    constexpr AKPointTemplate<TA,TB> &operator*=(TA factor) noexcept
    {
        m_x *= factor;
        m_y *= factor;
        return *this;
    }

    constexpr AKPointTemplate<TA,TB> &operator/=(TA factor) noexcept
    {
        m_x /= factor;
        m_y /= factor;
        return *this;
    }

    constexpr AKPointTemplate<TA,TB> &operator+=(TB factor) noexcept
    {
        m_x += factor;
        m_y += factor;
        return *this;
    }

    constexpr AKPointTemplate<TA,TB> &operator-=(TB factor) noexcept
    {
        m_x -= factor;
        m_y -= factor;
        return *this;
    }

    constexpr AKPointTemplate<TA,TB> &operator*=(TB factor) noexcept
    {
        m_x *= factor;
        m_y *= factor;
        return *this;
    }

    constexpr AKPointTemplate<TA,TB> &operator/=(TB factor) noexcept
    {
        m_x /= factor;
        m_y /= factor;
        return *this;
    }

    constexpr AKPointTemplate<TA,TB> &operator+=(const AKPointTemplate<TA,TB> &p) noexcept
    {
        m_x += p.m_x;
        m_y += p.m_y;
        return *this;
    }

    constexpr AKPointTemplate<TA,TB> &operator-=(const AKPointTemplate<TA,TB> &p) noexcept
    {
        m_x -= p.m_x;
        m_y -= p.m_y;
        return *this;
    }

    constexpr AKPointTemplate<TA,TB> &operator*=(const AKPointTemplate<TA,TB> &p) noexcept
    {
        m_x *= p.m_x;
        m_y *= p.m_y;
        return *this;
    }

    constexpr AKPointTemplate<TA,TB> &operator/=(const AKPointTemplate<TA,TB> &p) noexcept
    {
        m_x /= p.m_x;
        m_y /= p.m_y;
        return *this;
    }

    /***************************************************************/

    constexpr AKPointTemplate<TA,TB> &operator+=(const AKPointTemplate<TB,TA> &p) noexcept
    {
        m_x += p.m_x;
        m_y += p.m_y;
        return *this;
    }

    constexpr AKPointTemplate<TA,TB> &operator-=(const AKPointTemplate<TB,TA> &p) noexcept
    {
        m_x -= p.m_x;
        m_y -= p.m_y;
        return *this;
    }

    constexpr AKPointTemplate<TA,TB> &operator*=(const AKPointTemplate<TB,TA> &p) noexcept
    {
        m_x *= p.m_x;
        m_y *= p.m_y;
        return *this;
    }

    constexpr AKPointTemplate<TA,TB> &operator/=(const AKPointTemplate<TB,TA> &p) noexcept
    {
        m_x /= p.m_x;
        m_y /= p.m_y;
        return *this;
    }

    /***************************************************************/

    constexpr AKPointTemplate<TA,TB> operator+(TA factor) const noexcept
    {
        return AKPointTemplate<TA,TB>(m_x+factor,m_y+factor);
    }

    constexpr AKPointTemplate<TA,TB> operator-(TA factor) const noexcept
    {
        return AKPointTemplate<TA,TB>(m_x-factor,m_y-factor);
    }

    constexpr AKPointTemplate<TA,TB> operator*(TA factor) const noexcept
    {
        return AKPointTemplate<TA,TB>(m_x*factor,m_y*factor);
    }

    constexpr AKPointTemplate<TA,TB> operator/(TA factor) const noexcept
    {
        return AKPointTemplate<TA,TB>(m_x/factor,m_y/factor);
    }

    constexpr AKPointTemplate<TA,TB> operator+(TB factor) const noexcept
    {
        return AKPointTemplate<TA,TB>(m_x+factor,m_y+factor);
    }

    constexpr AKPointTemplate<TA,TB> operator-(TB factor) const noexcept
    {
        return AKPointTemplate<TA,TB>(m_x-factor,m_y-factor);
    }

    constexpr AKPointTemplate<TA,TB> operator*(TB factor) const noexcept
    {
        return AKPointTemplate<TA,TB>(m_x*factor,m_y*factor);
    }

    constexpr AKPointTemplate<TA,TB> operator/(TB factor) const noexcept
    {
        return AKPointTemplate<TA,TB>(m_x/factor,m_y/factor);
    }

    constexpr AKPointTemplate<TA,TB> operator+(const AKPointTemplate<TA,TB> &p) const noexcept
    {
        return AKPointTemplate<TA,TB>(m_x+p.m_x,m_y+p.m_y);
    }

    constexpr AKPointTemplate<TA,TB> operator-(const AKPointTemplate<TA,TB> &p) const noexcept
    {
        return AKPointTemplate<TA,TB>(m_x-p.m_x,m_y-p.m_y);
    }

    constexpr AKPointTemplate<TA,TB> operator*(const AKPointTemplate<TA,TB> &p) const noexcept
    {
        return AKPointTemplate<TA,TB>(m_x*p.m_x,m_y*p.m_y);
    }

    constexpr AKPointTemplate<TA,TB> operator/(const AKPointTemplate<TA,TB> &p) const noexcept
    {
        return AKPointTemplate<TA,TB>(m_x/p.m_x,m_y/p.m_y);
    }

    /***************************************************************/

    constexpr AKPointTemplate<TA,TB> operator+(const AKPointTemplate<TB,TA> &p) const noexcept
    {
        return AKPointTemplate(m_x+p.m_x,m_y+p.m_y);
    }

    constexpr AKPointTemplate<TA,TB> operator-(const AKPointTemplate<TB,TA> &p) const noexcept
    {
        return AKPointTemplate(m_x-p.m_x,m_y-p.m_y);
    }

    constexpr AKPointTemplate<TA,TB> operator*(const AKPointTemplate<TB,TA> &p) const noexcept
    {
        return AKPointTemplate(m_x*p.m_x,m_y*p.m_y);
    }

    constexpr AKPointTemplate<TA,TB> operator/(const AKPointTemplate<TB,TA> &p) const noexcept
    {
        return AKPointTemplate(m_x/p.m_x,m_y/p.m_y);
    }

    /***************************************************************/

    constexpr bool operator==(const AKPointTemplate<TA,TB> &p) const noexcept
    {
        return m_x == p.m_x && m_y == p.m_y;
    }

    constexpr bool operator==(const AKPointTemplate<TB,TA> &p) const noexcept
    {
        return m_x == p.m_x && m_y == p.m_y;
    }

    constexpr bool operator!=(const AKPointTemplate<TA,TB> &p) const noexcept
    {
        return m_x != p.m_x || m_y != p.m_y;
    }

    constexpr bool operator!=(const AKPointTemplate<TB,TA> &p) const noexcept
    {
        return m_x != p.m_x || m_y != p.m_y;
    }

private:
    friend class AKRectTemplate<TA,TB>;
    friend class AKPointTemplate<TB,TA>;
    TA m_x = 0;
    TA m_y = 0;
};

constexpr AK::AKPoint operator+(AK::Int32 i, AK::AKPoint p) noexcept
{
    p.setX(p.x()+i);
    p.setY(p.y()+i);
    return p;
}

constexpr AK::AKPoint operator-(AK::Int32 i, AK::AKPoint p) noexcept
{
    p.setX(p.x()-i);
    p.setY(p.y()-i);
    return p;
}

constexpr AK::AKPoint operator*(AK::Int32 i, AK::AKPoint p) noexcept
{
    p.setX(p.x()*i);
    p.setY(p.y()*i);
    return p;
}

constexpr AK::AKPointF operator+(AK::Int32 i, AK::AKPointF p) noexcept
{
    p.setX(p.x()+i);
    p.setY(p.y()+i);
    return p;
}

constexpr AK::AKPointF operator-(AK::Int32 i, AK::AKPointF p) noexcept
{
    p.setX(p.x()-i);
    p.setY(p.y()-i);
    return p;
}

constexpr AK::AKPointF operator*(AK::Int32 i, AK::AKPointF p) noexcept
{
    p.setX(p.x()*i);
    p.setY(p.y()*i);
    return p;
}

#endif // AKPOINT_H
