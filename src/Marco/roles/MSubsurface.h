#ifndef MSUBSURFACE_H
#define MSUBSURFACE_H

#include <Marco/roles/MSurface.h>

/**
 * @brief Represents a subwindow.
 *
 * This class implements the `wl_subsurface` role as specified by the Wayland protocol.
 *
 * A subsurface must have a mapped parent surface in order to be mapped and can be positioned
 * relative to its top-left corner, see setParent() and setPos().
 *
 * They can be children of any type of surface role, see MSurface::subsurfaces().
 */
class AK::MSubsurface : public MSurface
{
public:

    /**
     * @brief Constructs a subsurface.
     *
     * @param parent The parent surface. If `nullptr`, the subsurface will not be mapped.
     */
    MSubsurface(MSurface *parent = nullptr) noexcept;

    /**
     * @brief Destroys the subsurface.
     */
    ~MSubsurface();

    AKCLASS_NO_COPY(MSubsurface)

    /**
     * @brief Gets the current parent surface.
     *
     * @return A pointer to the parent surface, or `nullptr` if no parent is set.
     */
    MSurface *parent() const noexcept;

    /**
     * @brief Sets the parent surface.
     *
     * Assigns a new parent surface to the subsurface. The parent surface must not
     * be a descendant of the current subsurface, otherwise, this operation is ignored.
     * The subsurface is added to the end of the parent's subsurface list, ensuring it
     * is on top of all its siblings.
     *
     * Setting the parent to `nullptr` unmaps the subsurface and clears the parent relationship.
     *
     * @param surface The new parent surface, or `nullptr` to unset the parent.
     * @return `true` if the operation was successful, `false` otherwise.
     *
     * @see placeAbove()
     * @see placeBelow()
     */
    bool setParent(MSurface *surface) noexcept;

    /**
     * @brief Gets the iterator linking to the parent's subsurface list.
     *
     * The returned iterator is valid only if a parent is currently assigned to this sub-surface.
     *
     * @see MSurface::subsurfaces()
     *
     * @return An iterator referencing the parent's list of subsurfaces.
     */
    const std::list<MSubsurface*>::iterator &parentLink() const noexcept;

    /**
     * @brief Places this subsurface above another sibling subsurface.
     *
     * The specified sibling subsurface must belong to the same parent, see setParent().
     * If `nullptr` is passed, this subsurface will be placed directly above the current parent().
     *
     * @param subSurface The sibling subsurface to place this subsurface above, or nullptr.
     * @return `true` if the operation was successful, `false` otherwise.
     */
    bool placeAbove(MSubsurface *subSurface) noexcept;

    /**
     * @brief Places this subsurface below another sibling subsurface.
     *
     * The specified sibling subsurface must belong to the same parent, see setParent().
     *
     * @param subSurface The sibling subsurface to place this subsurface below.
     * @return `true` if the operation was successful, `false` otherwise.
     */
    bool placeBelow(MSubsurface *subSurface) noexcept;

    /**
     * @brief Sets the position of the subsurface.
     *
     * The position is relative to the top-left corner of the parent surface.
     *
     * @param pos The new position as an SkIPoint.
     */
    void setPos(const SkIPoint &pos) noexcept;

    /**
     * @brief Sets the position of the subsurface.
     *
     * Overload for specifying the position using individual X and Y coordinates.
     *
     * @param x The X-coordinate of the new position.
     * @param y The Y-coordinate of the new position.
     */
    void setPos(Int32 x, Int32 y) noexcept { setPos({x, y}); };

    /**
     * @brief Gets the current position of the subsurface.
     *
     * The position is relative to the top-left corner of the parent surface.
     *
     * @see setPos()
     *
     * @return The current position as an SkIPoint.
     */
    const SkIPoint &pos() const noexcept;

    class Imp;
    Imp *imp() const noexcept;
private:
    std::unique_ptr<Imp> m_imp;
protected:
    void onUpdate() noexcept override;
    void render() noexcept;
};

#endif // MSUBSURFACE_H
