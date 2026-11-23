#ifndef CZ_AKAVATAR_H
#define CZ_AKAVATAR_H

#include <CZ/AK/Nodes/AKRoundSolidColor.h>
#include <CZ/AK/Nodes/AKImage.h>
#include <CZ/AK/Nodes/AKFontIcon.h>
#include <filesystem>
#include <memory>

/**
 * @brief A person's picture in a circle, or a generic person silhouette without one.
 * @ingroup AKNodes
 *
 * The picture is cropped to its centered square and masked to a circle. Without a picture (or if
 * it can't be loaded) a white "person" icon over gray is shown, like macOS's account pictures.
 *
 * @see UserPicturePath() to find the current user's picture.
 */
class CZ::AKAvatar : public AKRoundSolidColor
{
public:
    /** @brief Creates an avatar @p size units wide and tall, without a picture. */
    AKAvatar(Int32 size = 32, AKNode *parent = nullptr) noexcept;

    /** @brief Sets the diameter. */
    void setSize(Int32 size) noexcept;
    Int32 size() const noexcept { return m_size; }

    /** @brief Shows @p picture (nullptr: the silhouette). */
    void setPicture(std::shared_ptr<RImage> picture) noexcept;

    /**
     * @brief Loads the picture at @p path (raster formats or SVG).
     *
     * @return false (showing the silhouette) if it can't be loaded or @p path is empty.
     */
    bool setPictureFromFile(const std::filesystem::path &path) noexcept;

    /** @brief The picture shown, or nullptr. */
    std::shared_ptr<RImage> picture() const noexcept { return m_picture; }

    /**
     * @brief The picture of the user running the process, or an empty path if they have none.
     *
     * AccountsService's (`/var/lib/AccountsService/icons/` followed by the login name), else `~/.face`.
     */
    static std::filesystem::path UserPicturePath() noexcept;

    /** @brief The full name of the user running the process (GECOS), else their login name. */
    static std::string UserRealName() noexcept;

    /** @brief The login name of the user running the process. */
    static std::string UserLoginName() noexcept;

    AKImage image { this };     ///< The picture (hidden without one)
    AKFontIcon icon { this };   ///< The silhouette (hidden with a picture)

protected:
    /// Its circle mask at the size it's drawn at (see CircleMask() in AKAvatar.cpp).
    void onSceneBegin() override;

private:
    std::shared_ptr<RImage> m_picture;
    Int32 m_size { 0 };
    Int32 m_maskSize { 0 }; // Pixels
};

#endif // CZ_AKAVATAR_H
