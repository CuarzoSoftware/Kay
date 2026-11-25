#ifndef CZ_AKGRIDVIEW_H
#define CZ_AKGRIDVIEW_H

#include <CZ/AK/Nodes/AKScroll.h>
#include <CZ/AK/Nodes/AKSubScene.h>
#include <CZ/AK/Nodes/AKSolidColor.h>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

/**
 * @brief A scrollable grid of equally sized cells that only creates the ones in view.
 * @ingroup AKNodes
 *
 * For long lists and grids (thousands of files, photos...): laying out and walking a node per item
 * would make every scrolled frame slower the more items there are. Here the cells have a fixed
 * size, so where each one goes is plain arithmetic: only the rows in view (plus OverscanRows above
 * and below) have nodes, and those leaving the view are reused for the ones coming in.
 *
 * The app provides the cells and fills them:
 *
 * @code
 * AKGridView grid { parent };
 * grid.setCellSize(SkSize::Make(104, 110));
 * grid.makeCell = []{ return std::make_unique<MyCell>(); };
 * grid.bindCell = [this](AKNode &cell, size_t index){ static_cast<MyCell&>(cell).show(items[index]); };
 * grid.setItemCount(items.size());
 * @endcode
 *
 * Cells are laid out left to right, top to bottom, as many per row as fit (columns()), separated by
 * the spacing and surrounded by the content padding. It scrolls like an AKScroll (kinetic, rubber
 * band, scroll bars).
 *
 * The state shown by a cell (e.g. selected) must come from the app's model: cells are rebound to
 * other items while scrolling (bindCell is called whenever a cell shows another item, and for the
 * visible ones with reloadItems()).
 *
 * The cells are grouped in batches (setBatchSize(): some rows, all their columns or a part of
 * them), each an AKSubScene: its cells are rendered once into the batch's own image, and while
 * scrolling only the batches move (one image each, however many cells they hold). A batch is
 * rendered again only when one of its cells changes. With an opaque row background
 * (setRowBackground(), the color behind the grid) the batches are fully opaque: what's under them
 * isn't painted, and they're copied instead of blended.
 */
class CZ::AKGridView : public AKScroll
{
public:
    /** @brief Creates an empty grid. */
    AKGridView(AKNode *parent = nullptr) noexcept;
    ~AKGridView() noexcept;

    /** @brief Rows created beyond the visible ones (above and below), ready while scrolling. */
    static constexpr size_t OverscanRows { 1 };

    /** @brief Creates a cell (owned by the grid). Called when more cells are needed. */
    std::function<std::unique_ptr<AKNode>()> makeCell;

    /** @brief Shows item @p index in @p cell (a cell created by makeCell). */
    std::function<void(AKNode &cell, size_t index)> bindCell;

    /** @brief Size of every cell (each cell node is given this size). */
    void setCellSize(SkSize size) noexcept;
    SkSize cellSize() const noexcept { return m_cellSize; }

    /** @brief Space between columns and between rows. */
    void setSpacing(SkScalar columns, SkScalar rows) noexcept;

    /** @brief Space around the cells. */
    void setContentPadding(SkScalar padding) noexcept;

    /**
     * @brief Fills each row (its whole width, the spacing below it included) with @p color.
     *
     * Use the color the grid is shown over: with an opaque one the rows are opaque too (faster to
     * draw). std::nullopt (the default): transparent rows.
     */
    void setRowBackground(std::optional<CZAdaptiveColor> color) noexcept;

    /**
     * @brief Cells per batch (see the class description): @p rows rows of @p columns cells
     *        (0: all the columns of the row).
     *
     * Bigger batches mean fewer images to draw while scrolling, but more cells to bind and render
     * at once when one comes into view, and more to render again when one of their cells changes.
     */
    void setBatchSize(size_t rows, size_t columns = 0) noexcept;
    size_t batchRows() const noexcept { return m_batchRows; }
    size_t batchColumns() const noexcept { return m_batchColumns; }

    /** @brief Number of items. The visible cells are bound again (see reloadItems()). */
    void setItemCount(size_t count) noexcept;
    size_t itemCount() const noexcept { return m_count; }

    /** @brief Binds every visible cell again (the items changed). */
    void reloadItems() noexcept;

    /** @brief Binds the cell of @p index again, if it's visible. */
    void reloadItem(size_t index) noexcept;

    /** @brief Cells per row at the current width (at least 1). */
    size_t columns() const noexcept;

    /** @brief Where item @p index is, in content coordinates (not scrolled: 0,0 is the top-left of the content). */
    SkRect cellRect(size_t index) const noexcept;

    /** @brief The item under @p pos (scene coordinates, e.g. the pointer), if any. */
    std::optional<size_t> indexAt(const SkPoint &pos) const noexcept;

    /** @brief The cell showing item @p index, or nullptr if it's not in view. */
    AKNode *cell(size_t index) const noexcept;

    /** @brief The cells in view, by item index. */
    const std::unordered_map<size_t, AKNode*> &visibleCells() const noexcept { return m_visible; }

    /** @brief Scrolls just enough for item @p index to be fully visible (once laid out). */
    void scrollToItem(size_t index) noexcept;

protected:
    void layoutEvent(const CZLayoutEvent &e) override;
    void onSceneBegin() override;

private:
    // The whole content's size: what AKScroll scrolls. Holds the cells.
    class Content final : public AKContainer
    {
    public:
        Content(AKGridView &grid) noexcept;
        AKGridView &grid;
    protected:
        void layoutEvent(const CZLayoutEvent &e) override;
    };

    // A batch of cells (some rows, some columns), rendered into its own image (see the class description)
    class Batch final : public AKSubScene
    {
    public:
        Batch(AKNode *parent) noexcept;
        AKSolidColor background { CZAdaptiveColor(0), this };
        std::vector<std::unique_ptr<AKNode>> cells; // Row by row, `columns` per row
        size_t columns { 0 };                       // Of this batch (the last chunk of a row may have less)
        size_t batchRow { 0 }, chunk { 0 };         // Which one it shows
    };

    void updateCells() noexcept;
    void updateContentSize() noexcept;
    void applyPendingScroll() noexcept;
    size_t chunks() const noexcept;                       // Batches per batch row
    size_t chunkColumns(size_t chunk) const noexcept;     // Columns of that chunk
    void releaseBatch(size_t key) noexcept;
    void bindBatch(Batch &batch, size_t batchRow, size_t chunk) noexcept;
    void dropBatches() noexcept;
    SkScalar contentHeight() const noexcept;

    Content m_content { *this };
    std::vector<std::unique_ptr<Batch>> m_batches;               // Every batch created
    std::unordered_map<size_t, Batch*> m_visibleBatches;         // In use, by batchRow * chunks() + chunk
    std::unordered_map<size_t, std::vector<Batch*>> m_freeBatches; // Hidden, by chunk (they differ in width)
    std::unordered_map<size_t, AKNode*> m_visible;               // Cells in use, by item index
    size_t m_builtColumns { 0 };                                 // Grid columns the batches were built for
    size_t m_batchRows { 1 }, m_batchColumns { 0 };
    std::optional<CZAdaptiveColor> m_rowBackground;
    SkSize m_cellSize { 64.f, 64.f };
    SkScalar m_columnSpacing { 0.f }, m_rowSpacing { 0.f }, m_padding { 0.f };
    size_t m_count { 0 };
    std::optional<size_t> m_scrollTo;
    bool m_inLayout { false };  // Within a layout event: cells are updated on the next scene begin
    bool m_dirty { false };
};

#endif // CZ_AKGRIDVIEW_H
