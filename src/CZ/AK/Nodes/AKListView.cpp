#include <CZ/AK/Nodes/AKListView.h>

using namespace CZ;

AKListView::AKListView(AKNode *parent) noexcept : AKGridView(parent)
{
    setFillWidth(true);
    setSpacing(0.f, 0.f);

    // Rows are short: a few per image keeps the images (and the draws) as few as a grid's
    setBatchSize(DefaultBatchRows, 0);
    setCellSize(SkSize::Make(1.f, DefaultRowHeight));
}

void AKListView::setRowHeight(SkScalar height) noexcept
{
    setCellSize(SkSize::Make(1.f, height));
}
