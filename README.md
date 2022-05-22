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

I only build on Windows once in a while, it is possible that not everything works out of the box.  The importer package currently uses X11 and I've not found the time to port it to Windows.

The Windows build uses SDL and OpenGL to create the UI.  You can either build using Visual Studio or Cygwin (with Clang or the GCC compiler).  In both cases, you need to download the [development SDL package](https://www.libsdl.org/release/SDL2-devel-2.0.22-VC.zip) for Windows.

### Visual Studio
If you don't have a build setup on Windows yet, installing Visual Studio is probably the easiest option.  Select the "Desktop Development with C++" workload and its optional "C++ Clang tools for Windows" component.  Obtain the same git repositories as for Linux and make the same adjustment for Bandit/Snowhouse (mv the snowhouse include folder into bandit/assertion_frameworks, see above).  Unpack the SDL zip file into `ddhelper`.  Rename the directory from `SDL2-2.0.x` to just `SDL`.  Open the `ddhelper` in Visual Studio to create and build a Studio solution.

### Cygwin with Clang
If you're not using Visual Studio, you probably want to install [Cygwin](https://www.cygwin.com/setup-x86_64.exe), select git, clang, cmake, and ninja from the Cygwin installer (or separately, if you wish).  Open a Cygwin terminal to run the same commands as for Linux to obtain the packages and move everything into place.  Unpack the SDL zip file into `ddhelper`.  Rename the directory from `SDL2-2.0.x` to just `SDL`.

Use CMake to create a Ninja Makefile and build the project:

```
mkdir build
cd build
cmake .. -G "Ninja Multi-Config" -DCMAKE_C_COMPILER=clang.exe -DCMAKE_CXX_COMPILER=clang++.exe
ninja
```
