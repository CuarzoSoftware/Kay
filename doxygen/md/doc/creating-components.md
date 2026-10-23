# Creating Components {#creating_components_page}

This guide explains how to build your own components: which base class to derive from, how to track property changes, how to render, and how to react to input and theme changes. Read @ref core_components_page "Core Components" first if you haven't.

## Choosing a base class

Ask yourself what your component needs to do:

| If your component... | Derive from | Examples |
|---|---|---|
| Is composed entirely of existing nodes (it only arranges/wires them) | AKContainer (or an existing node such as AKRoundSolidColor) | AKButton, AKTextField, AKWindowButtonGroup |
| Fills its rect with GPU-cheap content: an image/texture, a solid color, a mask | AKRenderable | AKImage, AKSolidColor |
| Draws complex content with Skia (text, paths, gradients, custom vector shapes) | AKBakeable | AKText, AKPath, AKFontIcon |
| Applies effects/post-processing to a whole subtree, or should cache the composition of its children | AKSubScene | — |
| Renders behind another node, outside the layout (shadows, blur) | AKBackgroundEffect | AKBackgroundBoxShadowEffect, AKBackgroundBlurEffect |

Rules of thumb:

- Prefer **composition** (AKContainer + built-in nodes) whenever possible: you get damage tracking, theming and input handling for free.
- Prefer **AKBakeable** over AKRenderable when drawing with Skia. Skia *can* be used inside a renderEvent(), but it's much slower than the built-in Ream shaders, and without a cached surface you'd re-draw on every repaint. Baking draws once and lets the scene composite the cached surface afterwards.
- Use **AKRenderable** directly only when the content maps to a Ream painter call (RPainter::drawImage(), RPainter::drawColor()), e.g. to display a Wayland surface's texture in a compositor.

## The Changes enum pattern

Nodes don't repaint immediately when a property changes. Instead, each setter records a change bit that is consumed during the next AKScene::render(). Every subclass extends its parent's `Changes` enum, starting at the parent's `CHLast`:

```cpp
class MySlider : public AKContainer
{
public:
    enum Changes
    {
        CHValue = AKContainer::CHLast, // AKContainer doesn't add changes, so this equals AKNode::CHLast
        CHRange,
        CHLast // Always keep a CHLast at the end so subclasses can chain
    };

    void setValue(Float32 value) noexcept
    {
        if (m_value == value)
            return;

        m_value = value;
        addChange(CHValue); // Also marks intersected targets as dirty
    }
    // ...
};
```

Guidelines:

- Always early-return when the value didn't change, then store it and call AKNode::addChange().
- Consume the accumulated changes in AKNode::onSceneBegin() (layout/asset updates), AKBakeable::bakeEvent() (re-drawing) or AKRenderable::renderEvent(), via AKNode::changes() or the event's `changes` member: `if (changes().testAnyOf(CHValue, CHRange)) ...`.
- Changes are tracked **per target**, so a node shown on two screens repaints correctly on both.
- For state that doesn't need re-drawing (e.g. driving an animation), AKNode::repaint() just marks the targets dirty.

## Composite components (AKContainer)

The simplest and most common case: arrange existing nodes and wire their signals.

```cpp
class MyLabeledIcon : public AKContainer
{
public:
    MyLabeledIcon(const std::string &iconName, const std::string &label, AKNode *parent = nullptr) noexcept :
        AKContainer(YGFlexDirectionRow, /* clipsChildren */ false, parent),
        m_icon(iconName, 24, this), // Children simply pass `this` as parent
        m_label(label, this)
    {
        layout().setAlignItems(YGAlignCenter);
        layout().setGap(YGGutterColumn, 8.f);
        layout().setPadding(YGEdgeAll, 8.f);
    }

private:
    AKFontIcon m_icon;
    AKText m_label;
};
```

Composites can also derive from a rendering node when the component itself has a background: AKButton, for instance, derives from AKRoundSolidColor (its rounded background) and adds AKText and AKFontIcon children.

## Custom renderables (AKRenderable)

Implement AKRenderable::renderEvent(). It is called during AKScene::render() with the AKRenderEvent, whose coordinates are relative to the current target surface:

- `event.rect`: the node rect in target surface coordinates.
- `event.damage`: the region within `rect` that must be repainted (never paint outside it — the painter functions take it as a clip).
- `event.pass`: the RPass to render with (use RPass::getPainter() for the built-in shaders).

```cpp
class MyTexture : public AKRenderable
{
public:
    MyTexture(AKNode *parent = nullptr) noexcept :
        AKRenderable(RenderableHint::Image, parent) {}

    void setImage(std::shared_ptr<RImage> image) noexcept
    {
        if (m_image == image)
            return;

        m_image = image;
        addChange(CHImage);
        addDamage(AK_IRECT_INF); // Damage the entire node
    }

    enum Changes { CHImage = AKRenderable::CHLast, CHLast };

protected:
    void renderEvent(const AKRenderEvent &e) override
    {
        if (e.damage.isEmpty() || !m_image)
            return;

        auto *p { e.pass->getPainter() };

        RDrawImageInfo info {};
        info.image = m_image;
        info.dst = e.rect;
        info.src.setWH(m_image->size().width(), m_image->size().height());
        info.srcScale = 1.f;

        p->drawImage(info, &e.damage);
    }

private:
    std::shared_ptr<RImage> m_image;
};
```

Notes:

- The `RenderableHint` passed to the constructor tells the scene whether the content is a solid color or an image, which it uses for opacity/color-related optimizations.
- When the content changes, add damage explicitly with AKRenderable::addDamage(). Layout-driven damage (moving, resizing, scale changes) is handled by the scene.
- Fill AKRenderable::opaqueRegion (in node-local coordinates) whenever part of your content is fully opaque: the scene uses it to cull occluded nodes and skip blending. Update it only when needed (e.g. on size/content changes, typically in onSceneBegin()).
- Common properties (opacity(), color(), colorFactor(), custom blend modes) are applied by the scene automatically.

## Custom bakeables (AKBakeable)

Implement AKBakeable::bakeEvent(). The node gets its own RSurface per target, scaled by the target's scale factor, and the default renderEvent() blits it to the target. The AKBakeEvent provides (all regions in node-local coordinates):

- `event.changes`: the accumulated changes for the current target.
- `event.damage`: on input, the region the scene asks you to repaint (non-empty when the node is shown for the first time or was resized). Add any region you re-draw to it.
- `event.clip`: the currently exposed (non-occluded) region — useful to skip drawing hidden parts of large nodes.
- `event.opaque`: fill it with the region you know is fully opaque (persistent across calls).
- `event.surface`: the RSurface to draw into.

```cpp
class MyBadge : public AKBakeable
{
public:
    MyBadge(AKNode *parent = nullptr) noexcept : AKBakeable(parent)
    {
        layout().setWidth(24.f);
        layout().setHeight(24.f);
    }

    enum Changes { CHCount = AKBakeable::CHLast, CHLast };

    void setCount(UInt32 count) noexcept
    {
        if (m_count == count)
            return;

        m_count = count;
        addChange(CHCount);
    }

protected:
    void bakeEvent(const AKBakeEvent &e) override
    {
        // Nothing to repaint and none of our properties changed: keep the cached surface
        if (e.damage.isEmpty() && !e.changes.test(CHCount))
            return;

        auto pass { e.surface->beginPass() };
        auto *c { pass->getCanvas() }; // SkCanvas, already scaled to the target's scale factor

        c->save();
        c->clipIRect(SkIRect::MakeSize(worldRect().size()));
        c->clear(SK_ColorTRANSPARENT);
        // ... draw with Skia ...
        c->restore();

        e.damage.op(SkIRect::MakeSize(worldRect().size()), SkRegion::kUnion_Op); // We repainted everything
    }

private:
    UInt32 m_count { 0 };
};
```

Notes:

- bakeEvent() is called on every render (unless the node is invisible or fully occluded) — it is your early-return that prevents redundant re-drawing.
- Draw in **logical coordinates** (worldRect().size()); the canvas is pre-scaled so the result is pixel-perfect at any scale factor. Re-draw when the scale changes (`AKNode::CHLayoutScale` — included in the incoming `event.damage`).
- Surfaces only grow on resize. Call RSurface::shrink() if you need to reclaim memory.

## Sub-scenes (AKSubScene)

Derive from AKSubScene when a whole subtree should be rendered into a private framebuffer: children are baked (with internal damage tracking handled by a nested scene) and then composited as a single image, so you can apply opacity, masks or other post-processing to the result, or simply cache a complex subtree. Children are always clipped to its bounds. Override AKSubScene::bakeEvent() and call bakeChildren() first if you want to post-process the baked result.

## Background effects (AKBackgroundEffect)

Derive from AKBackgroundEffect for decorations rendered behind a target node and outside the layout (shadows, blur, glows). The effect is attached with AKNode::addBackgroundEffect() and positioned either right behind the node (AKBackgroundEffect::Behind) or behind all of its siblings (AKBackgroundEffect::Background). Implement:

- `onTargetNodeChanged()`: react to being (re)attached.
- `targetNodeRectCalculated()`: set `effectRect` (relative to the target node) once the node's rect is known — e.g. inflate it to make room for a shadow.
- `renderEvent()`: render as a regular renderable.

## Reacting to input and window state

Override the AKNode event handlers you need: `pointerEnterEvent()`, `pointerMoveEvent()`, `pointerButtonEvent()`, `pointerScrollEvent()`, `pointerLeaveEvent()`, `keyboardEnterEvent()`, `keyboardKeyEvent()`, `keyboardLeaveEvent()`, `windowStateEvent()`, `layoutEvent()`, etc. Always call the base class implementation unless you want to suppress propagation. See @ref events_page "Event System" for how events reach nodes.

Useful facilities:

- AKNode::setInputRegion() to limit the interactive area (nullptr = whole node).
- AKNode::setKeyboardFocusable() so tab-navigation (AKKeyboard::nextFocusableNode()) can reach your component, and AKNode::setKeyboardFocus() to request focus (e.g. on click).
- AKNode::setCursor() to choose the cursor shape shown while the node has pointer focus.
- AKScene::setPointerGrab() to keep receiving pointer events during drags.
- Expose interactions with `CZSignal`s (e.g. `CZSignal<const CZPointerButtonEvent&> onClick;`), as the built-in components do.

## Theming and color schemes

- Take colors and metrics from the current theme (CZ::theme()) instead of hardcoding them, so your component follows theme customizations.
- Use `CZAdaptiveColor` for colors that differ between light and dark schemes, resolving them against AKNode::colorScheme() and re-resolving in `colorSchemeEvent()` or via the AKNode::onColorSchemeChanged signal (see AKSolidColor for a minimal example).
- Cache expensive Skia assets shared between instances (like the theme does with rounded-rect nine-patches and shadows) keyed by scale factor, so each variant is baked once.

## Animations

Use `CZLinearAnimation` (or CZTimer) to drive transitions: on each animation tick update the animated property (which calls addChange()/addDamage() and therefore repaints) — see AKButton's color transition for a real example. For purely positional animations, updating layout properties is enough; the scene damages moved nodes automatically.
