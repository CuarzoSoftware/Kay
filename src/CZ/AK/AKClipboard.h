#ifndef CZ_AKCLIPBOARD_H
#define CZ_AKCLIPBOARD_H

#include <CZ/AK/AK.h>
#include <functional>
#include <optional>
#include <string>

namespace CZ { class CZInputEvent; }

/**
 * @brief The system clipboard (text), as provided by the windowing layer.
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
};

#endif // CZ_AKCLIPBOARD_H
