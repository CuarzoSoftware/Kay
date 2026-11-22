#ifndef CZ_AKFORMSECTION_H
#define CZ_AKFORMSECTION_H

#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Nodes/AKText.h>
#include <CZ/AK/Nodes/AKFormRow.h>
#include <CZ/Core/CZAdaptive.h>

/**
 * @brief A titled group of AKFormRow rows on a rounded card, like macOS's settings.
 * @ingroup AKNodes
 *
 * [title (bold, hidden if empty)]
 * [card: row, separator, row, ...]
 *
 * Rows (or any node) are added to the card: `AKFormRow row { "Name", &section.card };`. The visible
 * rows after the first one show their separator (updated as rows are added, removed, shown or
 * hidden).
 */
class CZ::AKFormSection : public AKContainer
{
public:
    /** @brief Creates the section with @p title (empty: no title). */
    AKFormSection(const std::string &title = "", AKNode *parent = nullptr) noexcept;

    /** @brief Sets the title (empty: hidden). */
    void setTitle(const std::string &title) noexcept;

    /** @brief Shows the visible rows' separators (but the first one's). Called automatically. */
    void updateSeparators() noexcept;

    AKText title { "", this };

    /// The card holding the rows
    class Card : public AKRoundSolidColor
    {
    public:
        Card(AKFormSection &section) noexcept;
    protected:
        void layoutEvent(const CZLayoutEvent &e) override;
        void colorSchemeEvent(const CZColorSchemeEvent &e) override;
    private:
        friend class AKFormSection;
        void updateColors() noexcept;
        AKFormSection &m_section;
    } card { *this };

    static inline CZAdaptiveColor CardColor  { 0x08000000, 0x0DFFFFFF }; ///< The card's fill
    static inline CZAdaptiveColor CardBorder { 0x12000000, 0x14FFFFFF }; ///< The card's hairline
};

#endif // CZ_AKFORMSECTION_H
