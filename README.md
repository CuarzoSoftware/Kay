# Kay

Named in honor to Alan Kay, this library is meant to be used for developing rich, fast, and efficient user interfaces for Wayland applications and compositors. However, it can be integrated into any OpenGL context (and in the future, Vulkan and software rasterizer), which means it can also be used on macOS and Windows.

It offers the following features:

- Ability to create reusable UI components (nodes) using all the flexibility offered by [Skia](https://skia.org/), which are then composited with simpler and faster built-in shaders.
- An automatic CSS Flexbox-compliant layout system powered by [Yoga](https://www.yogalayout.dev/).
- Damage tracking.
- Fractional scaling.
- Transforms for rotated/flipped screens.

What it **does not** provide:

- Tools for creating windows, popups, menus, etc (window manager related functionality). If you're building a desktop app, these need to be handled by yourself.
- Listeners for system input events. However, you can specify input regions for nodes, and utilities are available to locate nodes at given positions using user-defined flags as filters.
- A complete set of built-in components (not yet, at least).

## Core Concepts

There are three core concepts related to Kay:

- **AKScene**: A scene is responsible for calculating the positions and dimensions of a tree of nodes, rendering and composing them, and performing other tasks such as damage tracking.
- **AKTarget**: A target defines where a scene will render and provides additional information such as the root node to capture, viewport, scaling factor, the current buffer age, optional clipping, and access to the damage and opaque regions generated from the scene. Typically, a desktop application would create a unique scene with a unique target for each window, as components are not displayed in more than one window at a time. However, for compositors, a single scene is required to, for example, allow dragging a window from one screen to another. Thus, a single scene can be created with different targets for each screen. Nodes track their state and some properties for each target. For instance, if a node is rendered on one target, its damage region will only be cleared for that target but will remain for those where it hasn't been rendered yet.
- **AKNode**: A node is a component that has its own layout and can be nested within other nodes. Depending on its capabilities, it can serve as a container, perform rendering into a target, or render into its own framebuffer, which is then composited into a target by the scene.

## Nodes

Core node types:

- **AKContainer**: A node with no rendering capabilities that only serves as a container for other nodes. It can also (like any other node) optionally clip its children by its bounds.
- **AKRenderable**: Renderable nodes have an `onRender()` virtual method where they can draw content directly into the current target using the internal shaders (AKPainter). They are useful for displaying textures (e.g., Wayland surfaces), solid colors, creating efficient mask effects, etc. Examples of built-in renderable nodes are AKImage, AKSolidColor, and AKBackgroundBlurEffect. Skia can also be used within an `onRender()` event, but its performance is considerably worse than the built-in shaders, so it is recommended to be used with moderation.
- **AKBakeable**: A bakeable node has its own framebuffer with dimensions given by the current target properties (e.g., scaling factor). They have an `onBake()` event and can be used to avoid re-creating complex component each time, allowing the scene to re-composite them efficiently if they e.g. only move. They also allow for more sophisticated effects, such as applying opacity to a group of layers or border radius effects. When resized, their framebuffer only grows in size to avoid re-creating it each time, but it can optionally be automatically or manually shrunk if memory needs to be saved. They are a subclass of AKRenderable, but the default `onRender()` implementation is used to perform their composition into the target.
- **AKSubScene**: A subscene is a subclass of AKBakeable that takes care of rendering its children into its own framebuffer. This allows for sophisticated post-processing effects and avoids frequent re-compositing of layers. Since children nodes are baked into the framebuffer, they are always clipped by its bounds.

## Example

Example running on top of [Louvre](https://github.com/CuarzoSoftware/Louvre) (everything being rendered and composited by Kay):

![](https://lh3.googleusercontent.com/pw/AP1GczPe_4h170fkQwQ6tPfxGGHoLv00X2eHpdg8Ggnq4Gyx4DOsi0Z6eQ7bpZNvVN778wuakYI-ArsJmzeVvZiJARsvgw5VTkl-9Bt9xZpQl5Sjyf59Kpc=w2400)
