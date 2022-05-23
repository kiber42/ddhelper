# About

This repository holds library and application code for **DDHelper**, a set of tools to help solve challenges in the brilliant puzzle game [Desktop Dungeons](http://www.desktopdungeons.net/).  For in-depth information about the game and its mechanics, also have a look at the [DDWiki](http://www.qcfdesign.com/wiki/DesktopDungeons/index.php?title=Desktop_Dungeons), a resource that I used often during development.

There are four main components to DDHelper:

- The **engine** package, that holds all the game logic.  It currently covers the essentials needed to simulate heroes and monsters, melee and magic, the faith system and items.  It does not provide functionality yet to represent actual dungeon (or sub-dungeon) maps, it simply assumes that every monster, every shop etc. is accessible, and that there is always a suitable wall available for a Pisorf cast, et cetera.
- The **solver** package is in an early stage.  It provides functionality to automatically find winning sequences of actions for a given set of monsters and resources.
- The **importer** package provides functionality to grab the current game state from the game window.  Currently, it only imports monsters and their stats.  It will move the mouse cursor to hover over monsters when they are not at full health, to determine the exact amount of HP they have.  It currently only works on Linux.
- Finally, the **ui** package provides a graphical frontend to access most of the features of the engine, solver, and importer libraries.  It provides the **dhelper** executable.

# Prerequisites

The ddhelper UI is built using imgui with an OpenGL + SDL backend.  The solver uses multi-threading, built on the Intel Threading Building Blocks (Linux build only).  Screenshot acquisition is done using OpenCV.  The engine and solver package both come with a set of unit tests (`testengine` and `testsolver`, respectively), implemented using the bandit/snowhouse testing framework.

## Linux

A recent version of clang (>=11) or gcc (>=10) is required.  CMake is used as a build tool.

```
sudo apt install clang
sudo apt install cmake
sudo apt install libglew-dev
sudo apt install libsdl2-dev
sudo apt install libtbb-dev
sudo apt install libopencv2-dev
```

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

Configure and execute the CMake build from an IDE of your choice, or from the command line (assuming clang):

```
cmake -S . -B build -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++
make -j -C build
```

The executable for the helper UI is `build/ui/ddhelper`.  It is still work in progress, but perhaps you'll find useful already.  Have fun!

## Windows

I only build on Windows once in a while, it is possible that not everything works out of the box.  The importer package currently uses X11 and I've not found the time to port this package to Windows.  The Windows build uses SDL and OpenGL to create the UI.  SDL is often not available, see the instructions below how to obtain the development and runtime libraries.

### Prepare Visual Studio and sources
Install any edition of Visual Studio.  In the Visual Studio Installer, select the "Desktop Development with C++" workload.  Obtain the same git repositories as for Linux and make the same adjustment for Bandit/Snowhouse (mv the snowhouse include folder into bandit/assertion_frameworks).  If you have a Unix Terminal available (e.g. GIT Bash or Cygwin), you can simplify this step and just run the same commands as above:

```
git clone https://github.com/kiber42/ddhelper.git
cd ddhelper
git clone https://github.com/ocornut/imgui.git
git clone https://github.com/banditcpp/bandit.git
git clone https://github.com/banditcpp/snowhouse.git
cp -r snowhouse/include/snowhouse/* bandit/bandit/assertion_frameworks/snowhouse
rm -rf snowhouse
```

### Prepare SDL
Download the [development SDL package](https://www.libsdl.org/release/SDL2-devel-2.0.22-VC.zip) and unpack it into `ddhelper`.  Rename the directory from `SDL2-2.0.x` to just `SDL`.  Copy the file `SDL/lib/x64/SDL2.dll` to a suitable location for libraries, such as `C:\Windows\system32` (any directory on the Path will do).

### Build and run
Now everything is in place.  Open the `ddhelper` directory with Visual Studio to create a CMake based solution.  Build and run `ddhelper.exe`.  I hope you'll find it useful.  Happy dungeoneering!
