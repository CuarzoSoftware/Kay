# Event System

The **Cuarzo Framework** event system (including **Kay**) is heavily inspired by **Qt**.

**CZObjects** or more specifically in this context, **AKNode** (a subclass of **AKObject**) react to events through **virtual methods**. The base class `CZObject` defines a **protected virtual** method named `event()`, which handles all incoming events. Events (`CZEvent`) can be dispatched to an object either **directly** via this method, or preferably through helper functions like `CZCore::sendEvent()`, `CZCore::postEvent()`, or `CZSafeEventQueue`.

A typical `event()` implementation uses a `switch`/`case` structure to delegate events to more specific handlers.
For example, sending a `CZKeyboardKeyEvent` to an `AKNode` will route it through `event()`, which will then invoke `AKNode::keyboardKeyEvent()`.

Although it's technically possible to call `event()` directly, it is **not recommended**. Much like in Qt, objects can **install event filters** on other objects. When an event is sent to a target, it is first passed to the event filter via `eventFilter()`. The filter has the option to inspect the event and even **stop its propagation** but this mechanism only works if `event()` isn't called directly.

- `CZCore::sendEvent()` is lightweight and delivers the event **immediately** to the target object. However, it's potentially **dangerous** if the event leads to object destruction, specially when iterating a node tree.
- `CZCore::postEvent()` safely queues events to be dispatched later by `CZCore::dispatch()`. If a target object is destroyed before dispatch, the event is simply discarded, making it safe.
- `CZSafeEventQueue` behaves similarly, queuing events with **weak references** to target objects, but events are manually dispatched using `CZSafeEventQueue::dispatch()`.
  (Internally, `CZCore::postEvent()` uses this safe queue.)

## Input Events

Kay does not include input backends and expects instead to be fed with them. An exception to this is when using Marco, since Marco handles this itself.
If you are using Kay with Louvre, SRM, or in other context you need feed events manually.

Events are typically sent to AKApp, which then dispatches it to the apropiate AKScene and the AKScene to one or more AKNodes, however each event types have specific handling which are described below.

## Pointer Events

In **Kay**, an `AKScene` can be thought of as a window, and as such, it can be **focused**. To focus a scene, you must send a `CZPointerEnterEvent` to the desired `AKScene`. This will assign the scene to `AKApp::pointer().focus()`. Only one scene can be focused at a time.

Once a scene is focused, all subsequent pointer events should be sent to `AKApp`, which will internally dispatch them to the focused scene.
To remove focus, or to change focus to another scene, you should first send a `CZPointerLeaveEvent` to `AKApp`.

Both `CZPointerEnterEvent` and `CZPointerMoveEvent` have a `pos` attribute, which should represent the pointer's position in **world coordinates**. This allows the `AKScene` to correctly route the event to the appropriate `AKNode`.

Each scene manages and exposes information about its currently focused nodes.

> ⚠️ Note: Events received by `AKNodes` might not directly match the ones originally sent. For example, `AKScene` may synthesize its own enter/leave events for nodes internally.

## Keyboard Events

Keyboard events follow the **same focus model** as pointer events. However, a `CZKeymap` is required to fill fields such as the **key symbol** and the UTF-8 character produced via a **compose table**.

`CZCore` (which must be initialized before creating an `AKApp`) provides a default keymap, and you can replace or configure it as needed.

In **Louvre** and **Marco**, key events are already fully populated, so you don’t need to worry. But in other contexts, you should **first feed the raw key event** (key code and pressed state) into `CZKeymap::feed()` to populate the remaining fields before passing the event to `AKApp` or `AKScene`.

Some `AKNode` components, such as `AKTextField`, rely on that additional key information to function correctly.

`CZKeymap` can also be used to **configure key repeat behavior** (delay and interval settings).

## Touch Events

TODO

## Event Propagation

As explained earlier, `AKScene` is responsible for managing node focus and dispatching events to the appropriate nodes. However, instead of delivering the event solely to the focused node, it propagates the event **up through the node’s parent hierarchy**, all the way to the root node.

## Event History

`AKApp` maintains a history of all received events.
See `AKApp::pointer()` and `AKApp::keyboard()` for more details.
