# About

This repository holds library and application code for **DDHelper**, a set of tools to help solve challenges in the excellent game [Desktop Dungeons](http://www.desktopdungeons.net/).  To learn more about the game an its mechanics, also consider the [DDWiki](http://www.qcfdesign.com/wiki/DesktopDungeons/index.php?title=Desktop_Dungeons), a resource that I used often during development.

There are four main components to DDHelper:

- The **engine** package, that holds all the game logic.  It currently covers the essentials needed to simulate heroes and monsters, melee and magic, the faith system and items.  It does not provide functionality yet to represent actual dungeon (or sub-dungeon) maps, it simply assumes that every monster, every shop etc. is accessible, and that there is always a suitable wall available for a Pisorf cast, et cetera.
- The **solver** package is in an early stage.  It provides functionality to automatically find winning sequences of actions for a given set of monsters and resources.
- The **importer** package provides functionality to grab the current game state from the game window.  Currently, it only imports monsters and their stats.  It will move the mouse cursor to hover over monsters when they are not at full health, to determine the exact amount of HP they have.  It currently only works on Linux.
- Finally, the **ui** package provides a graphical frontend to access most of the features of the engine, solver, and importer libraries.  It provides the **dhelper** executable.

# Prerequisites

The ddhelper UI is built using imgui with an OpenGL or Vulkan backend.  The solver uses multi-threading, built on the Intel Threading Building Blocks (Linux build only).  Screenshot acquisition is done using OpenCV.  The engine and solver package both come with a set of unit tests (`testengine` and `testsolver`, respectively), implemented using the bandit/snowhouse testing framework.

## Linux

A recent version of clang (>=11) or gcc (>=10) is required.  CMake is used as a build tool.

```
sudo apt install clang
sudo apt install cmake
sudo apt install libsdl2-dev
sudo apt install libtbb-dev
sudo apt install libopencv2-dev
```

For the OpenGL build:
`sudo apt install libglew-dev`

For the Vulkan build:
`sudo apt install libvulkan-dev`

```
git clone https://github.com/kiber42/ddhelper.git
cd ddhelper
git clone https://github.com/ocornut/imgui.git
git clone https://github.com/banditcpp/bandit.git
git clone https://github.com/banditcpp/snowhouse.git
cp -r snowhouse/include/snowhouse/* bandit/bandit/assertion_frameworks/snowhouse
rm -rf snowhouse
```

Configure and execute the CMake build from an IDE of your choice, or from the command line:

```
mkdir build
cd build
cmake ..
make -j
```
The most important executables are `ui/ddhelper`, `engine/testengine` and `solver/testsolve`.

## Windows

The Windows build is not currently maintained and is probably broken.  Sorry!

The Windows build uses SDL + OpenGL.  The instructions are for a fresh clang build setup in Visual Studio Code.  Alternatively, you could use Visual Studio.

Download and install [git](https://git-scm.com/download/win), [Visual Studio Code](https://code.visualstudio.com/download), [clang](https://github.com/llvm/llvm-project/releases/tag/llvmorg-14.0.0), and [cmake](https://cmake.org/download/).

Also download the [development SDL package](https://www.libsdl.org/download-2.0.php) for Windows.

Start GIT bash and acquire the required repositories (same commands as for Linux).
Additionally, you need to unpack SDL into a sub-directory called `SDL` inside `ddhelper`.

Open VS Code.  Open and trust the `ddhelper` directory, then open any C++ source file (e.g. from `engine/src`).  VS Code will prompt you to install the C/C++ Extension Pack.  Run "CMake: Configure" and then "CMake: Build" (use `Ctrl+Shift+P` to bring up the relevant prompt).

## Random Visual Studio Code notes

Useful extensions:

- C/C++ Extension
- CMake Tools Extension
- clangd
- Clang-Format Extension
- CMake-Format Extension
- `pip install cmake-format`

`Ctrl+Shift+P` CMake: Configure

`Ctrl+Shift+P` CMake: Build
