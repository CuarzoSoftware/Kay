#ifndef CZ_AKCLIPBOARD_H
#define CZ_AKCLIPBOARD_H

#include <CZ/AK/AK.h>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace CZ { class CZInputEvent; }

/**
 * @brief The system clipboard (text, or data in other formats, e.g. files), as provided by the
 *        windowing layer.
 * @ingroup AKCore
 *
 * Kay doesn't talk to the windowing system itself: the toolkit built on it (e.g. Marco, with
 * Wayland's data device) sets an implementation with AKApp::setClipboard(). Without one,
 * AKApp::clipboard() is `nullptr` and nodes don't offer copy & paste.
 */
class CZ::AKClipboard
{
public:
    virtual ~AKClipboard() = default;

    /**
     * @brief Puts @p text on the clipboard.
     *
     * @param triggeringEvent The input event that triggered it (e.g. the Ctrl+C key press):
     *                        compositors only let clients that just got input own the clipboard.
     */
    virtual void setText(const std::string &text, const CZInputEvent &triggeringEvent) noexcept = 0;

    /**
     * @brief Reads the clipboard's text, asynchronously (it may come from another process).
     *
     * @p done is invoked later from the event loop (never from within this call) with the text,
     * or `std::nullopt` if the clipboard is empty or holds no text.
     */
    virtual void requestText(std::function<void(std::optional<std::string> text)> done) noexcept = 0;

    /// Data in one format: its MIME type and its bytes.
    using Format = std::pair<std::string, std::string>;

    /**
     * @brief Puts data on the clipboard, in several formats (e.g. files as "text/uri-list" and
     *        "x-special/gnome-copied-files", and their paths as "text/plain;charset=utf-8").
     *
     * @param triggeringEvent See setText(). Unsupported by default (nothing happens).
     */
    virtual void setData(const std::vector<Format> &formats, const CZInputEvent &triggeringEvent) noexcept
    {
        (void)formats; (void)triggeringEvent;
    }

    /**
     * @brief Reads the clipboard's data in the first of @p mimeTypes it has, asynchronously.
     *
     * @p done is invoked later from the event loop (never from within this call) with the format
     * read, or `std::nullopt` if the clipboard has none of them. Unsupported by default (nullopt).
     */
    virtual void requestData(const std::vector<std::string> &mimeTypes, std::function<void(std::optional<Format> data)> done) noexcept
    {
        (void)mimeTypes;
        done(std::nullopt);
    }

    /// The formats (MIME types) the clipboard has now (e.g. to enable Paste). None by default.
    virtual std::vector<std::string> mimeTypes() const noexcept { return {}; }
};

#endif // CZ_AKCLIPBOARD_H
