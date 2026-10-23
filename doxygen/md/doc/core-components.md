# Core Components {#core_components_page}

This page describes the building blocks every Kay application or compositor is made of, and how they relate to each other. For a hands-on guide to writing your own components, see @ref creating_components_page "Creating Components".

## Overview

A Kay UI is a **tree of nodes** (AKNode subclasses) laid out with Flexbox (Yoga). An **AKScene** takes the root of that tree and renders it into one or more **AKTargets** (render destinations), performing damage tracking so only what changed is repainted. A single **AKApp** instance provides shared services (input routing, fonts, the Ream graphics context) to all scenes.

```
AKApp (singleton)
 ├── AKScene (one per window, or one shared by all screens in a compositor)
 │    ├── AKTarget(s)  →  RSurface (where the result is stored)
 │    └── root AKNode
 │         ├── AKContainer
 │         │    ├── AKButton
 │         │    └── AKText
 │         └── ...
 └── AKPointer / AKKeyboard (input state & focus)
```

## AKApp

AKApp is a per-process singleton created with AKApp::GetOrMake(). It requires a **CZCore** (event loop, timers, events) and an **RCore** (Ream graphics context) to exist first — when using Marco or Louvre these are created for you.

It provides:

- AKApp::pointer() and AKApp::keyboard(): global input state, focus tracking and event history.
- AKApp::fontManager() and AKApp::fontCollection(): Skia font services used by text nodes.
- Event dispatching: input events are typically sent to the AKApp, which forwards them to the focused AKScene (see @ref events_page "Event System").

Nodes keep a shared reference to the app, so it stays alive as long as any node exists.

## AKScene

An AKScene renders a node tree into targets. Its responsibilities:

- Calculating the layout (Yoga) and the final rect of each node.
- Damage tracking: diffing per-node changes and accumulating damage per target.
- Rendering nodes back-to-front with opaque/translucent pass separation and occlusion culling.
- Input focus: in Kay, a scene can be thought of as a window. It keeps track of the pointer focus, keyboard focus and pointer grabs (AKScene::pointerFocus(), AKScene::keyboardFocus(), AKScene::setPointerGrab()) and dispatches input events to the right nodes.

Scenes are created with AKScene::Make() and the node tree is assigned with AKScene::setRoot(). Only the children of the root node are rendered; the root itself produces no output, but its layout properties affect its children.

## AKTarget

An AKTarget describes **where** a scene renders during an AKScene::render() call. Targets are created with AKScene::makeTarget() and cannot be shared across scenes. The key properties:

- AKTarget::surface: the destination RSurface (mandatory). Its viewport defines the world-space region displayed by the target.
- AKTarget::age: the buffer age, enabling damage-ring reuse of previous frames.
- AKTarget::inDamage / AKTarget::inClip: optional extra damage and clipping.
- AKTarget::outDamage / AKTarget::outOpaque / AKTarget::outInvisible / AKTarget::outInput: optional regions filled during rendering, useful for compositors (e.g. to pass damage to the display backend or compute input regions).
- AKTarget::isDirty() and AKTarget::onMarkedDirty: tell you when a new frame is needed, e.g. to schedule a repaint of a window or screen.

**Applications** typically use one scene with a single target per window.
**Compositors** typically share a single scene across multiple targets (one per screen). Nodes keep their state (damage, changes, baked surfaces, etc.) **per target**, so a node spanning two screens is tracked correctly on both.

## AKNode

AKNode is the base class of all components. By itself it handles:

- **Hierarchy**: AKNode::setParent(), AKNode::insertBefore()/insertAfter(), children ordering (later siblings are rendered above earlier ones).
- **Layout**: each node owns an AKLayout (a Yoga node) exposing Flexbox properties. After layout, AKNode::worldRect() is the node rect in world coordinates and AKNode::scale() the greatest scale factor among intersected targets.
- **Change tracking**: AKNode::addChange() records a property change (see AKChanges) per target and marks intersected targets as dirty. AKNode::repaint() only marks targets dirty.
- **Visibility & clipping**: AKNode::setVisible(), AKNode::enableChildrenClipping().
- **Input**: pointer/keyboard event handlers, AKNode::setInputRegion(), keyboard focusability and AKNode::setCursor().
- **Color scheme**: light/dark scheme propagation (AKNode::setColorScheme(), AKNode::onColorSchemeChanged).

Nodes advertise their rendering role through AKNode::caps(). The roles are implemented by the following subclasses, which are the classes you normally derive from:

### AKContainer

A node with **no output of its own**: it only lays out and groups children, optionally clipping them (AKNode::enableChildrenClipping()). Use it as the base for composite components built entirely from existing nodes.

### AKRenderable

A node that **draws directly into the target** each time it must be repainted, through AKRenderable::renderEvent(). Rendering is done with the Ream painter (RPass / RPainter), which uses efficient internal shaders — ideal for images (e.g. Wayland surfaces), solid fills and masks. It also provides common properties handled by the scene: opacity, color, color factor, custom blend modes, an AKRenderable::opaqueRegion (occlusion culling hint) and an AKRenderable::invisibleRegion.

Built-in examples: AKImage, AKSolidColor.

### AKBakeable

An AKRenderable with **its own surface (framebuffer)** per target. Content is drawn with Skia into the surface during AKBakeable::bakeEvent() (usually only when something changed), and the default renderEvent() implementation simply blits the surface into the target. The surface resolution follows the target scale factor, so baked content is always pixel-perfect.

Use it for content that is expensive to draw (text, paths, complex vector shapes): once baked, the scene can re-composite it for free when it only moves. Surfaces only grow when the node is resized; call RSurface::shrink() to reclaim memory.

Built-in examples: AKText, AKPath, AKFontIcon, AKNinePatch.

### AKSubScene

An AKBakeable that **renders its children into its own framebuffer** (an internal nested AKScene handles damage tracking inside it). Children are always clipped to its bounds. Use it to apply post-processing to an entire subtree (opacity, masks, effects) or to avoid re-compositing complex subtrees.

### AKBackgroundEffect

A special AKRenderable that is not part of the layout. It is attached to a *target node* with AKNode::addBackgroundEffect() and is rendered **behind** it — either immediately behind (AKBackgroundEffect::Behind) or at the bottom of the target's parent (AKBackgroundEffect::Background). Used for shadows, blur, and similar effects.

Built-in examples: AKBackgroundBoxShadowEffect, AKBackgroundImageShadowEffect, AKBackgroundBlurEffect, AKEdgeShadow.

## AKLayout

Every node exposes its Yoga layout through AKNode::layout(). All standard Flexbox properties are available (flex direction, justify/align, grow/shrink, gaps, padding, margins, absolute positioning, aspect ratio, explicit or percentage sizes, etc.). The scene calculates the layout of the whole tree during AKScene::render() (unless AKTarget::layoutOnRender is disabled, e.g. when an application needs to measure the root beforehand to size its window).

## AKChanges — the change/damage model

Property setters do not apply or repaint anything immediately. Instead they:

1. Store the new value.
2. Call AKNode::addChange(CHSomething), which records the change **per target** and marks intersected targets dirty (triggering AKTarget::onMarkedDirty).

Later, during the next AKScene::render(), the node reacts to the accumulated AKChanges — in AKNode::onSceneBegin(), AKBakeable::bakeEvent() or AKRenderable::renderEvent() — updating or re-drawing only what is necessary, and adds damage with AKRenderable::addDamage(). Each node subclass extends its parent's `Changes` enum starting at the parent's `CHLast` (see @ref creating_components_page "Creating Components").

## AKTheme

AKTheme centralizes colors (adaptive light/dark system colors), metrics, fonts and cached assets (e.g. rounded-rect nine-patches, shadows) used by the built-in components. The active theme is retrieved with CZ::theme() and replaced with CZ::setTheme(), allowing you to subclass it to customize the look of built-in components. Many color properties are `CZAdaptiveColor`s, which resolve to a concrete color depending on the node's current color scheme (light or dark).

## What's next

- @ref creating_components_page "Creating Components": how to build your own components and which base class to choose.
- @ref events_page "Event System": how events are dispatched and how to feed input from your own backend.
