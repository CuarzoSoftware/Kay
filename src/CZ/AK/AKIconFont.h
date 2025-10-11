#ifndef CZ_AKICONFONT_H
#define CZ_AKICONFONT_H

#include <CZ/AK/AKObject.h>
#include <CZ/Ream/Ream.h>
#include <optional>

#include <CZ/skia/modules/skparagraph/src/ParagraphImpl.h>
#include <CZ/skia/modules/skparagraph/src/ParagraphBuilderImpl.h>

/**
 * @brief A utility class for rendering and caching icons from an icon font.
 *
 * This class allows loading icons by name or codepoint from a font family and renders them as images.
 * It also handles caching and efficient retrieval of icons.
 */
class CZ::AKIconFont final : public AKObject
{
public:
    /**
     * @brief Creates an icon font from a font family and an optional codepoint map.
     *
     * This method loads an icon font based on the specified font family. Optionally, it can load a codepoint map from a file.
     * If the codepoint map fails to load, or the font is not found, the icon font is still created with a fallback font.
     *
     * Use `hasCodepointMap()` to check if the map was successfully loaded.
     *
     * @param fontFamily The name of the font family.
     * @param codepointsPath Path to a `.codepoints` file containing icon name and hex codepoint pairs.
     * @return A shared pointer to an AKIconFont instance, or `nullptr` on failure.
     */
    static std::shared_ptr<AKIconFont> Make(const std::string &fontFamily, const std::filesystem::path &codepointsPath) noexcept;

    /**
     * @brief Creates an icon font from a font family and a string-based codepoint map.
     *
     * This method is an alternative to loading the codepoint map from a file. It uses a direct string input for the codepoints.
     *
     * @param fontFamily The name of the font family.
     * @param codepoints Optional string containing the icon name and codepoint pairs (one per line).
     * @return A shared pointer to an AKIconFont instance, or `nullptr` if loading fails.
     */
    static std::shared_ptr<AKIconFont> Make(const std::string &fontFamily, const char *codepoints = nullptr) noexcept;

    /**
     * @brief Renders an icon from the font by its name at a specified size.
     *
     * This method retrieves and renders an icon by its name (as defined in the codepoint map),
     * and returns the rendered image at the specified size.
     *
     * @param iconName The name of the icon (e.g., "account_circle") as defined in the codepoints map.
     * @param size The desired output image size (in pixels, square).
     * @return A shared pointer to an RImage containing the rendered icon, or `nullptr` if the icon cannot be found or rendered.
     */
    std::shared_ptr<RImage> getIconByName(const std::string &iconName, UInt32 size) noexcept;

    /**
     * @brief Renders an icon from a UTF-8 codepoint at a specified size.
     *
     * This method renders an icon based on the UTF-8 representation of a codepoint.
     *
     * @param utf8 The UTF-8 encoded string of the icon's codepoint.
     * @param size The desired output image size (in pixels, square).
     * @return A shared pointer to an RImage containing the rendered icon, or `nullptr` if the icon cannot be found or rendered.
     */
    std::shared_ptr<RImage> getIconByUTF8(const std::string &utf8, UInt32 size) noexcept;

    bool hasCodepointMap() const noexcept { return m_codepoints != std::nullopt; }

    /**
     * @brief Checks if an icon name exists in the font.
     *
     * This method checks whether a given icon name is available in the icon font.
     *
     * @param iconName The name of the icon to check (e.g., "account_circle").
     * @return `true` if the icon name exists in the font, `false` otherwise.
     */
    bool hasIconName(const std::string &iconName) const noexcept;

    /**
     * @brief Converts a Unicode codepoint to a UTF-8 encoded string.
     *
     * This method is useful for converting codepoints to UTF-8 strings, which can be used in various text rendering systems like SkParagraph.
     *
     * @param cp The Unicode codepoint (e.g., 0xE853).
     * @return A UTF-8 encoded string representing the given codepoint.
     */
    static std::string UTF8FromCodepoint(UInt32 cp) noexcept;

    /**
     * @brief Parses a multiline codepoint string into a map of icon names to UTF-8 codepoints.
     *
     * This method parses a multiline string in the format: `<icon_name> <hex_codepoint>` per line, and returns a map of icon names to their UTF-8 codepoints.
     *
     * @param input A multiline string containing icon names and their corresponding hex codepoints.
     * @return A map of icon names to UTF-8 codepoints, or `std::nullopt` if any line is invalid.
     */
    static std::optional<std::unordered_map<std::string, std::string>> ParseCodepoints(const char *input) noexcept;

private:
    // [utf8 code point][size] = image
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
