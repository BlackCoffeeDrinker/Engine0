# Engine00

Basic engine for doing ALTTP types of games for DOS.

> **WIP** This project is still in its early stages and is not yet ready for use.

The current target is 16 colors VGA 640x480 but the system allows for any GFX backend. VBE modes will be suported in the
future along with resizable windows.

The current default palette is :

| Index | Red | Green | Blue | Hex Value |
|-------|-----|-------|------|-----------|
| 0     | 20  | 12    | 28   | #140C1C   |
| 1     | 68  | 36    | 52   | #442434   |
| 2     | 48  | 52    | 109  | #30346D   |
| 3     | 78  | 74    | 78   | #4E4A4E   |
| 4     | 133 | 76    | 48   | #854C30   |
| 5     | 52  | 101   | 36   | #346524   |
| 6     | 208 | 70    | 72   | #D04648   |
| 7     | 117 | 113   | 97   | #757161   |
| 8     | 89  | 125   | 206  | #597DCE   |
| 9     | 210 | 125   | 44   | #D27D2C   |
| 10    | 133 | 149   | 161  | #8595A1   |
| 11    | 109 | 170   | 44   | #6DAA2C   |
| 12    | 210 | 170   | 153  | #D2AA99   |
| 13    | 109 | 194   | 202  | #6DC2CA   |
| 14    | 218 | 212   | 94   | #DAD45E   |
| 15    | 222 | 238   | 214  | #DEEEE6   |

The visuals will be a mix of Zelda a Link to The Past and Farland on the PC-98. I'm targeting a 386 with 8MB of RAM.
Sound Blaster support, Mouse support. The engine shall be capable of animated sprites and be easy for a end dev to make a
new games in. It must support smooth scrolling and loading assets as needed (no loading screen required). To do so all
resources use a special ResourcePtrT<...> smart pointer that can lazy load and unload the actual asset as needed. The
GUI engine is inspired by QT. The Engine core is platform-independent and uses "Backends" to access the underlying
hardware.

## Basic definitions

* **GameClock**: The engine's clock that keeps track of time in a game.
* **Sprite**: A bitmap that can be animated.
* **Actor**: Movable, Interactable item. Has a world position, a collision mask, a sprite and can fire events.
* **NPC**: Special Actor that has dialogs and can be interacted with.
* **Enemy**: Special Actor that has AI and can be interacted with.
* **Player**: Special Actor reserved for the controllable player avatar.
* **Trigger**: Special Actor that has no sprite and fires an event when the player collides with it.
* **Background Tile**: A sprite and if the player collides with it.
* **World**: A tile map and the list of all actors.
* **Engine**: The root class of the game. It owns the current `World`, the root `Widget`, the input-to-`Action`
  bindings, the script engine and the game clock. Games are expected to derive from `Engine` and override its
  lifecycle hooks (`OnInit`, `OnFirstTick`, `OnPause`, `OnResume`, `OnWorldLoaded`, `OnWorldUnload`, `ExecuteAction`,
  ...). The backend drives the `Engine` by calling `Tick()` every frame and `ProcessInputEvent()` whenever an
  `InputEvent` is received.
* **Action**: A game-defined event to be handled (Quit, Jump, MoveLeft, MoveRight, ...). An `Action` is a pair of a
  value and an `ActionCategory`, so different categories of actions can reuse the same underlying values without
  colliding.
* **ActionInstance**: An `Action` that was triggered at a specific point in time (a `GameClock::time_point`). The
  `Engine` queues `ActionInstance`s and executes them in order when their time comes.
* **InputEvent**: An event fired by the backend that indicates the user pressed a key, clicked a mouse button or
  moved an axis (mouse, joystick, gamepad). It carries the originating `InputSystem`, a `Type`
  (`KeyUp`/`KeyDown`/`Axis`), a value and, for axis events, a delta. `Engine::BindInputEventToAction` maps an
  `InputEvent` to an `Action` so the game logic never has to deal with raw input directly.
* **InputSystem**: A backend-provided source of input (e.g. Keyboard, Mouse, Joystick). It only needs to know how to
  name itself and how to describe the meaning of a given input value (e.g. turning a scancode into "Space" or a
  mouse button into "Left Button"). `InputEvent`s always reference the `InputSystem` that generated them.
* **Painter**: The drawing interface exposed by a backend. It tracks the current pen/brush state and exposes
  drawing primitives (`DrawPoint`, `DrawLine`, `DrawEllipse`, `DrawRect`, `DrawSurface`, ...). Backends only have to
  implement `DrawPoint` themselves; every other primitive has a default (likely slower) implementation built on top
  of it, but backends are encouraged to override them for better performance.
* **Stream** / **WritableStream**: An abstraction over a readable (and optionally writable) sequence of bytes. The
  backing storage does not need to be a plain file; it could just as well be an entry inside a ZIP or TAR archive.
  `Stream` exposes `Read`, `SeekTo`, `Position`, `Size` and convenience helpers, while `WritableStream` adds `Write`.
* **StreamFactory**: The backend service responsible for creating `Stream`/`WritableStream` instances by resource
  name (`OpenStream`, `OpenStreamForWrite`) and for configuring where resources are loaded from
  (`SetResourceDirectory`). The engine core only ever talks to `StreamFactory::GlobalStreamFactory()`, never to the
  filesystem directly.

The background is a tile base grid. Anything that needs to be dynamic should be an actor.

## Building the project

### Prerequisites

You will need:

* A modern C/C++ compiler
* CMake 3.1+ installed (on a Mac, run `brew install cmake`)
* If you prefer to code in a great IDE, I highly recommend [Jetbrains CLion](https://www.jetbrains.com/clion/). It is
  fully compatible with this project.

### Building The Project

#### Git Clone

First we need to check out the git repo:

```bash
❯ mkdir ~/workspace
❯ cd ~/workspace
❯ git clone \
    https://github.com/BlackCoffeeDrinker/Engine0.git \
    my-project
❯ cd my-project
❯ git submodule init && git submodule update
```

#### Run cmake to build the makefile

```bash
❯ cmake -B build-dir
❯ cd build-dir
❯ make -j 12
```

or

```bash
❯ cmake -B build-dir
❯ cmake --build build-dir -j 12
```

### Building in CLion

> **NOTE**: Since JetBrains
> software [does not officially support git submodules](https://youtrack.jetbrains.com/issue/IDEA-64024), you must run
`git submodule init && git submodule update` before starting CLion on a freshly checked-out repo.

Assuming you've done the above two steps, you can start CLion, and open the project's top level folder. CLion should
automatically detect the top level `CMakeLists.txt` file and provide you with the full set of build targets.

Select menu option **Build ➜ Build Project**.

## Platforms / Backends

The engine core (`engine/include/`, `engine/src/`) is platform-independent. Everything platform-specific lives
behind a small set of backend interfaces so the same game code can run on DOS, macOS, or any other supported target.
A backend is expected to provide implementations for, at least:

* **`StreamFactory`**: How to open resources for reading/writing (files, packed archives, ...) and where the
  resource directory lives on that platform.
* **`InputSystem`** (one or more): How keyboard, mouse, joystick or other input devices are polled/interrupted, and
  how to translate a raw platform-specific value/scancode into a human-readable name. The backend is responsible for
  turning raw hardware events into `InputEvent`s and feeding them to `Engine::ProcessInputEvent`.
* **`Painter`**: How to actually put pixels/lines/shapes/surfaces on the screen for the target video mode.
* A way to drive the main loop: initializing the platform, calling `Engine::Tick()` on a regular basis, and
  shutting everything down cleanly on exit.

Current backends:

* **`engine/src/Backend_DOS/`**: The DOS/DJGPP backend (the primary long-term target). Targets 16 colors VGA
  640x480, planar Mode 12h. Keyboard input is IRQ1-driven with full extended scancode (`0xE0` prefix) support,
  mouse input uses the INT 33h mouse driver with relative motion via mickeys, and joystick input uses the gameport
  joystick via BIOS INT 15h (axes) and direct port `0x201` reads (buttons) with software calibration.
* **`engine/src/Backend_Apple/`**: macOS-specific backend pieces (streams, platform thread naming, sink creation),
  used together with the SDL backend for local development and testing on macOS.

> Legacy DOS code may still exist under `TO_DELETE_platforms/dos/` while the backend refactor is completed; new work
> should target `engine/src/Backend_DOS/` instead.

## Making your own game

> TODO
