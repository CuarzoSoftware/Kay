#ifndef AKFONTICON_H
#define AKFONTICON_H

#include <CZ/AK/Nodes/AKRenderable.h>

/**
 * @brief A node that displays a font icon.
 *
 * This node renders symbols from an AKIconFont using either an icon name or its corresponding UTF-8 code.
 * The icon size is specified in logical coordinates, and the image is automatically regenerated based on the current node's scale factor.
 *
 * If no specific AKIconFont is provided, the default font from AKTheme::fontIcon is used, which is Material Round Icons by default.
 *
 * You can find a list of Material icon names here: https://fonts.google.com/icons?icon.style=Rounded
 *
 * @note Tinting is enabled by default. You can adjust the icon color with setColor(), or disable tinting by calling enableReplaceImageColor().
 *
 * When no icon is set, the invisibleRegion covers the entire node, and no rendering occurs. However, the node's visibility (isVisible()) remains unaffected.
 */
class CZ::AKFontIcon : public AKRenderable
{
public:
    enum Changes
    {
        CHIcon = AKRenderable::CHLast, ///< Icon-related changes
        CHLast ///< Last change type
    };

    /**
     * @brief Constructs an AKFontIcon with a specific icon name, size, and optional custom AKIconFont.
     *
     * @param iconName The name of the icon from the AKIconFont.
     * @param size The size of the icon in logical coordinates.
     * @param iconFont A custom AKIconFont (optional).
     * @param parent The parent node (optional).
     */
    AKFontIcon(const std::string &iconName = "", UInt32 size = 32, std::shared_ptr<AKIconFont> iconFont = nullptr, AKNode *parent = nullptr) noexcept;

    /**
     * @brief Constructs an AKFontIcon with the default icon font.
     */
    AKFontIcon(const std::string &iconName = "", UInt32 size = 32, AKNode *parent = nullptr) noexcept;

    /**
     * @brief Constructs an AKFontIcon with default values (empty icon name, size 32, and default iconFont).
     *
     * @param parent The parent node (optional).
     */
    AKFontIcon(AKNode *parent = nullptr) noexcept;

    /**
     * @brief Sets the icon size in logical coordinates.
     *
     * The icon is regenerated later during onSceneBegin() if the size changes.
     *
     * @param size The new size of the icon in logical coordinates.
     */
    void setSize(UInt32 size) noexcept;

    /**
     * @brief Sets the icon using its name from the available icon font.
     *
     * The image is not updated immediately but will be rendered in onSceneBegin().
     *
     * @param iconName The name of the icon to be displayed.
     */
    void setIconFromName(const std::string &iconName) noexcept;

    /**
     * @brief Sets the icon using its UTF-8 codepoint representation.
     *
     * The image is not updated immediately but will be rendered in onSceneBegin().
     *
     * @param utf8 The UTF-8 representation of the icon's codepoint.
     */
    void setIconFromUTF8(const std::string &utf8) noexcept;

    /**
     * @brief Gets the current icon name or UTF-8 code being used.
     *
     * @see isUTF8()
     *
     * @return The current icon name or UTF-8 code as a string.
     */
    const std::string &icon() const noexcept { return m_icon; }

    /**
     * @brief Checks if the last icon was set using setIconFromUTF8() or setIconFromName()
     */
    bool isUTF8() const noexcept { return m_isUTF8; }

    /**
     * @brief Gets the current icon font used by this node.
     *
     * @return The shared pointer to the AKIconFont, or nullptr if no custom font is set.
     */
    std::shared_ptr<AKIconFont> iconFont() const noexcept { return m_iconFont; }

    /**
     * @brief Sets a custom AKIconFont for the node.
     *
     * If nullptr is passed, the default theme's icon font is restored.
     *
     * @param iconFont The custom AKIconFont to be used.
     */
    void setIconFont(std::shared_ptr<AKIconFont> iconFont) noexcept;

    /**
     * @brief Gets the image representation of the icon, if available.
     *
     * If no valid icon is found with the current name, this could return nullptr.
     *
     * @return A shared pointer to the image, or nullptr if no image is found.
     */
    std::shared_ptr<RImage> image() const noexcept { return m_image; }

protected:
    void onSceneBegin() override;
    void renderEvent(const AKRenderEvent &event) override;
    std::string m_icon;
    std::shared_ptr<RImage> m_image;
    std::shared_ptr<AKIconFont> m_iconFont;
    bool m_isUTF8 {};
};

#endif // AKFONTICON_H
