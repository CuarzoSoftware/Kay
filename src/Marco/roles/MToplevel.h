#ifndef MTOPLEVEL_H
#define MTOPLEVEL_H

#include <Marco/roles/MSurface.h>
#include <AK/AKWindowState.h>

/**
 * @brief Represents a toplevel window.
 *
 * The `MToplevel` class implements the `xdg_toplevel` role within the `xdg_shell` protocol.
 * It is the most common type of window, capable of being maximized, set to fullscreen, minimized, and more.
 *
 * ## Decorations
 *
 * By default, toplevels use ClientSide decorations with builtinDecorationsEnabled().
 * These built-in decorations add rounded corners and shadows. To disable them, use enableBuiltinDecorations().
 *
 * When built-in decorations are disabled, you can define custom decorations using setDecorationMargins().
 * This informs both the internal scene and compositor about which parts of the window serve as decorations.
 * The final surface size will be the specified size of the central node plus the custom decoration margins.
 * Custom decorations can be created by adding nodes to the root() node, either behind or on top of the central node.
 *
 * Alternatively, you can request ServerSide decorations with setDecorationMode().
 * This is an asynchronous operation and may not be supported by the compositor.
 * To handle decoration mode changes, override decorationModeChanged() or subscribe to the onDecorationModeChanged signal.
 *
 * When using ServerSide decorations:
 * - Custom decoration margins are ignored.
 * - Built-in decorations are hidden.
 * - You should hide your custom decoration nodes.
 */
class AK::MToplevel : public MSurface
{
public:

    /**
     * @enum WMCapabilities
     * @brief Enumerates the capabilities supported by the compositor.
     *
     * These capabilities specify the actions that the compositor can handle.
     * Use the following methods to query and handle capability changes:
     * @see wmCapabilities()
     * @see wmCapabilitiesChanged()
     * @see onWMCapabilitiesChanged()
     */
    enum WMCapabilities
    {
        /**
         * @brief Supports displaying the window menu.
         * @see showWindowMenu()
         */
        WindowMenuCap = 1 << 1,

        /**
         * @brief Supports maximizing windows.
         *
         * If unsupported, calling setMaximized() has no effect.
         */
        MaximizeCap = 1 << 2,

        /**
         * @brief Supports fullscreen windows.
         *
         * If unsupported, calling setFullscreen() has no effect.
         */
        FullscreenCap = 1 << 3,

        /**
         * @brief Supports minimizing windows.
         *
         * If unsupported, calling setMinimized() has no effect.
         */
        MinimizeCap = 1 << 4,
    };


    /**
     * @brief Enumerates the available modes for window decorations.
     *
     * @see setDecorationMode()
     * @see decorationModeChanged()
     * @see onDecorationModeChanged()
     * @see enableBuiltinDecorations()
     */
    enum DecorationMode
    {
        /**
         * @brief Indicates that the window uses Client-Side Decorations.
         *
         * In this mode, the client application is responsible for drawing
         * and managing the window's decorations.
         */
        ClientSide = 1,

        /**
         * @brief Indicates that the window uses Server-Side Decorations.
         *
         * In this mode, the server (compositor) is responsible for managing
         * and rendering the window's decorations.
         */
        ServerSide = 2
    };

    /**
     * @brief Default constructor for `MToplevel`.
     */
    MToplevel() noexcept;

    AKCLASS_NO_COPY(MToplevel)

    /**
     * @brief Destructor for `MToplevel`.
     */
    ~MToplevel() noexcept;

    /**
     * @brief Retrieves the current window states.
     *
     * Returns a bitset containing the current window states, such as Maximized, Fullscreen, Activated, etc.
     * Individual states can also be checked using helper methods like `maximized()`, `fullscreen()`, etc.
     *
     * @return A bitset representing the current window states.
     * @see windowStateEvent(), onStatesChanged
     */
    AKBitset<AKWindowState> states() const noexcept;

    /**
     * @brief Requests the compositor to maximize or unmaximize the window.
     *
     * The state change is not applied immediately. The compositor will notify the window
     * of the new state via `windowStateEvent()` and `onStatesChanged`.
     *
     * @param maximized If `true`, the window will be maximized, if `false`, it will be unmaximized.
     * @see suggestedSizeChanged(), windowStateEvent(), onStatesChanged
     */
    void setMaximized(bool maximized) noexcept;

    /**
     * @brief Checks if the window is currently maximized.
     *
     * This is equivalent to calling `states().check(AKMaximized)`.
     *
     * @return `true` if the window is maximized, `false` otherwise.
     */
    bool maximized() const noexcept;

    /**
     * @brief Requests the compositor to set the window to fullscreen or exit fullscreen mode.
     *
     * The state change is not applied immediately. The compositor will notify the window
     * of the new state via `windowStateEvent()` and `onStatesChanged`.
     *
     * @param fullscreen If `true`, the window will be set to fullscreen, if `false`, it will exit fullscreen mode.
     * @param screen The screen on which to display the window in fullscreen mode. If `nullptr`, the compositor will choose the screen.
     * @see suggestedSizeChanged(), windowStateEvent(), onStatesChanged
     */
    void setFullscreen(bool fullscreen, MScreen *screen = nullptr) noexcept;

    /**
     * @brief Checks if the window is currently in fullscreen mode.
     *
     * This is equivalent to calling `states().check(AKFullscreen)`.
     *
     * @return `true` if the window is in fullscreen mode, `false` otherwise.
     */
    bool fullscreen() const noexcept;

    /**
     * @brief Requests the compositor to minimize the window.
     *
     * Note that minimizing is not a window state, and there is no way to query
     * whether the compositor has minimized the window.
     */
    void setMinimized() noexcept;

    /**
     * @brief Sets the minimum size of the window and notifies the compositor.
     *
     * This function updates the minimum dimensions of the window (central node) and communicates
     * the changes to the compositor. Internally, it calls `layout().setMin(Width/Height)`.
     * You should avoid using these layout functions directly.
     *
     * @note Passing 0 for either width or height disables the constraint for that axis.
     *
     * @param size The minimum dimensions (width and height) of the window.
     */
    void setMinSize(const SkISize &size) noexcept;

    /**
     * @brief Retrieves the minimum size of the window.
     *
     * This function returns the current minimum dimensions of the window (central node), as
     * previously set with `setMinSize()`.
     *
     * @return The minimum dimensions (width and height) of the window.
     */
    const SkISize &minSize() const noexcept;

    /**
     * @brief Sets the maximum size of the window and notifies the compositor.
     *
     * This function updates the maximum dimensions of the window (central node) and communicates
     * the changes to the compositor. Internally, it calls `layout().setMax(Width/Height)`.
     * Users should avoid using these layout functions directly.
     *
     * @note Passing 0 for either width or height disables the constraint for that axis.
     *
     * @param size The maximum dimensions (width and height) of the window.
     */
    void setMaxSize(const SkISize &size) noexcept;

    /**
     * @brief Retrieves the maximum size of the window.
     *
     * This function returns the current maximum dimensions of the window (central node), as
     * previously set with `setMaxSize()`.
     *
     * @return The maximum dimensions (width and height) of the window.
     */
    const SkISize &maxSize() const noexcept;


    /**
     * @brief Retrieves the last window size suggested by the compositor.
     *
     * If either the width or height is `0`, it means the compositor is letting the application
     * to decide the size for that axis.
     *
     * @return The suggested window (central node) size.
     */
    const SkISize &suggestedSize() const noexcept;

    /**
     * @brief Retrieves the last bounds suggested by the compositor.
     *
     * If either the width or height is `0`, it means the compositor is letting the application
     * to decide the size for that axis.
     *
     * The bounds can for example correspond to the size of a monitor excluding any panels or other shell components,
     * so that a surface isn't created in a way that it cannot fit.
     *
     * @return The suggested window (central node) bounds.
     */
    const SkISize &suggestedBounds() const noexcept;

    /**
     * @brief Sets the title of the window.
     *
     * @param title The new title for the window.
     * @see onTitleChanged
     */
    void setTitle(const std::string &title);

    /**
     * @brief Retrieves the current window title.
     *
     * The title is set using `setTitle()` and is empty by default.
     *
     * @return The current window title.
     */
    const std::string &title() const noexcept;

    /**
     * @brief Retrieves the compositor's supported window management capabilities.
     *
     * This function returns a bitset representing the compositor's capabilities,
     * such as support for maximizing, minimizing, fullscreen, and more.
     *
     * @see wmCapabilitiesChanged()
     * @see onWMCapabilitiesChanged
     */
    AKBitset<WMCapabilities> wmCapabilities() const noexcept;

    /**
     * @brief Requests the compositor to display the window menu at a specific position.
     *
     * This function triggers the compositor to show the window menu, typically used
     * for window management actions, at the specified position.
     *
     * @param event The input event that caused the request, such as a right-click.
     * @param pos The position within the central node where the window menu should be displayed.
     *
     * @return `true` if the request is successful, `false` otherwise.
     */
    bool showWindowMenu(const AKInputEvent &event, const SkIPoint &pos) noexcept;

    /**
     * @brief Retrieves the current decoration mode of the window.
     *
     * This function returns the decoration mode, which specifies whether the
     * window is using client-side or server-side decorations.
     *
     * @return The current decoration mode of the type `DecorationMode`.
     * @see setDecorationMode()
     */
    DecorationMode decorationMode() const noexcept;

    /**
     * @brief Sets the decoration mode for the window.
     *
     * This function allows you to set whether the window should use client-side
     * or server-side decorations. The state change might not be applied immediately,
     * as it depends on the compositor's capabilities.
     *
     * @param mode The decoration mode to set, of type `DecorationMode`.
     * @see decorationMode(), onDecorationModeChanged
     */
    void setDecorationMode(DecorationMode mode) noexcept;

    /**
     * @brief Checks if built-in client-side decorations are enabled.
     *
     * This function returns whether the built-in client-side decorations,
     * such as rounded corners and shadows, are currently enabled for the window.
     *
     * @note Enabled by default.
     *
     * @return `true` if built-in decorations are enabled, `false` otherwise.
     * @see enableBuiltinDecorations()
     */
    bool builtinDecorationsEnabled() const noexcept;

    /**
     * @brief Enables or disables built-in client-side decorations.
     *
     * This function allows you to toggle the use of the built-in client-side
     * decorations, such as rounded corners and shadows, for the window.
     *
     * @param enabled If `true`, built-in decorations are enabled. If `false`,
     *        built-in decorations are disabled.
     * @see builtinDecorationsEnabled()
     */
    void enableBuiltinDecorations(bool enabled) noexcept;

    /**
     * @brief Retrieves the built-in client decoration margins.
     *
     * This is primarily informational and has no direct functional use.
     *
     * Used when the decorationMode() is ClientSide and built-in decorations
     * are enabled.
     */
    const SkIRect &builtinDecorationMargins() const noexcept;

    /**
     * @brief Retrieves the custom client decoration margins.
     *
     * Used when the decorationMode() is ClientSide and built-in decorations
     * are disabled.
     *
     * @see setDecorationMargins()
     */
    const SkIRect &decorationMargins() const noexcept;

    /**
     * @brief Configures custom decoration margins for the window.
     *
     * This function sets custom margins for the window decorations. It is applicable
     * when decorationMode() is set to ClientSide and built-in decorations are disabled.
     *
     * @param margins Positive values for the margins of each edge (L, T, R, B).
     *                If a negative value is provided for any edge, it will be overridden with 0.
     */
    void setDecorationMargins(const SkIRect &margins) noexcept;

    /**
     * @brief Sets the parent toplevel window for the current window.
     *
     * Child toplevels are tipically used to display modals, the compositor will always
     * place child toplevels on top of the parent.
     *
     * @note Toplevels can only be children of other toplevels.
     *
     * @param parent A pointer to the parent `MToplevel` window. Passing `nullptr` removes the parent-child relationship.
     * @return `true` if the parent toplevel was successfully set, `false` otherwise.
     * @see parentToplevel(), childToplevels()
     */
    bool setParentToplevel(MToplevel *parent) noexcept;

    /**
     * @brief Retrieves the parent toplevel window of the current window.
     *
     * @return A pointer to the parent `MToplevel` window, or `nullptr` if none is set.
     * @see setParentToplevel(), childToplevels()
     */
    MToplevel *parentToplevel() const noexcept;

    /**
     * @brief Retrieves the set of child toplevel windows associated with the current window.
     *
     * This function returns a collection of all toplevel windows that are children of
     * the current window.
     *
     * @return An unordered set of pointers to the child `MToplevel` windows.
     * @see setParentToplevel(), parentToplevel()
     */
    const std::unordered_set<MToplevel*> &childToplevels() const noexcept;

    const std::unordered_set<MPopup*> &childPopups() const noexcept;

    /**
     * @brief Signal emitted when the window states change.
     *
     * This signal is triggered by the default implementation of `windowStateEvent()`.
     */
    AKSignal<const AKWindowStateEvent&> onStatesChanged;

    /**
     * @brief Signal emitted when the window title changes.
     *
     * This signal is triggered after calling `setTitle()` with a new title.
     */
    AKSignal<> onTitleChanged;

    /**
     * @brief Signal triggered when the compositor requests to close the window.
     *
     * To ignore the request, call `event->ignore()`.
     *
     * If the request is not ignored, the toplevel window will be unmapped (see setMapped()) but not
     * destroyed.
     */
    AKSignal<const AKWindowCloseEvent&> onBeforeClose;

    /**
     * @brief Signal emitted when the suggested window size changes.
     *
     * This signal is triggered when the compositor suggests a new size for the window.
     * It can occur before the window is mapped or when the compositor updates its size suggestion.
     *
     * @see suggestedSize(), suggestedSizeChanged()
     */
    AKSignal<> onSuggestedSizeChanged;

    /**
     * @brief Signal emitted when the suggested window bounds change.
     *
     * This signal is triggered when the compositor suggests new bounds for the window.
     * Suggested bounds typically exclude areas occupied by shell components like panels
     * to ensure the window fits within the usable space.
     *
     * @see suggestedBounds(), suggestedBoundsChanged()
     */
    AKSignal<> onSuggestedBoundsChanged;

    /**
     * @brief Signal emitted when the compositor's window management capabilities change.
     *
     * This signal notifies the application of updates to the compositor's supported
     * capabilities, such as maximizing, minimizing, or other window states.
     *
     * @see wmCapabilities(), wmCapabilitiesChanged()
     */
    AKSignal<> onWMCapabilitiesChanged;

    /**
     * @brief Signal emitted when the window's decoration mode changes.
     *
     * This signal is triggered when the decoration mode switches between client-side
     * and server-side decorations, allowing applications to react accordingly.
     *
     * @see setDecorationMode(), decorationMode(), decorationModeChanged()
     */
    AKSignal<> onDecorationModeChanged;

    /**
     * @brief Signal emitted when the decorationMargins() change.
     *
     * @see setDecorationMargins() and decorationMarginsChanged()
     */
    AKSignal<> onDecorationMarginsChanged;

    class Imp;
    Imp *imp() const noexcept;

protected:

    /**
     * @brief Handles changes in the compositor's window management capabilities.
     *
     * This virtual method is called when the compositor notifies the application
     * of changes in its supported window management capabilities.
     * By default, it triggers the `onWMCapabilitiesChanged` signal.
     */
    virtual void wmCapabilitiesChanged();

    /**
     * @brief Handles changes in the window's decoration mode.
     *
     * This virtual method is called when the window's decoration mode changes,
     * such as switching between client-side and server-side decorations.
     * By default, it triggers the `onDecorationModeChanged` signal.
     */
    virtual void decorationModeChanged();

    /**
     * @brief Handles changes of decorationMargins()
     */
    virtual void decorationMarginsChanged();

    /**
     * @brief Notifies the window that the compositor has suggested a new size.
     *
     * This method is triggered before the window is mapped and whenever the compositor
     * suggests a new size, such as after a maximize or fullscreen request.
     *
     * While it is generally advisable to follow the compositor's suggestions, they can be ignored.
     *
     * @see suggestedSize()
     */
    virtual void suggestedSizeChanged();

    /**
     * @brief Notifies the window that the compositor has suggested new bounds.
     *
     * This method could be triggered before the window is mapped and whenever the compositor
     * suggests new bounds.
     *
     * @see suggestedBounds()
     */
    virtual void suggestedBoundsChanged();

    /**
     * @brief Handles changes in the window states.
     *
     * This method is called when the window states change. By default, it triggers the
     * `onStatesChanged` signal.
     *
     * @param event The event containing the state changes.
     */
    void windowStateEvent(const AKWindowStateEvent &event) override;
    bool eventFilter(const AKEvent &event, AKObject &object) override;
    bool event(const AKEvent &e) override;
    void onUpdate() noexcept override;

private:
    std::unique_ptr<Imp> m_imp;
    void render() noexcept;
};

#endif // MTOPLEVEL_H
