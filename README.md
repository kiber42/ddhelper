Prerequisites
-------------
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

Visual Studio Code Extensions:
+ C/C++ Extension
+ CMake Tools Extension
+ Clang-Format Extension

`Ctrl+Shift+P` CMake: Configure

`Ctrl+Shift+P` CMake: Build
