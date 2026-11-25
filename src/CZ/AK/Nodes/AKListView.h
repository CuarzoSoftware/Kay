#ifndef CZ_AKLISTVIEW_H
#define CZ_AKLISTVIEW_H

#include <CZ/AK/Nodes/AKGridView.h>

/**
 * @brief A scrollable list of equally tall rows that only creates the ones in view.
 * @ingroup AKNodes
 *
 * The list version of AKGridView (it is one, with a single column): each row is a cell as wide as
 * the list (minus the content padding), DefaultRowHeight tall by default. Like the grid, only the
 * rows in view exist, they're reused while scrolling, and they're baked in batches (several rows per
 * image, see AKGridView::setBatchSize()).
 *
 * @code
 * AKListView list { parent };
 * list.setRowColors(std::pair { evenColor, oddColor }); // Optional stripes, like macOS's lists
 * list.makeCell = []{ return std::make_unique<MyRow>(); };
 * list.bindCell = [this](AKNode &row, size_t index){ static_cast<MyRow&>(row).show(items[index]); };
 * list.setItemCount(items.size());
 * @endcode
 *
 * The row cells have no width set: they stretch between the content padding.
 */
class CZ::AKListView : public AKGridView
{
public:
    /** @brief Creates an empty list. */
    AKListView(AKNode *parent = nullptr) noexcept;

    static constexpr SkScalar DefaultRowHeight { 24.f };
    static constexpr size_t   DefaultBatchRows { 6 };

    /** @brief Height of every row. */
    void setRowHeight(SkScalar height) noexcept;
    SkScalar rowHeight() const noexcept { return cellSize().height(); }
};

#endif // CZ_AKLISTVIEW_H
