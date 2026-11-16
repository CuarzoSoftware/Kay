#include <CZ/AK/AKApp.h>
#include <CZ/AK/AKIconFont.h>
#include <CZ/AK/AKLog.h>
#include <CZ/AssetBroker/ABCore.h>
#include <CZ/AssetBroker/ABGenerators.h>

#include <CZ/Core/CZBitset.h>

#include <CZ/Ream/RImage.h>
#include <CZ/Ream/RCore.h>
#include <CZ/Ream/RDevice.h>
#include <CZ/Ream/RPass.h>
#include <CZ/Ream/RSurface.h>

#include <fstream>
#include <sstream>
#include <string>

using namespace CZ;

std::shared_ptr<AKIconFont> AKIconFont::Make(const std::string &fontFamily, const std::filesystem::path &codepoints) noexcept
{
    std::ifstream fileStream { codepoints };

    if (!fileStream)
    {
        AKLog(CZError, CZLN, "Unable to open codepoints file: {}", codepoints.c_str());
        return AKIconFont::Make(fontFamily);
    }

    std::ostringstream buffer;
    buffer << fileStream.rdbuf();

    return AKIconFont::Make(fontFamily, buffer.str().c_str());
}

std::shared_ptr<CZ::AKIconFont> AKIconFont::Make(const std::string &fontFamily, const char *codepoints) noexcept
{
    auto iconFont { std::shared_ptr<AKIconFont>(new AKIconFont()) };
    iconFont->m_fontFamily = fontFamily;

    if (codepoints)
    {
        iconFont->m_codepoints = ParseCodepoints(codepoints);

        if (!iconFont->m_codepoints)
        {
            AKLog(CZError, CZLN, "Failed to parse codepoints");
            return iconFont;
        }
    }

    return iconFont;
}

std::shared_ptr<RImage> AKIconFont::getIconByName(const std::string &iconName, UInt32 size) noexcept
{
    if (!hasCodepointMap())
        return {};

    auto utf8 { m_codepoints->find(iconName) };

    if (utf8 == m_codepoints->end())
    {
        AKLog(CZError, CZLN, "No codepoint found for the icon: {}", iconName);
        return {};
    }

    return getIconByUTF8(utf8->second, size);
}

std::shared_ptr<RImage> AKIconFont::getIconByUTF8(const std::string &utf8, UInt32 size) noexcept
{
    if (size == 0 || utf8.empty())
        return {};

    auto found { m_cache[utf8][size] };

    if (auto lock = found.lock())
        return lock;

    // Drawn (and shared with other apps) by the AssetBroker: the glyph stretched to a size x size square
    if (!m_assetBroker)
        m_assetBroker = ABCore::GetOrMake();

    if (!m_assetBroker)
        return {};

    auto image { m_assetBroker->generate({ ABGenerators::Glyph, { Int64(size) }, { m_fontFamily, utf8 } }) };

    if (!image)
    {
        AKLog(CZError, CZLN, "Failed to draw the glyph of {} at {}px", m_fontFamily, size);
        return {};
    }

    m_cache[utf8][size] = image;
    return image;
}

bool AKIconFont::hasIconName(const std::string &iconName) const noexcept
{
    if (m_codepoints.has_value())
        return m_codepoints->contains(iconName);

    return false;
}

std::string AKIconFont::UTF8FromCodepoint(UInt32 cp) noexcept
{
    std::string utf8;

    if (cp <= 0x7F)
        utf8 += static_cast<char>(cp);
    else if (cp <= 0x7FF)
    {
        utf8 += static_cast<char>(0xC0 | ((cp >> 6) & 0x1F));
        utf8 += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp <= 0xFFFF)
    {
        utf8 += static_cast<char>(0xE0 | ((cp >> 12) & 0x0F));
        utf8 += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        utf8 += static_cast<char>(0x80 | (cp & 0x3F));
    } else
    {
        utf8 += static_cast<char>(0xF0 | ((cp >> 18) & 0x07));
        utf8 += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        utf8 += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        utf8 += static_cast<char>(0x80 | (cp & 0x3F));
    }

    return utf8;
}

std::optional<std::unordered_map<std::string, std::string>> AKIconFont::ParseCodepoints(const char *input) noexcept
{
    assert(input);
    std::unordered_map<std::string, std::string> map;
    std::istringstream stream { input };
    std::string line;

    while (std::getline(stream, line))
    {
        std::istringstream linestream { line };
        std::string name, hex;

        if (!(linestream >> name >> hex))
        {
            AKLog(CZError, CZLN, "Malformed line (missing name or hex code)");
            return std::nullopt;
        }

        try
        {
            UInt32 codepoint = std::stoul(hex, nullptr, 16);
            map[name] = UTF8FromCodepoint(codepoint);
        } catch (const std::exception&)
        {
            AKLog(CZError, CZLN, "Invalid hex string or number overflow");
            return std::nullopt;
        }
    }

    return map;
}
