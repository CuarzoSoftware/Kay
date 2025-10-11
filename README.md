<img style="position:absolute;height:72px;margin:0px;padding:0;top:400px" src="doxygen/img/Logo.png"/>
<p align="left">
  <a href="https://github.com/CuarzoSoftware/Kay/blob/main/LICENSE">
    <img src="https://img.shields.io/badge/license-LGPLv2.1-blue.svg" alt="Kay is released under the LGPLv2.1 license." />
  </a>
  <a href="https://github.com/CuarzoSoftware/Kay">
    <img src="https://img.shields.io/badge/version-0.1.0-brightgreen" alt="Current Kay version." />
  </a>
</p>


# Kay

> ⚠️ **This project is a work in progress.**

**Kay** is a C++ GUI framework and rendering library for Linux, named in tribute to Alan Kay. It provides tools for building scalable UI components with a flexible scene system, 
a layout engine based on [Yoga](https://www.yogalayout.dev/), and drawing capabilities that integrate with multiple graphics APIs through **Ream**.

Kay is designed to work with any input system and graphics context supported by Ream. It can, for example, be embedded into Wayland clients or compositors to optimize rendering.

### Key Features

* **Built-in Components** – A set of ready-to-use UI elements that handle input events.
* **Custom Drawing** – Support for custom components rendered with [Skia](https://skia.org/) or Ream's `RPainter`.
* **Flexbox Layout** – A layout system powered by Yoga.
* **Damage Tracking** – Only modified regions of the UI are re-rendered.
* **Scaling** – Supports high-DPI environments and fractional scaling.
* **Screen Transforms** – Allows rendering to rotated, flipped, etc displays.
* **Input Integration** – Can be fed with external input events.

### Core Components

#### `AKScene`

Represents a UI tree. It manages node layout, damage tracking, and drawing.

#### `AKTarget`

Defines how and where a scene is drawn.

#### `AKNode`

The base class for all layout and visual elements in Kay. Nodes can be nested and support layout updates.

Subclasses include:

* **`AKContainer`**
  A node used only for layout. It doesn’t render content but may clip its children.

* **`AKRenderable`**
  A drawable node that implements `renderEvent()` to paint content directly into the target.

* **`AKBakeable`** *(inherits from `AKRenderable`)*
  Draws into an off-screen image (framebuffer), which is reused unless content or size changes. Implements `bakeEvent()`, and helps avoid repeating expensive drawing operations. Framebuffers are resized only when needed and can shrink automatically or manually.

* **`AKSubScene`** *(inherits from `AKBakeable`)*
  Renders its children into its own framebuffer. Useful for caching entire subtrees or applying post-processing. All child nodes are clipped to the `AKSubScene` bounds.

* **`AKBackgroundEffect`** *(inherits from `AKRenderable`)*
  Used for visual effects such as shadows or blur behind other components.
