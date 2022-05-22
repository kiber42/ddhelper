# About

This repository holds library and application code for **DDHelper**, a set of tools to help solve challenges in the brilliant puzzle game [Desktop Dungeons](http://www.desktopdungeons.net/).  For in-depth information about the game and its mechanics, also have a look at the [DDWiki](http://www.qcfdesign.com/wiki/DesktopDungeons/index.php?title=Desktop_Dungeons), a resource that I used often during development.

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

Once you have installed these packages, acquire the source code and move some files into place using the following commands:

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

The executable for the helper UI is `ui/ddhelper`.

## Windows

I only build on Windows once in a while, it is likely that not everything works out of the box.  The importer package currently uses X11 and I've not found the time to port it to Windows.

The Windows build uses SDL and OpenGL to create the UI.  You can either build using Visual Studio (with Clang or the MSVC compiler) or using Cygwin (with Clang or the GCC compiler).  If you don't have a build setup on Windows yet, installing Visual Studio is probably the easiest option.  Select the "Desktop Development with C++" workload and its optional "C++ Clang tools for Windows" component.

In any case you also need to download and install [cmake](https://cmake.org/download/) and [git](https://git-scm.com/download/win).
For convenience, you can use the GIT bash and acquire the required repositories using the same set of commands as for Linux (see above).

Download the appropriate [development SDL package](https://www.libsdl.org/download-2.0.php) for Windows (VC or MinGW) and unpack it into `ddhelper`.  Rename the directory from `SDL2-2.0.x` to just `SDL`.

As for Linux, use CMake to create Makefiles or a Visual Studio solution.  When not using VS, you might want to use the [Ninja build system](https://github.com/ninja-build/ninja/releases).  It is a single .exe file that you can just copy to `ddhelper` (or anywhere else as long as you add to your PATH environment variable).

```
mkdir build
cd build
cmake .. -G "Ninja Multi-Config"
ninja
```
