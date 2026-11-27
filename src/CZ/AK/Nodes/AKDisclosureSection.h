#ifndef CZ_AKDISCLOSURESECTION_H
#define CZ_AKDISCLOSURESECTION_H

#include <CZ/AK/Nodes/AKContainer.h>
#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Nodes/AKSolidColor.h>
#include <CZ/AK/Nodes/AKFontIcon.h>
#include <CZ/AK/Nodes/AKText.h>
#include <CZ/Core/CZSignal.h>
#include <string>

/**
 * @brief A titled section the user can collapse and expand, like the ones of Finder's Get Info
 *        window ("General:", "More Info:"...) and of macOS's inspectors.
 * @ingroup AKNodes
 *
 * [separator (a hairline above it)]
 * [header: disclosure triangle, title (bold)]
 * [content (hidden while collapsed)]
 *
 * Nodes are added to content (a column): `AKInfoRow kind { "Kind:", "Folder", &section.content };`.
 * Clicking the header toggles it and emits onToggled. Collapsed, it's just as tall as its header:
 * a window sized to its content can follow (e.g. MSurface::minContentSize() after onToggled).
 *
 * The header highlights while hovered or pressed (AKTheme::ControlHoverFill and
 * AKTheme::ControlPressedFill).
 */
class CZ::AKDisclosureSection : public AKContainer
{
public:
    /** @brief Creates the section with @p title, expanded or not. */
    AKDisclosureSection(const std::string &title = "", bool expanded = true, AKNode *parent = nullptr) noexcept;

    static constexpr SkScalar HeaderHeight   { 24.f };
    static constexpr SkScalar TitleFontSize  { 12.f };
    static constexpr SkScalar ContentPadding { 12.f }; ///< Left and right of the content
    static constexpr SkScalar HeaderInset    { 4.f };  ///< Of the header's highlight, from the edges

    /** @brief Sets the title. */
    void setTitle(const std::string &title) noexcept { titleText.setText(title); }
    const std::string &title() const noexcept { return titleText.text(); }

    /** @brief Shows or hides the content without emitting onToggled. */
    void setExpanded(bool expanded) noexcept;
    bool expanded() const noexcept { return m_expanded; }

    /** @brief Whether the hairline above the header is shown (true by default; e.g. off for the first section). */
    void setShowsSeparator(bool shows) noexcept { separator.setVisible(shows); }
    bool showsSeparator() const noexcept { return separator.visible(); }

    /** @brief Expanded or collapsed by the user (the new state). */
    CZSignal<bool> onToggled;

    AKSolidColor separator { 0, this }; ///< The hairline above the header

    /// The clickable title row
    class Header : public AKRoundSolidColor
    {
    public:
        Header(AKDisclosureSection &section) noexcept;
    protected:
        void pointerEnterEvent(const CZPointerEnterEvent &e) override;
        void pointerLeaveEvent(const CZPointerLeaveEvent &e) override;
        void pointerButtonEvent(const CZPointerButtonEvent &e) override;
        void colorSchemeEvent(const CZColorSchemeEvent &e) override;
    private:
        friend class AKDisclosureSection;
        void updateColors() noexcept;
        AKDisclosureSection &m_section;
        bool m_hover { false }, m_pressed { false };
    } header { *this };

    AKFontIcon arrow { "arrow_drop_down", 18, &header }; ///< The disclosure triangle
    AKText titleText { "", &header };

    /** @brief Where the section's nodes go (a column, hidden while collapsed). */
    AKContainer content { YGFlexDirectionColumn, false, this };

protected:
    /// @cond INTERNAL
    bool m_expanded { true };
    /// @endcond
};

#endif // CZ_AKDISCLOSURESECTION_H
