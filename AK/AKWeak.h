#ifndef AKWEAK_H
#define AKWEAK_H

#include <AK/AK.h>
#include <functional>

class AK::AKWeakUtils
{
public:
    static std::vector<void *> &objectRefs(const AKObject *object) noexcept;
    static bool isObjectDestroyed(const AKObject *object) noexcept;
};

/**
 * @brief Weak reference to an AKObject
 *
 * The AKWeak class template provides a mechanism for creating weak pointer references to subclasses of AKObject.\n
 * It is conceptually similar to `std::weak_ptr` but is specifically tailored for AKObject subclasses, avoiding
 * the pointer indirection and associated performance overhead of the `std::weak_ptr` thread-safe mechanisms.
 *
 * When the object being referenced is destroyed, an optional on destroy callback event is emitted, and get() returns `nullptr`.
 */
template <class T>
class AK::AKWeak
{
public:

    /**
     * Callback function type used to handle the `OnDestroy()` event.
     */
    using OnDestroyCallback = std::function<void(T*)>;

    /**
     * @brief Creates an empty AKWeak
     */
    AKWeak() noexcept = default;

    /**
     * @brief Creates a reference for the given AKObject, or initializes an empty AKWeak if `nullptr` is passed.
     *
     * @param object The AKObject to create a reference for, or `nullptr` to initialize an empty AKWeak.
     */
    AKWeak(T *object) noexcept
    {
        static_assert(std::is_base_of<AKObject, T>::value, "AKWeak template error: T must be a subclass of AKObject.");

        if (object)
            pushBackTo(object);
    }

    /**
     * @brief Destructor, removes the AKWeak from the AKObject references.
     */
    ~AKWeak() noexcept
    {
        clear();

        if (m_onDestroyCallback)
            delete m_onDestroyCallback;
    }

    /**
     * @brief Copy constructor, assigns the AKObject of another AKWeak.
     *
     * @param other The AKWeak object to copy from.
     */
    AKWeak(const AKWeak<T> &other) noexcept
    {
        copy(other);
    }

    /**
     * @brief Assignment operator, assigns the AKObject of another AKWeak.
     *
     * @param other The AKWeak object to assign from.
     * @return Reference to the updated AKWeak object.
     */
    AKWeak<T> &operator=(const AKWeak<T> &other) noexcept
    {
        copy(other);
        return *this;
    }

    /**
     * @brief Gets a pointer to the AKObject or `nullptr` if not set or the object has been destroyed.
     *
     * @return Raw pointer to the referenced AKObject.
     */
    T *get() const noexcept
    {
        return m_object;
    }

    /**
     * @brief Implicit conversion to raw pointer.
     *
     * Provides access to the underlying raw pointer through an implicit conversion.
     *
     * @return A pointer to the underlying object.
     */
    operator T*() const noexcept
    {
        return m_object;
    }

    /**
     * @brief Access underlying object via pointer semantics.
     *
     * Allows accessing members of the underlying object using pointer semantics.
     *
     * @return Pointer to the underlying object.
     */
    T* operator->() const noexcept
    {
        return m_object;
    }

    /**
     * @brief Return the number of existing references to the current AKObject.
     *
     * @return The number of existing references to the current AKObject, if no object is set returns 0.
     */
    UInt64 count() const noexcept
    {
        if (m_object)
        {
            const auto &refs = (std::vector<AKWeak<T>*>&)AKWeakUtils::objectRefs((const AKObject*)m_object);
            return refs.size();
        }

        return 0;
    }

    /**
     * @brief Replace the reference with another object.
     *
     * @param object The AKObject to set as the new reference, or `nullptr` to unset the reference.
     */
    void reset(T *object = nullptr) noexcept
    {
        static_assert(std::is_base_of<AKObject, T>::value, "AKWeak template error: Type must be a subclass of AKObject.");

        clear();

        if (object)
            pushBackTo(object);
    }

    /**
     * @brief Set the onDestroy callback function.
     *
     * @note callback functions are not copied across AKWeak instances.
     *
     * @param callback The callback function to be called when the referenced object is destroyed. Passing `nullptr` disables the callback.
     */
    void setOnDestroyCallback(const OnDestroyCallback &callback) noexcept
    {
        if (m_onDestroyCallback)
        {
            delete m_onDestroyCallback;
            m_onDestroyCallback = nullptr;
        }

        if (callback)
            m_onDestroyCallback = new OnDestroyCallback(callback);
    }

private:
    friend class AKObject;

    void copy(const AKWeak<T> &other) noexcept
    {
        clear();

        if (other.m_object)
            pushBackTo(other.m_object);
    }

    void clear() noexcept
    {
        if (m_object)
        {
            auto &refs = (std::vector<AKWeak<T>*>&)AKWeakUtils::objectRefs((const AKObject*)m_object);
            refs.back()->m_index = m_index;
            refs[m_index] = refs.back();
            refs.pop_back();
            m_object = nullptr;
        }
    }

    void pushBackTo(T *object) noexcept
    {
        if (AKWeakUtils::isObjectDestroyed((const AKObject*)object))
            return;

        m_object = object;
        auto &refs = (std::vector<AKWeak<T>*>&)AKWeakUtils::objectRefs((const AKObject*)m_object);
        refs.push_back(this);
        m_index = refs.size() - 1;
    }

    T *m_object { nullptr };
    UInt64 m_index { 0 };
    OnDestroyCallback *m_onDestroyCallback { nullptr };
};

#endif // AKWEAK_H
