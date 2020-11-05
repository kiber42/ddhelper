Prerequisites
=============

Linux
-----
```
sudo apt install clang
sudo apt install cmake
sudo apt install libsdl2-dev
```

For OpenGL build:
`sudo apt install libglew-dev`

For Vulkan build:
`sudo apt install libvulkan-dev`

```
git clone git@github.com:ocornut/imgui.git
git clone git@github.com:banditcpp/bandit.git
git clone git@github.com:banditcpp/snowhouse.git
cp -r snowhouse/include/snowhouse/* bandit/bandit/assertion_frameworks/snowhouse
rm -rf snowhouse
mv bandit engine
```

Windows
-------
Install `clang`, `cmake`, `ninja`, `git`
Retrieve `imgui` to this directory.
Retrieve `bandit` and `snowhouse` and move them to the `engine` sub-directory,
  the `snowhouse` include files may need to be placed inside the appropriate `bandit` sub-directory.
Download the SDL zip file for Windows, unpack them into a sub-directory called `SDL`.
The Windows build uses SDL + OpenGL.

Visual Studio Code
------------------

Useful extensions:
+ C/C++ Extension
+ CMake Tools Extension
+ Clang-Format Extension
+ CMake-Format Extension
+ `pip install cmake-format`

`Ctrl+Shift+P` CMake: Configure

`Ctrl+Shift+P` CMake: Build
