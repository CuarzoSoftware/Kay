#ifndef CZ_AK_H
#define CZ_AK_H

#include <CZ/Core/Cuarzo.h>
#include <CZ/skia/core/SkFontMgr.h>
#include <filesystem>

#define AK_MAX_BUFFER_AGE 3

/**
 * @defgroup AKCore Core
 * @brief Core classes
 */
namespace CZ
{
    class AKApp;
    class AKObject;

    class AKTheme;
    class AKChanges;
    class AKBackgroundDamageTracker;

    class AKLog;

    class AKScene;  /* Renders a root AKNode into an AKTarget */
    class AKTarget;
    class AKTarget; /* An AKScene render destination */
    class AKLayout; /* Yoga layout of an AKNode */

    /*********** CORE NODE TYPES ***********/

    class AKNode;       /* Base class for nodes */
    class AKContainer;  /* Container for other nodes. Doesn't produce any output on its own */
    class AKRenderable; /* A node that draws directly into an AKTarget */
    class AKBakeable;   /* A node that draws into its own buffer, which is then blitted into an AKTarget */
    class AKSubScene;   /* A node whose children are drawn into its own buffer and then rendered into an AKTarget */

    class AKSolidColor;
    class AKPath;
    class AKImage;
    class AKImageFrame;
    class AKThreeImagePatch;
    class AKTextCaret;
    class AKText;
    class AKCoreTextEditor;
    class AKRoundContainer;
    class AKButton;
    class AKTextField;
    class AKWindowButton;
    class AKWindowButtonGroup;
    class AKScroll;
    class AKScrollBar;

    /************ EFFECTS ************/

    class AKRoundCornersEffect;
    class AKBackgroundEffect;
    class AKBackgroundBoxShadowEffect;
    class AKBackgroundImageShadowEffect;
    class AKBackgroundBlurEffect;
    class AKEdgeShadow;

    /************ INPUT *************/

    class AKKeyboard;
    class AKPointer;

    /************ EVENTS *************/

    class AKRenderEvent;
    class AKBakeEvent;
    class AKSceneChangedEvent;
    class AKVibrancyEvent;

    AKTheme *theme() noexcept;
    void setTheme(AKTheme *theme) noexcept;
    sk_sp<SkFontMgr> akFontManager() noexcept;
    AKPointer &akPointer() noexcept;
    AKKeyboard &akKeyboard() noexcept;
    const std::filesystem::path &akAssetsDir() noexcept;
};

#define AK_IRECT_INF SkIRect::MakeLTRB(-2147483, -2147483, 2147483, 2147483)

#endif // CZ_AK_H
