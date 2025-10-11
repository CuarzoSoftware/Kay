#include <CZ/AK/AKApp.h>
#include <CZ/AK/AKIconFont.h>
#include <CZ/AK/AKLog.h>

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
    iconFont->m_style.setFontFamilies({SkString(fontFamily)});
    iconFont->m_paragraphStyle.setTextDirection(skia::textlayout::TextDirection::kLtr);
    iconFont->m_builder = skia::textlayout::ParagraphBuilderImpl::make(iconFont->m_paragraphStyle, AKApp::Get()->fontCollection());

    if (!iconFont->m_builder)
    {
        AKLog(CZError, CZLN, "Failed to create ParagraphBuilder");
        return {};
    }

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
    if (size == 0)
        return {};

    auto found { m_cache[utf8][size] };

    if (auto lock = found.lock())
        return lock;

    auto surface { RSurface::Make(SkISize(size, size), 1, true) };

    if (!surface)
    {
        AKLog(CZError, CZLN, "Failed to create RImage");
        return {};
    }

    auto pass { surface->beginPass(RPassCap_SkCanvas) };
    auto *c { pass->getCanvas() };
    c->clear(SK_ColorTRANSPARENT);
    m_builder->Reset();
    SkPaint p;
    p.setBlendMode(SkBlendMode::kSrc);
    p.setAntiAlias(true);
    p.setColor(SK_ColorWHITE);
    m_style.setForegroundColor(p);
    m_style.setHeight(size);
    m_style.setFontSize(size);
    m_builder->pushStyle(m_style);
    m_builder->addText(utf8.c_str());
    m_paragraph = m_builder->Build();

    if (!m_paragraph)
    {
        AKLog(CZError, CZLN, "Failed to create paragraph");
        return {};
    }

    m_paragraph->layout(size);

    if (m_paragraph->getMaxIntrinsicWidth() <= 0 || m_paragraph->getHeight() <= 0)
        return {};

    float scaleX = size / m_paragraph->getMaxIntrinsicWidth();
    float scaleY = size / m_paragraph->getHeight();

    c->save();
    c->scale(scaleX, scaleY);
    m_paragraph->paint(c, 0, 0);
    c->restore();
    pass.reset();

    m_cache[utf8][size] = surface->image();
    return surface->image();
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
