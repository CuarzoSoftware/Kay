#ifndef AKOBJECT_H
#define AKOBJECT_H

#include <AK.h>
#include <vector>

/**
 * @brief Base class for AK objects.
 *
 * @see LWeak
 */
class AK::AKObject
{
public:

    /**
     * @brief Copy constructor
     *
     * @note The user data and LWeak references are not copied.
     */
    AKObject(const AKObject &) noexcept
    {
        m_weakRefs.reserve(10);
    }

    /**
     * @brief Assignment operator (each object has its own individual LWeak reference count).
     *
     * @note The user data and LWeak references are not copied.
     */
    AKObject &operator=(const AKObject &) noexcept
    {
        return *this;
    }

    /**
     * @brief Store an unsigned integer value/pointer.
     */
    void setUserData(UIntPtr data) const noexcept
    {
        m_userData = data;
    }

    /**
     * @brief Retrieves the stored unsigned integer value/pointer.
     */
    UIntPtr userData() const noexcept
    {
        return m_userData;
    }

protected:

    /**
     * @brief Constructor of the AKObject class.
     */
    AKObject() noexcept = default;

    /**
     * @brief Destructor of the AKObject class.
     */
    virtual ~AKObject() noexcept;

    /**
     * @brief Notifies the object destruction.
     *
     * This method can be invoked from a subclass destructor to notify the object's imminent destruction
     * to all associated LWeak references in advance. If not invoked, the base AKObject automatically calls it.
     *
     * After invocation, all LWeak references are set to `nullptr`, preventing the creation of additional references for this object.
     */
    void notifyDestruction() noexcept;

private:
    friend class AKWeakUtils;
    mutable std::vector<void*> m_weakRefs;
    mutable UIntPtr m_userData { 0 };
    bool m_destroyed { false };
};

#endif // AKOBJECT_H
