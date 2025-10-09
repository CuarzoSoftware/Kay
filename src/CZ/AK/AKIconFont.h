#ifndef CZ_AKICONFONT_H
#define CZ_AKICONFONT_H

#include <CZ/AK/AKObject.h>
#include <CZ/Ream/Ream.h>
#include <optional>

#include <CZ/skia/modules/skparagraph/src/ParagraphImpl.h>
#include <CZ/skia/modules/skparagraph/src/ParagraphBuilderImpl.h>

/**
 * @brief Utility for rendering and caching icons from an icon font.
 *
 * Loads icons by name from a font family using codepoints, and renders them as images.
 */
class CZ::AKIconFont final : public AKObject
{
public:
    /**
     * @brief Creates an icon font from a font family and codepoint map file.
     *
     * @param fontFamily Font family name (must be available in the font manager).
     * @param codepoints Path to a `.codepoints` file (lines: `<name> <hex codepoint>`).
     * @return Shared pointer to an AKIconFont instance, or `nullptr` on failure.
     */
    static std::shared_ptr<AKIconFont> Make(const std::string& fontFamily, const std::filesystem::path& codepoints) noexcept;

    /**
     * @brief Renders an icon by name at the given size.
     *
     * @param iconName Icon name as defined in the codepoints file (e.g., "account_circle").
     * @param size Output image size in pixels (square).
     * @return Shared pointer to an RImage, or `nullptr` on failure.
     */
    std::shared_ptr<RImage> getIcon(const std::string& iconName, UInt32 size) noexcept;

    /**
     * @brief Converts a Unicode codepoint to a UTF-8 string.
     *
     * Useful for passing codepoints to SkParagraph.
     *
     * @param cp Unicode codepoint (e.g., 0xE853).
     * @return UTF-8 encoded string.
     */
    static std::string UTF8FromCodepoint(UInt32 cp) noexcept;

    /**
     * @brief Parses a codepoint string into a name → UTF-8 map.
     *
     * @param input Multiline string: `<icon_name> <hex_codepoint>` per line.
     * @return Map on success; `std::nullopt` if any line is invalid.
     */
    static std::optional<std::unordered_map<std::string, std::string>> ParseCodepoints(const std::string& input) noexcept;
private:
    // [icon name][size] = image
    std::unordered_map<std::string,
        std::unordered_map<UInt32, std::weak_ptr<RImage>>> m_cache;
    // [icon name][utf8 code point]
    std::optional<std::unordered_map<std::string, std::string>> m_codepoints;
    skia::textlayout::TextStyle m_style;
    skia::textlayout::ParagraphStyle m_paragraphStyle;
    std::unique_ptr<skia::textlayout::ParagraphBuilder> m_builder;
    std::unique_ptr<skia::textlayout::Paragraph> m_paragraph;
    AKIconFont() noexcept = default;
};

#endif // CZ_AKICONFONT_H
