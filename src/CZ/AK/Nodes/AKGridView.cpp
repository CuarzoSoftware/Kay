#include <CZ/AK/Nodes/AKGridView.h>
#include <CZ/Core/Events/CZLayoutEvent.h>
#include <algorithm>
#include <cmath>

using namespace CZ;

AKGridView::Content::Content(AKGridView &grid) noexcept :
    AKContainer(YGFlexDirectionColumn, false, &grid),
    grid(grid)
{
    layout().setWidthPercent(100.f);
    layout().setFlexShrink(0.f);

    // AKScroll measures its content through it alone (not the cells: as if they weren't there)
    enableChildrenClipping(true);
}

void AKGridView::Content::layoutEvent(const CZLayoutEvent &e)
{
    AKContainer::layoutEvent(e);

    // Its new height laid out: the scroll range follows, then a pending scroll can be done
    if (e.changes.has(CZLayoutChangeSize))
    {
        grid.m_inLayout = true;
        grid.calculateContentBounds();
        grid.applyConstraints();
        grid.m_inLayout = false;
        grid.m_dirty = true;
        grid.repaint();
    }
}

AKGridView::AKGridView(AKNode *parent) noexcept : AKScroll(parent)
{
    onOffsetChanged.subscribe(this, [this]{
        // Moved by the user or an animation: right away. During a layout pass: once it's done.
        if (m_inLayout)
        {
            m_dirty = true;
            repaint();
        }
        else
            updateCells();
    });
}

AKGridView::Batch::Batch(AKNode *parent) noexcept : AKSubScene(parent)
{
    layout().setPositionType(YGPositionTypeAbsolute);
    background.layout().setPositionType(YGPositionTypeAbsolute);
    background.layout().setWidthPercent(100.f);
    background.layout().setHeightPercent(100.f);
    const SkRegion empty;
    background.setInputRegion(&empty);
}

AKGridView::~AKGridView() noexcept
{
    m_visible.clear();
    m_visibleBatches.clear();
    m_freeBatches.clear();
    m_batches.clear();
}
void AKGridView::setCellSize(SkSize size) noexcept
{
    size.fWidth = std::max(1.f, size.width());
    size.fHeight = std::max(1.f, size.height());

    if (size == m_cellSize)
        return;

    m_cellSize = size;
    dropBatches();
    updateContentSize();
    updateCells();
}

void AKGridView::setSpacing(SkScalar columns, SkScalar rows) noexcept
{
    m_columnSpacing = std::max(0.f, columns);
    m_rowSpacing = std::max(0.f, rows);
    dropBatches();
    updateContentSize();
    updateCells();
}

void AKGridView::setContentPadding(SkScalar padding) noexcept
{
    m_padding = std::max(0.f, padding);
    dropBatches();
    updateContentSize();
    updateCells();
}

void AKGridView::setRowBackground(std::optional<CZAdaptiveColor> color) noexcept
{
    m_rowBackground = color;

    for (auto &batch : m_batches)
        batch->background.setColor(color ? *color : CZAdaptiveColor(0));
}

void AKGridView::setBatchSize(size_t rows, size_t columns) noexcept
{
    rows = std::max<size_t>(1, rows);

    if (rows == m_batchRows && columns == m_batchColumns)
        return;

    m_batchRows = rows;
    m_batchColumns = columns;
    dropBatches();
    updateCells();
}

void AKGridView::setItemCount(size_t count) noexcept
{
    m_count = count;

    if (m_scrollTo && *m_scrollTo >= count)
        m_scrollTo.reset();

    updateContentSize();
    reloadItems();
    updateCells();
}

void AKGridView::reloadItems() noexcept
{
    for (const auto &[key, batch] : m_visibleBatches)
        bindBatch(*batch, batch->batchRow, batch->chunk);
}

void AKGridView::reloadItem(size_t index) noexcept
{
    if (AKNode *c { cell(index) }; c && bindCell)
        bindCell(*c, index);
}

size_t AKGridView::columns() const noexcept
{
    const SkScalar width { layout().calculatedWidth() - 2.f * m_padding };
    const SkScalar step { m_cellSize.width() + m_columnSpacing };
    return std::max<size_t>(1, size_t(std::floor((width + m_columnSpacing) / step)));
}

SkScalar AKGridView::contentHeight() const noexcept
{
    const size_t cols { columns() };
    const size_t rows { (m_count + cols - 1) / cols };
    return 2.f * m_padding + rows * m_cellSize.height() + (rows > 0 ? (rows - 1) * m_rowSpacing : 0.f);
}

SkRect AKGridView::cellRect(size_t index) const noexcept
{
    const size_t cols { columns() };
    const size_t row { index / cols }, col { index % cols };
    return SkRect::MakeXYWH(m_padding + col * (m_cellSize.width() + m_columnSpacing),
                            m_padding + row * (m_cellSize.height() + m_rowSpacing),
                            m_cellSize.width(), m_cellSize.height());
}

std::optional<size_t> AKGridView::indexAt(const SkPoint &pos) const noexcept
{
    if (!SkRect::Make(worldRect()).contains(pos.x(), pos.y()))
        return std::nullopt;

    // Content coordinates
    const SkScalar x { pos.x() - m_content.worldRect().x() - m_padding };
    const SkScalar y { pos.y() - m_content.worldRect().y() - m_padding };

    if (x < 0.f || y < 0.f)
        return std::nullopt;

    const SkScalar stepX { m_cellSize.width() + m_columnSpacing }, stepY { m_cellSize.height() + m_rowSpacing };
    const size_t col { size_t(x / stepX) }, row { size_t(y / stepY) };

    // In the spacing between cells
    if (col >= columns() || x - col * stepX >= m_cellSize.width() || y - row * stepY >= m_cellSize.height())
        return std::nullopt;

    const size_t index { row * columns() + col };

    if (index >= m_count)
        return std::nullopt;

    return index;
}

AKNode *AKGridView::cell(size_t index) const noexcept
{
    const auto it { m_visible.find(index) };
    return it == m_visible.end() ? nullptr : it->second;
}

void AKGridView::scrollToItem(size_t index) noexcept
{
    if (index >= m_count)
        return;

    m_scrollTo = index;
    applyPendingScroll();
}

void AKGridView::applyPendingScroll() noexcept
{
    if (!m_scrollTo)
        return;

    // Laid out at its full height first (the scroll range follows the content's size)
    const SkScalar viewHeight { layout().calculatedHeight() };

    if (viewHeight <= 0.f || std::abs(m_content.layout().calculatedHeight() - contentHeight()) > 0.5f)
    {
        repaint();
        return;
    }

    const SkRect rect { cellRect(*m_scrollTo) };
    m_scrollTo.reset();

    const SkScalar top { -offsetY() };

    if (rect.top() - m_rowSpacing < top)
        setOffsetY(Int32(-(rect.top() - std::min(m_rowSpacing, m_padding))));
    else if (rect.bottom() + m_rowSpacing > top + viewHeight)
        setOffsetY(Int32(-(rect.bottom() + std::min(m_rowSpacing, m_padding) - viewHeight)));
}

void AKGridView::updateContentSize() noexcept
{
    m_content.layout().setHeight(contentHeight());
}

size_t AKGridView::chunks() const noexcept
{
    const size_t cols { m_builtColumns };
    const size_t per { m_batchColumns == 0 ? cols : std::min(m_batchColumns, cols) };
    return per == 0 ? 1 : (cols + per - 1) / per;
}

size_t AKGridView::chunkColumns(size_t chunk) const noexcept
{
    const size_t cols { m_builtColumns };
    const size_t per { m_batchColumns == 0 ? cols : std::min(m_batchColumns, cols) };
    return std::min(per, cols - chunk * per);
}

void AKGridView::releaseBatch(size_t key) noexcept
{
    auto it { m_visibleBatches.find(key) };

    if (it == m_visibleBatches.end())
        return;

    Batch *batch { it->second };
    const size_t cols { m_builtColumns };
    const size_t per { m_batchColumns == 0 ? cols : std::min(m_batchColumns, cols) };

    // Its items (as bound, see bindBatch())
    for (size_t r = 0; r < m_batchRows; r++)
        for (size_t c = 0; c < batch->columns; c++)
            m_visible.erase((batch->batchRow * m_batchRows + r) * cols + batch->chunk * per + c);

    batch->setVisible(false);
    m_freeBatches[batch->chunk].push_back(batch);
    m_visibleBatches.erase(it);
}

void AKGridView::dropBatches() noexcept
{
    // Built for other metrics (columns, cell size...): made again as needed
    m_visible.clear();
    m_visibleBatches.clear();
    m_freeBatches.clear();
    m_batches.clear();
    m_builtColumns = 0;
}

void AKGridView::bindBatch(Batch &batch, size_t batchRow, size_t chunk) noexcept
{
    batch.batchRow = batchRow;
    batch.chunk = chunk;

    const size_t cols { m_builtColumns };
    const size_t per { m_batchColumns == 0 ? cols : std::min(m_batchColumns, cols) };
    const SkScalar stepX { m_cellSize.width() + m_columnSpacing }, stepY { m_cellSize.height() + m_rowSpacing };

    // Covers its part of the rows: the first chunk from the left edge (padding included), the
    // last one to the right edge, the spacing below the cells included (opaque batches tile the content)
    const SkScalar x { chunk == 0 ? 0.f : m_padding + chunk * per * stepX };
    batch.layout().setPosition(YGEdgeLeft, x);
    batch.layout().setPosition(YGEdgeTop, m_padding + batchRow * m_batchRows * stepY);
    batch.layout().setHeight(m_batchRows * stepY);

    if (chunk + 1 == chunks())
        batch.layout().setWidth(std::max(1.f, layout().calculatedWidth() - x));
    else
        batch.layout().setWidth((chunk == 0 ? m_padding : 0.f) + per * stepX);

    for (size_t r = 0; r < m_batchRows; r++)
        for (size_t c = 0; c < batch.columns; c++)
        {
            AKNode &cell { *batch.cells[r * batch.columns + c] };
            const size_t row { batchRow * m_batchRows + r };
            const size_t item { row * cols + chunk * per + c };

            // Past the last item
            if (item >= m_count || chunk * per + c >= cols)
            {
                cell.setVisible(false);
                continue;
            }

            cell.setVisible(true);
            cell.layout().setPosition(YGEdgeLeft, m_padding + (chunk * per + c) * stepX - x);
            cell.layout().setPosition(YGEdgeTop, r * stepY);
            m_visible[item] = &cell;

            if (bindCell)
                bindCell(cell, item);
        }
}

void AKGridView::layoutEvent(const CZLayoutEvent &e)
{
    // The width changes the columns (and so the content's height)
    m_inLayout = true;
    AKScroll::layoutEvent(e);
    m_inLayout = false;

    if (e.changes.has(CZLayoutChangeSize))
    {
        updateContentSize();
        m_dirty = true;
        repaint();
    }
}

void AKGridView::onSceneBegin()
{
    AKScroll::onSceneBegin();

    if (!m_dirty)
        return;

    m_dirty = false;
    updateCells();
    applyPendingScroll();

    // New cells are laid out on the next frame
    repaint();
}

void AKGridView::updateCells() noexcept
{
    const SkScalar viewHeight { layout().calculatedHeight() };

    if (m_count == 0 || viewHeight <= 0.f || layout().calculatedWidth() <= 0.f || !makeCell)
    {
        for (auto it = m_visibleBatches.begin(); it != m_visibleBatches.end(); it = m_visibleBatches.begin())
            releaseBatch(it->first);

        return;
    }

    // Another number of columns: batches are made again
    const size_t cols { columns() };

    if (cols != m_builtColumns)
    {
        dropBatches();
        m_builtColumns = cols;
    }

    // The rows in view (and a few more), in batch rows
    const size_t rows { (m_count + cols - 1) / cols };
    const SkScalar stepY { m_cellSize.height() + m_rowSpacing };
    const SkScalar top { -offsetY() - m_padding };
    const Int64 firstRow { std::max<Int64>(0, Int64(std::floor(top / stepY)) - Int64(OverscanRows)) };
    const Int64 lastRow { std::min<Int64>(Int64(rows) - 1, Int64(std::floor((top + viewHeight) / stepY)) + Int64(OverscanRows)) };
    const Int64 firstBatchRow { firstRow / Int64(m_batchRows) };
    const Int64 lastBatchRow { lastRow < 0 ? -1 : lastRow / Int64(m_batchRows) };
    const size_t chunkCount { chunks() };

    // Those out of view become free
    std::vector<size_t> out;

    for (const auto &[key, batch] : m_visibleBatches)
        if (Int64(batch->batchRow) < firstBatchRow || Int64(batch->batchRow) > lastBatchRow)
            out.push_back(key);

    for (size_t key : out)
        releaseBatch(key);

    for (Int64 br = firstBatchRow; br <= lastBatchRow; br++)
        for (size_t chunk = 0; chunk < chunkCount; chunk++)
        {
            const size_t key { size_t(br) * chunkCount + chunk };

            if (m_visibleBatches.contains(key))
                continue;

            Batch *batch;
            auto &free { m_freeBatches[chunk] };

            if (!free.empty())
            {
                batch = free.back();
                free.pop_back();
            }
            else
            {
                auto &created { m_batches.emplace_back(std::make_unique<Batch>(&m_content)) };
                batch = created.get();
                batch->background.setColor(m_rowBackground ? *m_rowBackground : CZAdaptiveColor(0));
                batch->columns = chunkColumns(chunk);

                for (size_t i = 0; i < m_batchRows * batch->columns; i++)
                {
                    auto &cell { batch->cells.emplace_back(makeCell()) };
                    cell->setParent(batch);
                    cell->layout().setPositionType(YGPositionTypeAbsolute);
                    cell->layout().setWidth(m_cellSize.width());
                    cell->layout().setHeight(m_cellSize.height());
                }
            }

            m_visibleBatches[key] = batch;
            batch->setVisible(true);
            bindBatch(*batch, size_t(br), chunk);
        }
}
