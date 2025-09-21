#ifndef CZ_AKPATH_H
#define CZ_AKPATH_H

#include <CZ/AK/Nodes/AKBakeable.h>
#include <CZ/skia/core/SkPaint.h>
#include <CZ/skia/core/SkPath.h>

/**
 * @brief Baked path node.
 * @ingroup AKNodes
 *
 * This node can be used to display paths, such as vector graphics from SVG files.
 *
 * The customTextureColorEnabled() property is enabled by default, allowing you to change the color of the path
 * using setColor() without requiring re-baking the component. However, this only allows filling the
 * path with a solid color. If customized fills and strokes are needed, enableCustomTextureColor() should
 * be set to false, and a custom SkPaint and SkPaint can be provided.
 *
 * The default brush is black, and the default pen is disabled.
 *
 * By default, the node is scaled to fit the path's bounds, but the path can also be automatically scaled
 * to fit the current node's dimensions. See setSizeMode() for details.
 */
class CZ::AKPath : public AKBakeable
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
        enableReplaceImageColor(true);
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
        enableReplaceImageColor(true);
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
    void bakeEvent(const AKBakeEvent &event) override;
    SkPath m_path;
    SkPaint m_brush;
    SkRect m_bounds;
    SkMatrix m_matrix;
    SkPaint m_pen {/* TODO: No pen */};
    SizeMode m_sizeMode { ScaleLayout };
};

#endif // CZ_AKPATH_H
