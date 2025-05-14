#include <Marco/input/MPointer.h>
#include <Marco/MApplication.h>
#include <Marco/roles/MSurface.h>

using namespace AK;

void MPointer::setCursor(AKCursor cursor) noexcept
{
    if (!m_forceCursorUpdate && m_cursor == cursor && m_cursorSurface)
        return;

    m_forceCursorUpdate = false;

    m_cursor = cursor;

    if (cursor == AKCursor::Hidden)
    {
        wl_pointer_set_cursor(app()->wayland().pointer, eventHistory().enter.serial(), NULL, 0, 0);
        return;
    }

    if (!m_cursorSurface)
        m_cursorSurface = wl_compositor_create_surface(app()->wayland().compositor);

    if (!m_cursorTheme)
        m_cursorTheme = wl_cursor_theme_load(NULL, 48, app()->wayland().shm);

    if (!m_cursorTheme)
        return;

    wl_cursor_image *image { nullptr };

    auto it = m_cursors.find(cursor);

    if (it != m_cursors.end())
    {
        image = it->second->images[0];
    }
    else
    {
        wl_cursor *wlCursor { wl_cursor_theme_get_cursor(m_cursorTheme, cursorToString(cursor)) };

        if (wlCursor && wlCursor->image_count > 0)
        {
            m_cursors[cursor] = wlCursor;
            image = wlCursor->images[0];
        }
    }

    if (!image)
    {
        assert("Marco requires a default cursor theme. See https://wiki.archlinux.org/title/Cursor_themes#The_default_cursor_theme" && cursor != AKCursor::Default);
        setCursor(AKCursor::Default);
        return;
    }

    const Int32 scale { (image->width >= 48 || image->height >= 48) ? 2 : 1 };
    wl_surface_attach(m_cursorSurface, wl_cursor_image_get_buffer(image), 0, 0);
    wl_surface_set_buffer_scale(m_cursorSurface, scale);
    wl_surface_damage(m_cursorSurface, 0, 0, 128, 128);
    wl_surface_commit(m_cursorSurface);
    wl_pointer_set_cursor(app()->wayland().pointer, eventHistory().enter.serial(), m_cursorSurface, image->hotspot_x/scale, image->hotspot_y/scale);
}

AKCursor MPointer::findNonDefaultCursor(AKNode *node) const noexcept
{
    while (node && node->cursor() == AKCursor::Default)
        node = node->parent();

    if (node)
        return node->cursor();

    return AKCursor::Default;
}
