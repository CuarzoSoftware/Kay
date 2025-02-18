#ifndef AKPATH_H
#define AKPATH_H

#include <AK/nodes/AKBakeable.h>
#include <AK/AKBrush.h>
#include <AK/AKPen.h>
#include <include/core/SkPath.h>

/**
 * @brief Baked path node.
 * @ingroup AKNodes
 *
 * This node can be used to display paths, such as vector graphics from SVG files.
 *
 * The customTextureColorEnabled() property is enabled by default, allowing you to change the color of the path
 * using setColor() without requiring re-baking the component. However, this only allows filling the
 * path with a solid color. If customized fills and strokes are needed, enableCustomTextureColor() should
 * be set to false, and a custom AKBrush and AKPen can be provided.
 *
 * The default brush is black, and the default pen is disabled.
 *
 * By default, the node is scaled to fit the path's bounds, but the path can also be automatically scaled
 * to fit the current node's dimensions. See setSizeMode() for details.
 */
class AK::AKPath : public AKBakeable
{
public:
    /**
     * @enum Changes
     */
    enum Changes
    {
        /// path() changed.
        CHPath = AKBakeable::CHLast,

        /// sizeMode() changed.
        CHSizeMode,

        CHLast
    };

    /**
     * @brief SizeMode
     */
    enum SizeMode
    {
        /// The layout's width and height are scaled to match the path's bounds (default mode).
        ScaleLayout,

        /// The path is scaled to fit the node's layout.
        ScalePath
    };

    /**
     * @brief Initializes with an empty path.
     */
    AKPath(AKNode *parent = nullptr) noexcept :
        AKBakeable(parent)
    {
        m_brush.setAntiAlias(true);
        m_pen.setAntiAlias(true);
        enableCustomTextureColor(true);
    };

    /**
     * @brief Initializes with the given path.
     */
    AKPath(const SkPath &path, AKNode *parent = nullptr) noexcept :
        AKBakeable(parent),
        m_path(path)
    {
        m_brush.setAntiAlias(true);
        m_pen.setAntiAlias(true);
        enableCustomTextureColor(true);
    };

    /**
     * @brief Sets the path.
     */
    void setPath(const SkPath &path) noexcept
    {
        if (m_path == path)
            return;

        m_path = path;
        addChange(CHPath);
    }

    /**
     * @brief Gets the current path.
     */
    const SkPath &path() const noexcept
    {
        return m_path;
    }

    /**
     * @brief Sets the size mode.
     */
    void setSizeMode(SizeMode mode) noexcept
    {
        if (m_sizeMode == mode)
            return;

        m_sizeMode = mode;
        addChange(CHSizeMode);
    }

    /**
     * @brief Gets the current size mode.
     */
    SizeMode sizeMode() const noexcept
    {
        return m_sizeMode;
    }

protected:
    void onSceneBegin() override;
    void onBake(const BakeEvent &event) override;
    SkPath m_path;
    AKBrush m_brush;
    SkRect m_bounds;
    SkMatrix m_matrix;
    AKPen m_pen { AKPen::NoPen() };
    SizeMode m_sizeMode { ScaleLayout };
};

#endif // AKPATH_H
